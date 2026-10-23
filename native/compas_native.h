// compas_native.h — C API for the native C++20 backend.
//
// This header is intentionally C-compatible (extern "C") so that the
// Cascades/Qt 4.8.6 app (Dinkumware libstdc++) can load and call it via
// dlopen/dlsym without any C++ ABI mismatch.  The implementation is
// compiled with zsavajji/bb10-cxx20 (LLVM libc++ 19.1.7), which is ABI-
// incompatible with the Cascades runtime.
//
// The native backend provides:
//   - Tile fetching over HTTP (async, callback-driven)
//   - SQLite-backed tile cache
//   - Geometry helpers (lat/lon ↔ tile, distance)
//
// All memory passed across the boundary is owned by the caller or the
// callee explicitly — no C++ objects cross the ABI line.

#ifndef COMPAS_NATIVE_H
#define COMPAS_NATIVE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle to the native backend instance.
typedef struct compas_native compas_native_t;

// Callback invoked when a tile fetch completes (or fails).
//   user_data  – whatever was passed to compas_native_fetch_tile
//   z, x, y    – tile coordinates
//   png        – PNG bytes (owned by caller must free with free())
//   png_len    – length of png
//   status     – 0 = success, negative = error code
typedef void (*compas_tile_callback)(void* user_data,
                                     int z, int x, int y,
                                     const uint8_t* png, size_t png_len,
                                     int status);

// Create a native backend instance.
//   cache_dir  – directory for the SQLite tile cache (must exist, writable)
//   user_agent – HTTP User-Agent string (may be NULL)
// Returns NULL on failure.
compas_native_t* compas_native_create(const char* cache_dir,
                                       const char* user_agent);

// Destroy a native backend instance.
void compas_native_destroy(compas_native_t* ctx);

// Request a tile.  When the fetch completes (or fails), cb is invoked
// (possibly on a background thread).  The same tile may be returned from
// cache synchronously if it is already cached.
//   cb         – callback (must not be NULL)
//   user_data  – passed through to cb
void compas_native_fetch_tile(compas_native_t* ctx,
                              int z, int x, int y,
                              compas_tile_callback cb,
                              void* user_data);

// Look up a tile in the cache.  Returns the PNG bytes (malloc'd; caller
// frees) and sets *png_len.  Returns NULL if not cached.
//   z, x, y    – tile coordinates
//   png_len    – [out] length of returned buffer
// Returns malloc'd PNG buffer or NULL.
uint8_t* compas_native_get_cached_tile(compas_native_t* ctx,
                                       int z, int x, int y,
                                       size_t* png_len);

// Store a tile in the cache.
//   png, png_len  – PNG bytes (not modified; copied internally)
//   permanent     – if non-zero, store permanently; else ephemeral
void compas_native_put_cached_tile(compas_native_t* ctx,
                                   int z, int x, int y,
                                   const uint8_t* png, size_t png_len,
                                   int permanent);

// Convert lat/lon to tile coordinates at a given zoom level.
void compas_lat_lon_to_tile(double lat, double lon, int zoom,
                            int* out_x, int* out_y);

// Haversine distance between two lat/lon points, in metres.
double compas_haversine(double lat1, double lon1,
                        double lat2, double lon2);

#ifdef __cplusplus
}
#endif

#endif // COMPAS_NATIVE_H