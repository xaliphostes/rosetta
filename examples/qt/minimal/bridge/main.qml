import QtQuick
import QtQuick.Controls
import Rosetta 1.0

ApplicationWindow {
    visible: true
    width: 300; height: 200
    title: "QmlBridge Example"

    QmlBridge { id: bridge }

    Component.onCompleted: bridge.setObject("Person", personObj)

    Column {
        anchors.centerIn: parent
        spacing: 10

        Text { text: "Name: " + bridge.getField("name") }
        Text { text: "Age: " + bridge.getField("age") }
        
        Button {
            text: "Birthday!"
            onClicked: { bridge.invokeMethod("birthday", []); bridge.objectChanged() }
        }
        
        Text { text: bridge.invokeMethod("greet", []) }
    }
}
