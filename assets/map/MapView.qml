// assets/map/MapView.qml
import bb.cascades 1.0

CustomControl {
    id: root
    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Fill

    // Public properties
    property double latitude: 0.0
    property double longitude: 0.0
    property int zoom: 12
    property int tileSize: 256

    // Context properties set from C++
    property var _tileCache: null
    property var _networkMgr: null

    onPaintRequested: {
        var ctx = paintRequested.context;
        if (!_tileCache || !_networkMgr) {
            ctx.fillColor = "#ffff00";
            ctx.fillRect(0, 0, width, height);
            ctx.drawText(10, 20, "MapView not initialized");
            return;
        }

        ctx.fillColor = "#eeeeee";
        ctx.fillRect(0, 0, width, height);

        var tilePx = tileSize;
        var xtileStart = Math.floor((longitude2tile(longitude, zoom) - 0.5 * width / (tilePx * Math.pow(2, zoom))) );
        var xtileEnd   = Math.ceil((longitude2tile(longitude, zoom) + 0.5 * width / (tilePx * Math.pow(2, zoom))) );
        var ytileStart = Math.floor((lat2tile(latitude, zoom) - 0.5 * height / (tilePx * Math.pow(2, zoom))) );
        var ytileEnd   = Math.ceil((lat2tile(latitude, zoom) + 0.5 * height / (tilePx * Math.pow(2, zoom))) );

        for (var tx = xtileStart; tx <= xtileEnd; tx++) {
            for (var ty = ytileStart; ty <= ytileEnd; ty++) {
                var px = (tx - longitude2tile(longitude, zoom) + 0.5 * width / tilePx) * tilePx;
                var py = (ty - lat2tile(latitude, zoom) + 0.5 * height / tilePx) * tilePx;

                var img = _tileCache.getImage(tx, ty, zoom);
                if (img) {
                    ctx.drawImage(img, px, py, tilePx, tilePx);
                } else {
                    ctx.fillColor = "#cccccc";
                    ctx.fillRect(px, py, tilePx, tilePx);
                    ctx.fillColor = "#999999";
                    ctx.drawText(px+2, py+12, "loading…");
                    _networkMgr.fetchTile(tx, ty, zoom);
                }
            }
        }
    }

    function longitude2tile(lon, z) {
        return ((lon + 180) / 360) * Math.pow(2, z);
    }
    function lat2tile(lat, z) {
        var latRad = lat * Math.PI / 180;
        return ( (1 - Math.log(Math.tan(latRad) + 1/Math.cos(latRad)) / Math.PI) / 2 ) * Math.pow(2, z);
    }
}