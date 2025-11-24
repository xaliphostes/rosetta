// ============================================================================
// ColorDialog.qml - Simple color picker dialog
// ============================================================================
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    property color color: "#808080"

    title: "Select Color"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 350
    height: 400

    onAccepted: {
        color = Qt.rgba(redSlider.value, greenSlider.value, blueSlider.value, 1)
    }

    onAboutToShow: {
        redSlider.value = color.r
        greenSlider.value = color.g
        blueSlider.value = color.b
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        // Color preview
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Qt.rgba(redSlider.value, greenSlider.value, blueSlider.value, 1)
            border.color: "#333"
            border.width: 1
            radius: 5

            Label {
                anchors.centerIn: parent
                text: Qt.rgba(redSlider.value, greenSlider.value, blueSlider.value, 1).toString()
                color: (redSlider.value + greenSlider.value + blueSlider.value) > 1.5 ? "#000" : "#fff"
                font.family: "monospace"
            }
        }

        // RGB Sliders
        GridLayout {
            columns: 3
            rowSpacing: 10
            columnSpacing: 10
            Layout.fillWidth: true

            // Red
            Label { text: "R"; color: "#ff6666"; font.bold: true }
            Slider {
                id: redSlider
                Layout.fillWidth: true
                from: 0; to: 1
                value: 0.5

                background: Rectangle {
                    x: redSlider.leftPadding
                    y: redSlider.topPadding + redSlider.availableHeight / 2 - height / 2
                    width: redSlider.availableWidth
                    height: 8
                    radius: 4

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: Qt.rgba(0, greenSlider.value, blueSlider.value, 1) }
                        GradientStop { position: 1.0; color: Qt.rgba(1, greenSlider.value, blueSlider.value, 1) }
                    }
                }
            }
            Label {
                text: Math.round(redSlider.value * 255)
                Layout.preferredWidth: 30
                horizontalAlignment: Text.AlignRight
            }

            // Green
            Label { text: "G"; color: "#66ff66"; font.bold: true }
            Slider {
                id: greenSlider
                Layout.fillWidth: true
                from: 0; to: 1
                value: 0.5

                background: Rectangle {
                    x: greenSlider.leftPadding
                    y: greenSlider.topPadding + greenSlider.availableHeight / 2 - height / 2
                    width: greenSlider.availableWidth
                    height: 8
                    radius: 4

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: Qt.rgba(redSlider.value, 0, blueSlider.value, 1) }
                        GradientStop { position: 1.0; color: Qt.rgba(redSlider.value, 1, blueSlider.value, 1) }
                    }
                }
            }
            Label {
                text: Math.round(greenSlider.value * 255)
                Layout.preferredWidth: 30
                horizontalAlignment: Text.AlignRight
            }

            // Blue
            Label { text: "B"; color: "#6666ff"; font.bold: true }
            Slider {
                id: blueSlider
                Layout.fillWidth: true
                from: 0; to: 1
                value: 0.5

                background: Rectangle {
                    x: blueSlider.leftPadding
                    y: blueSlider.topPadding + blueSlider.availableHeight / 2 - height / 2
                    width: blueSlider.availableWidth
                    height: 8
                    radius: 4

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: Qt.rgba(redSlider.value, greenSlider.value, 0, 1) }
                        GradientStop { position: 1.0; color: Qt.rgba(redSlider.value, greenSlider.value, 1, 1) }
                    }
                }
            }
            Label {
                text: Math.round(blueSlider.value * 255)
                Layout.preferredWidth: 30
                horizontalAlignment: Text.AlignRight
            }
        }

        // Color presets
        Label { text: "Presets"; font.bold: true }

        GridLayout {
            columns: 8
            rowSpacing: 5
            columnSpacing: 5
            Layout.fillWidth: true

            Repeater {
                model: [
                    "#ff0000", "#ff8000", "#ffff00", "#80ff00",
                    "#00ff00", "#00ff80", "#00ffff", "#0080ff",
                    "#0000ff", "#8000ff", "#ff00ff", "#ff0080",
                    "#ffffff", "#c0c0c0", "#808080", "#404040",
                    "#000000", "#804000", "#408000", "#008040",
                    "#004080", "#400080", "#800040", "#804040"
                ]

                Rectangle {
                    width: 30
                    height: 30
                    color: modelData
                    border.color: "#333"
                    border.width: 1
                    radius: 3

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var c = Qt.color(modelData)
                            redSlider.value = c.r
                            greenSlider.value = c.g
                            blueSlider.value = c.b
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
