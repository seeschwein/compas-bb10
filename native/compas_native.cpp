// compas_native.cpp — C++20 native backend implementation
//
// Built with: bb10-cxx20 (Clang 19.1.7, LLVM libc++ 19.1.7)
// ABI: This file must NOT be compiled with the XIAZY Clang 19 / Dinkumware
// toolchain, or the Cascades/Qt 4.8.6 app will crash.

#include "compas_native.h"

#include <fmt/core.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include <zlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// Simple concurrent queue for pending tile requests (single-producer,
// single-consumer via mutex + condvar — sufficient for a native backend).
// ---------------------------------------------------------------------------

struct pending_tile {
    int z, x, y;
    compas_tile_callback cb;
    void* user_data;
    struct pending_tile* next;
};

struct compas_native_impl {
    const char* cache_dir;
    const char* user_agent;
    sqlite3* db;
    pthread_mutex_t pending_mutex;
    pthread_cond_t pending_cond;
    pending_tile* pending_head;
    volatile sig_atomic_t running;
};

// ---------------------------------------------------------------------------
// Global state (one instance)
// ---------------------------------------------------------------------------

static compas_native_impl g;

// ---------------------------------------------------------------------------
// pending queue helpers (thread-safe)
// ---------------------------------------------------------------------------

static void pending_push(int z, int x, int y,
                         compas_tile_callback cb, void* user_data)
{
    pending_tile* p = (pending_tile*)malloc(sizeof(pending_tile));
    if (!p) return;
    p->z = z; p->x = x; p->y = y; p->cb = cb; p->user_data = user_data;
    p->next = NULL;

    pthread_mutex_lock(&g.pending_mutex);
    p->next = g.pending_head;
    g.pending_head = p;
    pthread_mutex_unlock(&g.pending_mutex);
    pthread_cond_signal(&g.pending_cond);
}

static pending_tile* pending_pop(void)
{
    pthread_mutex_lock(&g.pending_mutex);
    pending_tile* p = g.pending_head;
    if (p) g.pending_head = p->next;
    pthread_mutex_unlock(&g.pending_mutex);
    return p;
}

// ---------------------------------------------------------------------------
// SQLite tile cache helpers
// ---------------------------------------------------------------------------

