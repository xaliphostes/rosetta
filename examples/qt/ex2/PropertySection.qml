// ============================================================================
// PropertySection.qml - Collapsible property section component
// ============================================================================
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string title: "Section"
    property bool expanded: true
    default property alias content: contentColumn.children

    Layout.fillWidth: true
    implicitHeight: headerRow.height + (expanded ? contentColumn.height + 10 : 0)
    color: "#252525"
    radius: 3
    clip: true

    Behavior on implicitHeight {
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            id: headerRow
            Layout.fillWidth: true
            height: 30
            color: headerMouse.containsMouse ? "#3d3d3d" : "#303030"
            radius: 3

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 5

                // Expand/collapse arrow
                Label {
                    text: expanded ? "▼" : "▶"
                    color: "#888888"
                    font.pixelSize: 10
                }

                // Title
                Label {
                    text: title
                    color: "#ffffff"
                    font.bold: true
                    font.pixelSize: 12
                    Layout.fillWidth: true
                }
            }

            MouseArea {
                id: headerMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: expanded = !expanded
            }
        }

        // Content
        ColumnLayout {
            id: contentColumn
            Layout.fillWidth: true
            Layout.margins: 8
            spacing: 8
            visible: expanded
        }
    }
}
