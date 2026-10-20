import bb.cascades 1.0

Page {
    id: root
    Container {
        layout: DockLayout {}
        MapView {
            id: mapView
            latitude: 37.7749
            longitude: -122.4194
            zoom: 12
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
        }
    }
}