static int create_tile_db(const char* cache_dir)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/tilecache.db", cache_dir);

    int rc = sqlite3_open_v2(path, &g.db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                             NULL);
    if (rc != SQLITE_OK) {
        fmt::print(stderr, "Failed to open tile cache DB: {}\n", sqlite3_errmsg(g.db));
        sqlite3_close(g.db);
        g.db = NULL;
        return -1;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS tiles ("
        "  z INTEGER NOT NULL,"
        "  x INTEGER NOT NULL,"
        "  y INTEGER NOT NULL,"
        "  png BLOB NOT NULL,"
        "  permanent INTEGER NOT NULL DEFAULT 0,"
        "  PRIMARY KEY (z, x, y)"
        ");";

    char* err = NULL;
    rc = sqlite3_exec(g.db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fmt::print(stderr, "Failed to create tiles table: {}\n", err);
        sqlite3_free(err);
        return -1;
    }

    const char* idx = "CREATE INDEX IF NOT EXISTS idx_tiles_zoom ON tiles(z, x);";
    rc = sqlite3_exec(g.db, idx, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fmt::print(stderr, "Failed to create index: {}\n", err);
        sqlite3_free(err);
        return -1;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// HTTP fetch helpers (libcurl)
// ---------------------------------------------------------------------------

struct http_fetch_ctx {
    const char* url;
    uint8_t* data;
    size_t size;
    size_t capacity;
};

static size_t http_write_callback(void* contents, size_t size, size_t nmemb,
                                  void* userp)
{
    size_t total = size * nmemb;
    struct http_fetch_ctx* ctx = (struct http_fetch_ctx*)userp;

    if (ctx->data == NULL) {
        ctx->capacity = total > 4 * 1024 * 1024 ? total : 4 * 1024 * 1024;
        ctx->data = (uint8_t*)malloc(ctx->capacity);
        if (ctx->data == NULL) return 0;
    }

    if (ctx->data && ctx->size + total <= ctx->capacity) {
        memcpy(ctx->data + ctx->size, contents, total);
        ctx->size += total;
    }
    return total;
}

static int http_fetch(const char* url, uint8_t** out_data, size_t* out_size)
{
    CURL* curl = curl_easy_init();
    if (!curl) return -1;

    struct http_fetch_ctx ctx = {NULL, NULL, 0, 0};

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, g.user_agent ? g.user_agent : "Compas/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        free(ctx.data);
        return -1;
    }

    *out_data = ctx.data;
    *out_size = ctx.size;
    return 0;
}

// ---------------------------------------------------------------------------
// PNG decompression (zlib)
// ---------------------------------------------------------------------------

static uint8_t* decompress_png(const uint8_t* src, size_t src_len,
                               size_t* out_len)
{
    z_stream str;
    memset(&str, 0, sizeof(str));

    if (inflateInit2(&str, -MAX_WBITS) != Z_OK) return NULL;

    str.next_in = (Bytef*)src;
    str.avail_in = src_len;

    size_t chunk = 65536;
    size_t out_cap = chunk;
    uint8_t* out = (uint8_t*)malloc(out_cap);
    if (!out) { inflateEnd(&str); return NULL; }
    str.next_out = out;
    str.avail_out = out_cap;

    int ret;
    do {
        if (str.avail_out == 0) {
            out_cap *= 2;
            uint8_t* new_out = (uint8_t*)realloc(out, out_cap);
            if (!new_out) { free(out); inflateEnd(&str); return NULL; }
            out = new_out;
            str.next_out = out + (out_cap / 2);
            str.avail_out = out_cap / 2;
        }
        ret = inflate(&str, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR) { free(out); inflateEnd(&str); return NULL; }
    } while (ret != Z_OK && ret != Z_STREAM_END);

    *out_len = str.total_out;
    inflateEnd(&str);
    return out;
}

// ---------------------------------------------------------------------------
// Native API implementation
// ---------------------------------------------------------------------------

compas_native_t* compas_native_create(const char* cache_dir,
                                       const char* user_agent)
{
    memset(&g, 0, sizeof(g));
    g.cache_dir = cache_dir;
    g.user_agent = user_agent;
    g.running = 1;

    pthread_mutex_init(&g.pending_mutex, NULL);
    pthread_cond_init(&g.pending_cond, NULL);
    g.pending_head = NULL;

    if (create_tile_db(cache_dir) != 0) {
        fmt::print(stderr, "Failed to create tile DB in {}\n", cache_dir);
        return NULL;
    }

    pthread_t worker;
    pthread_create(&worker, NULL, [](void* arg) -> void* {
        (void)arg;
        while (g.running) {
            pending_tile* pt = pending_pop();
            if (!pt) {
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
                pthread_cond_timedwait(&g.pending_cond, &g.pending_mutex, &ts);
                continue;
            }

            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(g.db,
                "SELECT png FROM tiles WHERE z = ? AND x = ? AND y = ? AND permanent = 1 LIMIT 1",
                -1, &stmt, NULL);
            if (rc == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, pt->z);
                sqlite3_bind_int(stmt, 2, pt->x);
                sqlite3_bind_int(stmt, 3, pt->y);

                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    size_t png_len = sqlite3_column_bytes(stmt, 0);
                    const uint8_t* png_blob = (const uint8_t*)sqlite3_column_blob(stmt, 0);

                    if (pt->cb) {
                        pt->cb(pt->user_data, pt->z, pt->x, pt->y,
                               png_blob, png_len, 0);
                    }
                }
                sqlite3_finalize(stmt);
            }

            if (pt->cb) {
                pt->cb(pt->user_data, pt->z, pt->x, pt->y,
                       NULL, 0, -1);
            }

            free(pt);
        }
        return NULL;
    }, NULL);

    return (compas_native_t*)1;
}

void compas_native_destroy(compas_native_t* ctx)
{
    (void)ctx;
    g.running = 0;
    pthread_cond_broadcast(&g.pending_cond);
    pthread_mutex_destroy(&g.pending_mutex);
    pthread_cond_destroy(&g.pending_cond);
    if (g.db) sqlite3_close(g.db);
}

void compas_native_fetch_tile(compas_native_t* ctx,
                              int z, int x, int y,
                              compas_tile_callback cb,
                              void* user_data)
{
    (void)ctx;
    pending_push(z, x, y, cb, user_data);
}

uint8_t* compas_native_get_cached_tile(compas_native_t* ctx,
                                       int z, int x, int y,
                                       size_t* png_len)
{
    (void)ctx;
    (void)z; (void)x; (void)y;
    (void)png_len;
    *png_len = 0;
    return NULL;
}

void compas_native_put_cached_tile(compas_native_t* ctx,
                                   int z, int x, int y,
                                   const uint8_t* png, size_t png_len,
                                   int permanent)
{
    (void)ctx;
    (void)z; (void)x; (void)y; (void)png; (void)png_len; (void)permanent;
}

void compas_lat_lon_to_tile(double lat, double lon, int zoom,
                            int* out_x, int* out_y)
{
    (void)lat; (void)lon; (void)zoom;
    if (out_x) *out_x = 0;
    if (out_y) *out_y = 0;
}

double compas_haversine(double lat1, double lon1,
                        double lat2, double lon2)
{
    (void)lat1; (void)lon1; (void)lat2; (void)lon2;
    return 0.0;
}