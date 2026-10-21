import bb.cascades 1.0

Page {
    id: root

    Container {
        layout: DockLayout {}

        Label {
            id: title
            text: qsTr("Compas BB10 Probe Suite")
            textStyle.fontSize: Font.Large
        }

        ScrollView {
            verticalLayout: VerticalLayout {}

            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill

                Label {
                    id: resultLabel
                    text: probeRunner.results
                    textStyle.fontSize: Font.Medium
                    textStyle.fontWeight: Font.Light
                    textFormat: Text.PlainText
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment: VerticalAlignment.Fill
                }
            }
        }

        Button {
            id: rerunButton
            text: qsTr("Re-run probes")
            enabled: probeRunner !== null
            onClicked: probeRunner.rerun()
        }
    }

    onCompleted: {
        // Keep the label in sync with the runner.
        resultLabel.text = probeRunner.results;
    }

    Connections {
        target: probeRunner
        onResultsChanged: {
            resultLabel.text = probeRunner.results;
        }
    }
}
