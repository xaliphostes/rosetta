import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Rosetta 1.0

ApplicationWindow {
    id: window
    width: 900
    height: 600
    visible: true
    title: "Rosetta QML Demo - Dynamic Property Editor"
    color: "#1e1e1e"

    // The QmlBridge - fully type-erased property access
    QmlBridge {
        id: connector
    }

    // Track which object is selected
    property var selectedObject: null
    property string selectedClassName: ""

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Left panel - Canvas with shapes
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#2d2d2d"
            // Use border.radius for older Qt or radius for newer
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    text: "Canvas"
                    color: "#ffffff"
                    font.bold: true
                    font.pixelSize: 16
                }

                // Drawing area
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#3d3d3d"
                    radius: 5
                    clip: true

                    // Circle shape
                    Rectangle {
                        id: circleVisual
                        x: exampleCircle.x - width/2
                        y: exampleCircle.y - height/2
                        width: exampleCircle.radius * 2
                        height: exampleCircle.radius * 2
                        radius: exampleCircle.radius
                        color: exampleCircle.color
                        visible: exampleCircle.visible
                        rotation: exampleCircle.rotation
                        border.width: selectedClassName === "ShapeCircle" ? 3 : 0
                        border.color: "#ffffff"

                        MouseArea {
                            anchors.fill: parent
                            drag.target: parent
                            onClicked: selectObject(exampleCircle, "ShapeCircle")
                            onPositionChanged: {
                                if (drag.active) {
                                    exampleCircle.x = circleVisual.x + circleVisual.width/2
                                    exampleCircle.y = circleVisual.y + circleVisual.height/2
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: exampleCircle.name
                            color: "#ffffff"
                            font.pixelSize: 12
                        }
                    }

                    // Rectangle shape
                    Rectangle {
                        id: rectVisual
                        x: exampleRect.x - width/2
                        y: exampleRect.y - height/2
                        width: exampleRect.width
                        height: exampleRect.height
                        color: exampleRect.color
                        visible: exampleRect.visible
                        rotation: exampleRect.rotation
                        border.width: selectedClassName === "ShapeRectangle" ? 3 : 0
                        border.color: "#ffffff"

                        MouseArea {
                            anchors.fill: parent
                            drag.target: parent
                            onClicked: selectObject(exampleRect, "ShapeRectangle")
                            onPositionChanged: {
                                if (drag.active) {
                                    exampleRect.x = rectVisual.x + rectVisual.width/2
                                    exampleRect.y = rectVisual.y + rectVisual.height/2
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: exampleRect.name
                            color: "#ffffff"
                            font.pixelSize: 12
                        }
                    }

                    // Click on empty space to deselect
                    MouseArea {
                        anchors.fill: parent
                        z: -1
                        onClicked: {
                            selectedObject = null
                            selectedClassName = ""
                            connector.clearObject()
                        }
                    }
                }

                // Quick select buttons
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: "Select Circle"
                        onClicked: selectObject(exampleCircle, "ShapeCircle")
                    }
                    Button {
                        text: "Select Rectangle"
                        onClicked: selectObject(exampleRect, "ShapeRectangle")
                    }
                    Button {
                        text: "Deselect"
                        onClicked: {
                            selectedObject = null
                            selectedClassName = ""
                            connector.clearObject()
                        }
                    }
                }
            }
        }

        // Right panel - Property editor
        Rectangle {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            color: "#2d2d2d"
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                // Header
                Text {
                    text: "Property Inspector"
                    color: "#ffffff"
                    font.bold: true
                    font.pixelSize: 16
                }

                // Class info
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: "#3d3d3d"
                    radius: 5

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: connector.hasObject ? connector.className : "No selection"
                            color: "#ffffff"
                            font.bold: true
                        }
                        Text {
                            visible: connector.hasObject
                            text: connector.fieldNames.length + " properties, " + 
                                  connector.methodNames.length + " methods"
                            color: "#888888"
                            font.pixelSize: 11
                        }
                    }
                }

                // Properties list
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#3d3d3d"
                    radius: 5
                    visible: connector.hasObject

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 5
                        clip: true

                        ColumnLayout {
                            width: parent.width - 10
                            spacing: 4

                            Repeater {
                                model: connector.fieldNames

                                delegate: PropertyRow {
                                    id: propertyRowDelegate
                                    Layout.fillWidth: true
                                    
                                    // Capture modelData in a local property
                                    readonly property string currentFieldName: modelData
                                    
                                    fieldName: currentFieldName
                                    fieldType: connector.getFieldType(currentFieldName)
                                    fieldValue: connector.getField(currentFieldName)
                                    onValueChanged: function(newValue) {
                                        connector.setField(currentFieldName, newValue)
                                    }
                                }
                            }
                        }
                    }
                }

                // Methods section
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#3d3d3d"
                    radius: 5
                    visible: connector.hasObject

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 5

                        Text {
                            text: "Methods"
                            color: "#ffffff"
                            font.bold: true
                        }

                        Flow {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 5

                            Repeater {
                                model: connector.methodNames

                                delegate: Button {
                                    text: modelData
                                    font.pixelSize: 11
                                    padding: 6

                                    property var info: connector.getMethodInfo(modelData)

                                    ToolTip.visible: hovered
                                    ToolTip.text: info.returnType + " " + modelData + 
                                                  "(" + info.argTypes.join(", ") + ")"

                                    onClicked: {
                                        if (info.arity === 0) {
                                            var result = connector.invokeMethod(modelData, [])
                                            if (info.returnType !== "void") {
                                                console.log(modelData + "() = " + result)
                                            }
                                            // Refresh connector to show updated values
                                            connector.setObject(selectedClassName, selectedObject)
                                        } else {
                                            console.log("Method " + modelData + " requires " + 
                                                       info.arity + " argument(s)")
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Registered classes info
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "#3d3d3d"
                    radius: 5

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            text: "Registered Classes"
                            color: "#ffffff"
                            font.bold: true
                            font.pixelSize: 11
                        }
                        Text {
                            Layout.fillWidth: true
                            text: connector.registeredClasses().join(", ")
                            color: "#888888"
                            font.pixelSize: 10
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }

    // Helper function to select an object
    function selectObject(obj, className) {
        selectedObject = obj
        selectedClassName = className
        connector.setObject(className, obj)
    }

    // Refresh connector when object properties change externally
    Connections {
        target: exampleCircle
        function onXChanged() { if (selectedClassName === "ShapeCircle") refreshConnector() }
        function onYChanged() { if (selectedClassName === "ShapeCircle") refreshConnector() }
    }

    Connections {
        target: exampleRect
        function onXChanged() { if (selectedClassName === "ShapeRectangle") refreshConnector() }
        function onYChanged() { if (selectedClassName === "ShapeRectangle") refreshConnector() }
    }

    function refreshConnector() {
        if (selectedObject && selectedClassName) {
            connector.setObject(selectedClassName, selectedObject)
        }
    }
}
