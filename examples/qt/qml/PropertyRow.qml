import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * PropertyRow - A single row in the property editor
 * Automatically chooses the right editor based on field type
 */
Rectangle {
    id: root
    height: 32
    color: "transparent"
    
    required property string fieldName
    required property string fieldType
    required property var fieldValue
    
    signal valueChanged(var newValue)
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        spacing: 8
        
        // Field name label
        Text {
            Layout.preferredWidth: 80
            text: fieldName
            color: "#cccccc"
            font.pixelSize: 11
            elide: Text.ElideRight
            
            ToolTip.visible: mouseArea.containsMouse
            ToolTip.text: fieldName + " (" + fieldType + ")"
            
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }
        
        // Value editor - chosen based on type
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                switch (fieldType) {
                    case "bool": return boolEditor
                    case "int": return intEditor
                    case "float":
                    case "double": return floatEditor
                    case "string": return stringEditor
                    default: return readonlyEditor
                }
            }
        }
    }
    
    // Boolean editor (checkbox)
    Component {
        id: boolEditor
        CheckBox {
            checked: fieldValue === true
            onCheckedChanged: {
                if (checked !== fieldValue) {
                    root.valueChanged(checked)
                }
            }
        }
    }
    
    // Integer editor
    Component {
        id: intEditor
        TextField {
            text: fieldValue !== undefined ? Math.round(fieldValue).toString() : "0"
            color: "#ffffff"
            font.pixelSize: 11
            horizontalAlignment: Text.AlignRight
            validator: IntValidator {}
            
            background: Rectangle {
                color: parent.activeFocus ? "#555555" : "#444444"
                radius: 3
            }
            
            onEditingFinished: {
                var val = parseInt(text)
                if (!isNaN(val) && val !== fieldValue) {
                    root.valueChanged(val)
                }
            }
        }
    }
    
    // Float/double editor
    Component {
        id: floatEditor
        TextField {
            text: fieldValue !== undefined ? fieldValue.toFixed(2) : "0.00"
            color: "#ffffff"
            font.pixelSize: 11
            horizontalAlignment: Text.AlignRight
            validator: DoubleValidator {}
            
            background: Rectangle {
                color: parent.activeFocus ? "#555555" : "#444444"
                radius: 3
            }
            
            onEditingFinished: {
                var val = parseFloat(text)
                if (!isNaN(val) && Math.abs(val - fieldValue) > 0.0001) {
                    root.valueChanged(val)
                }
            }
        }
    }
    
    // String editor
    Component {
        id: stringEditor
        TextField {
            text: fieldValue || ""
            color: "#ffffff"
            font.pixelSize: 11
            
            background: Rectangle {
                color: parent.activeFocus ? "#555555" : "#444444"
                radius: 3
            }
            
            onEditingFinished: {
                if (text !== fieldValue) {
                    root.valueChanged(text)
                }
            }
        }
    }
    
    // Read-only display for unknown types
    Component {
        id: readonlyEditor
        Text {
            text: fieldValue !== undefined ? String(fieldValue) : "N/A"
            color: "#888888"
            font.pixelSize: 11
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }
    }
}
