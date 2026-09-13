import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// The UI panel. `controller` is a C++ QObject exposed as a context property
// from MainWindow; assigning to its properties reaches the renderer directly.
Rectangle {
    id: root

    color: palette.window

    // Kept in sync with the controller so the swatch and the renderer agree.
    function pushColor() {
        controller.clearColor = Qt.rgba(redSlider.value,
                                        greenSlider.value,
                                        blueSlider.value,
                                        1.0)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: qsTr("Clear Colour")
            font.bold: true
            font.pointSize: 12
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            opacity: 0.7
            text: qsTr("Drives the Vulkan render pass clear value, proving the QML to renderer path.")
        }

        GridLayout {
            columns: 3
            columnSpacing: 8
            rowSpacing: 6
            Layout.fillWidth: true

            Label { text: qsTr("R") }
            Slider {
                id: redSlider
                Layout.fillWidth: true
                from: 0.0
                to: 1.0
                value: 0.0
                onValueChanged: root.pushColor()
            }
            Label {
                text: redSlider.value.toFixed(2)
                Layout.minimumWidth: 34
            }

            Label { text: qsTr("G") }
            Slider {
                id: greenSlider
                Layout.fillWidth: true
                from: 0.0
                to: 1.0
                value: 0.0
                onValueChanged: root.pushColor()
            }
            Label {
                text: greenSlider.value.toFixed(2)
                Layout.minimumWidth: 34
            }

            Label { text: qsTr("B") }
            Slider {
                id: blueSlider
                Layout.fillWidth: true
                from: 0.0
                to: 1.0
                value: 0.0
                onValueChanged: root.pushColor()
            }
            Label {
                text: blueSlider.value.toFixed(2)
                Layout.minimumWidth: 34
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            radius: 4
            border.width: 1
            border.color: Qt.rgba(0.5, 0.5, 0.5, 0.5)
            color: controller.clearColor
        }

        Button {
            text: qsTr("Reset to black")
            Layout.fillWidth: true
            onClicked: {
                redSlider.value = 0.0
                greenSlider.value = 0.0
                blueSlider.value = 0.0
            }
        }

        Item { Layout.fillHeight: true }
    }
}
