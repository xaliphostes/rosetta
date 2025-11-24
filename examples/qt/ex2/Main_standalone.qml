// ============================================================================
// Standalone QML 3D Demo (can run with: qml Main_standalone.qml)
// This version uses pure QML without C++ backend for quick testing
// ============================================================================
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

ApplicationWindow {
    id: root
    width: 1400
    height: 900
    visible: true
    title: "Qt6 QML 3D Property Editor Demo"
    color: "#1e1e1e"

    // Scene objects data model
    ListModel {
        id: sceneObjects

        // Default objects
        ListElement {
            name: "Sun"
            objectType: "light"
            lightType: 0  // Directional
            visible: true
            posX: 0; posY: 5; posZ: 5
            rotX: -45; rotY: 45; rotZ: 0
            scaX: 1; scaY: 1; scaZ: 1
            colR: 1.0; colG: 1.0; colB: 0.9
            metalness: 0; roughness: 0.5
            intensity: 1.0
            castShadows: true
        }
        ListElement {
            name: "Cube"
            objectType: "primitive"
            primitiveType: 0  // Cube
            visible: true
            posX: 0; posY: 0; posZ: 0
            rotX: 0; rotY: 0; rotZ: 0
            scaX: 1; scaY: 1; scaZ: 1
            colR: 0.2; colG: 0.6; colB: 0.9
            metalness: 0; roughness: 0.5
            intensity: 1.0
            castShadows: true
        }
        ListElement {
            name: "Sphere"
            objectType: "primitive"
            primitiveType: 1  // Sphere
            visible: true
            posX: 2; posY: 0.5; posZ: 0
            rotX: 0; rotY: 0; rotZ: 0
            scaX: 1; scaY: 1; scaZ: 1
            colR: 0.9; colG: 0.3; colB: 0.3
            metalness: 0.8; roughness: 0.2
            intensity: 1.0
            castShadows: true
        }
        ListElement {
            name: "Cylinder"
            objectType: "primitive"
            primitiveType: 2  // Cylinder
            visible: true
            posX: -2; posY: 0; posZ: 0
            rotX: 0; rotY: 0; rotZ: 0
            scaX: 0.5; scaY: 1.5; scaZ: 0.5
            colR: 0.3; colG: 0.9; colB: 0.4
            metalness: 0; roughness: 0.7
            intensity: 1.0
            castShadows: true
        }
    }

    property int selectedIndex: -1
    property var selectedObject: selectedIndex >= 0 ? sceneObjects.get(selectedIndex) : null
    property bool showGrid: true
    property bool showAxes: true

    // Main layout
    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left panel - Object hierarchy
        Rectangle {
            SplitView.preferredWidth: 200
            SplitView.minimumWidth: 150
            color: "#2d2d2d"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Label {
                    text: "Scene Hierarchy"
                    font.bold: true
                    font.pixelSize: 14
                    color: "#ffffff"
                }

                // Add buttons
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Button {
                        text: "+"
                        Layout.preferredWidth: 30
                        onClicked: addMenu.open()

                        Menu {
                            id: addMenu
                            MenuItem { text: "Cube"; onTriggered: addPrimitive(0, "Cube") }
                            MenuItem { text: "Sphere"; onTriggered: addPrimitive(1, "Sphere") }
                            MenuItem { text: "Cylinder"; onTriggered: addPrimitive(2, "Cylinder") }
                            MenuItem { text: "Cone"; onTriggered: addPrimitive(3, "Cone") }
                            MenuSeparator {}
                            MenuItem { text: "Point Light"; onTriggered: addLight(1, "PointLight") }
                            MenuItem { text: "Spot Light"; onTriggered: addLight(2, "SpotLight") }
                        }
                    }

                    Button {
                        text: "-"
                        Layout.preferredWidth: 30
                        enabled: selectedIndex >= 0
                        onClicked: {
                            sceneObjects.remove(selectedIndex)
                            selectedIndex = -1
                        }
                    }

                    Item { Layout.fillWidth: true }
                }

                // Object list
                ListView {
                    id: objectList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: sceneObjects

                    delegate: ItemDelegate {
                        width: objectList.width
                        height: 32
                        highlighted: index === selectedIndex

                        background: Rectangle {
                            color: highlighted ? "#0078d4" : (hovered ? "#3d3d3d" : "transparent")
                            radius: 3
                        }

                        contentItem: RowLayout {
                            spacing: 8

                            CheckBox {
                                checked: model.visible
                                onCheckedChanged: model.visible = checked
                            }

                            Label {
                                text: model.name
                                color: "#ffffff"
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: model.objectType === "light" ? "💡" : "📦"
                                font.pixelSize: 12
                            }
                        }

                        onClicked: selectedIndex = index
                    }
                }
            }
        }

        // Center - 3D View
        Rectangle {
            SplitView.fillWidth: true
            color: "#1a1a1a"

            View3D {
                id: view3d
                anchors.fill: parent

                environment: SceneEnvironment {
                    clearColor: "#2a2a3a"
                    backgroundMode: SceneEnvironment.Color
                    antialiasingMode: SceneEnvironment.MSAA
                    antialiasingQuality: SceneEnvironment.High
                }

                // Camera
                PerspectiveCamera {
                    id: camera
                    position: Qt.vector3d(5, 4, 8)
                    eulerRotation: Qt.vector3d(-20, 30, 0)
                    fieldOfView: 60
                    clipNear: 0.1
                    clipFar: 1000
                }

                // Ambient light
                DirectionalLight {
                    ambientColor: Qt.rgba(0.2, 0.2, 0.25, 1.0)
                }

                // Ground plane
                Model {
                    visible: showGrid
                    source: "#Rectangle"
                    scale: Qt.vector3d(20, 20, 1)
                    eulerRotation.x: -90
                    position: Qt.vector3d(0, -0.01, 0)
                    materials: DefaultMaterial {
                        diffuseColor: "#404050"
                    }
                }

                // Grid lines
                Repeater3D {
                    model: 21
                    visible: showGrid

                    Model {
                        source: "#Cube"
                        scale: Qt.vector3d(20, 0.01, 0.01)
                        position: Qt.vector3d(0, 0, index - 10)
                        materials: DefaultMaterial {
                            diffuseColor: "#555555"
                        }
                    }
                }

                Repeater3D {
                    model: 21
                    visible: showGrid

                    Model {
                        source: "#Cube"
                        scale: Qt.vector3d(0.01, 0.01, 20)
                        position: Qt.vector3d(index - 10, 0, 0)
                        materials: DefaultMaterial {
                            diffuseColor: "#555555"
                        }
                    }
                }

                // Axes
                Node {
                    visible: showAxes

                    // X axis (red)
                    Model {
                        source: "#Cylinder"
                        scale: Qt.vector3d(0.02, 3, 0.02)
                        position: Qt.vector3d(1.5, 0, 0)
                        eulerRotation.z: -90
                        materials: DefaultMaterial { diffuseColor: "#ff4444" }
                    }

                    // Y axis (green)
                    Model {
                        source: "#Cylinder"
                        scale: Qt.vector3d(0.02, 3, 0.02)
                        position: Qt.vector3d(0, 1.5, 0)
                        materials: DefaultMaterial { diffuseColor: "#44ff44" }
                    }

                    // Z axis (blue)
                    Model {
                        source: "#Cylinder"
                        scale: Qt.vector3d(0.02, 3, 0.02)
                        position: Qt.vector3d(0, 0, 1.5)
                        eulerRotation.x: 90
                        materials: DefaultMaterial { diffuseColor: "#4444ff" }
                    }
                }

                // Scene objects
                Repeater3D {
                    model: sceneObjects

                    delegate: Loader3D {
                        sourceComponent: model.objectType === "primitive" ? primitiveComp : lightComp
                        property int objIndex: index
                    }
                }

                // Primitive component
                Component {
                    id: primitiveComp

                    Model {
                        property var obj: sceneObjects.get(objIndex)

                        visible: obj ? obj.visible : false
                        position: obj ? Qt.vector3d(obj.posX, obj.posY, obj.posZ) : Qt.vector3d(0,0,0)
                        eulerRotation: obj ? Qt.vector3d(obj.rotX, obj.rotY, obj.rotZ) : Qt.vector3d(0,0,0)
                        scale: obj ? Qt.vector3d(obj.scaX, obj.scaY, obj.scaZ) : Qt.vector3d(1,1,1)

                        source: {
                            if (!obj) return "#Cube"
                            switch (obj.primitiveType) {
                                case 0: return "#Cube"
                                case 1: return "#Sphere"
                                case 2: return "#Cylinder"
                                case 3: return "#Cone"
                                default: return "#Cube"
                            }
                        }

                        materials: PrincipledMaterial {
                            baseColor: obj ? Qt.rgba(obj.colR, obj.colG, obj.colB, 1) : "#808080"
                            metalness: obj ? obj.metalness : 0
                            roughness: obj ? obj.roughness : 0.5
                        }

                        // Selection highlight outline
                        Model {
                            visible: objIndex === selectedIndex
                            source: parent.source
                            scale: Qt.vector3d(1.05, 1.05, 1.05)
                            materials: DefaultMaterial {
                                diffuseColor: "#ffff00"
                                opacity: 0.3
                            }
                        }
                    }
                }

                // Light component
                Component {
                    id: lightComp

                    Node {
                        property var obj: sceneObjects.get(objIndex)

                        visible: obj ? obj.visible : false
                        position: obj ? Qt.vector3d(obj.posX, obj.posY, obj.posZ) : Qt.vector3d(0,0,0)
                        eulerRotation: obj ? Qt.vector3d(obj.rotX, obj.rotY, obj.rotZ) : Qt.vector3d(0,0,0)

                        // Directional light
                        DirectionalLight {
                            visible: obj && obj.lightType === 0
                            color: obj ? Qt.rgba(obj.colR, obj.colG, obj.colB, 1) : "#ffffff"
                            brightness: obj ? obj.intensity : 1
                            castsShadow: obj ? obj.castShadows : true
                            shadowMapQuality: Light.ShadowMapQualityHigh
                        }

                        // Point light
                        PointLight {
                            visible: obj && obj.lightType === 1
                            color: obj ? Qt.rgba(obj.colR, obj.colG, obj.colB, 1) : "#ffffff"
                            brightness: obj ? obj.intensity : 1
                            castsShadow: obj ? obj.castShadows : true
                        }

                        // Spot light
                        SpotLight {
                            visible: obj && obj.lightType === 2
                            color: obj ? Qt.rgba(obj.colR, obj.colG, obj.colB, 1) : "#ffffff"
                            brightness: obj ? obj.intensity : 1
                            coneAngle: 45
                            castsShadow: obj ? obj.castShadows : true
                        }

                        // Light gizmo
                        Model {
                            source: "#Sphere"
                            scale: Qt.vector3d(0.15, 0.15, 0.15)
                            materials: DefaultMaterial {
                                diffuseColor: obj ? Qt.rgba(obj.colR, obj.colG, obj.colB, 1) : "#ffff00"
                                lighting: DefaultMaterial.NoLighting
                            }
                        }
                    }
                }
            }

            // Camera controller
            WasdController {
                controlledObject: camera
                speed: 0.3
                shiftSpeed: 1.5
            }

            // View controls
            RowLayout {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 10
                spacing: 5

                Button {
                    text: showGrid ? "Hide Grid" : "Show Grid"
                    onClicked: showGrid = !showGrid
                }

                Button {
                    text: showAxes ? "Hide Axes" : "Show Axes"
                    onClicked: showAxes = !showAxes
                }

                Button {
                    text: "Reset Camera"
                    onClicked: {
                        camera.position = Qt.vector3d(5, 4, 8)
                        camera.eulerRotation = Qt.vector3d(-20, 30, 0)
                    }
                }
            }

            // Info
            Label {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.margins: 10
                text: "WASD: Move | Shift: Fast | Mouse: Look | Objects: " + sceneObjects.count
                color: "#666666"
                font.pixelSize: 11
            }
        }

        // Right panel - Property editor
        Rectangle {
            SplitView.preferredWidth: 300
            SplitView.minimumWidth: 250
            color: "#2d2d2d"

            ScrollView {
                anchors.fill: parent
                contentWidth: availableWidth

                ColumnLayout {
                    width: parent.width
                    spacing: 0

                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 45
                        color: "#252525"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10

                            Label {
                                text: selectedObject ? selectedObject.name : "No Selection"
                                font.bold: true
                                font.pixelSize: 14
                                color: "#ffffff"
                                Layout.fillWidth: true
                            }

                            // Rename button
                            Button {
                                text: "✏️"
                                visible: selectedObject !== null
                                flat: true
                                onClicked: renameDialog.open()
                            }
                        }
                    }

                    // Properties
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.margins: 10
                        spacing: 10
                        visible: selectedObject !== null

                        // Transform section
                        PropertySectionStandalone {
                            title: "Transform"
                            Layout.fillWidth: true

                            GridLayout {
                                columns: 4
                                columnSpacing: 5
                                rowSpacing: 8
                                Layout.fillWidth: true

                                // Position
                                Label { text: "Position"; color: "#aaa"; font.bold: true; Layout.columnSpan: 4 }

                                Label { text: "X"; color: "#ff6666" }
                                SpinBox {
                                    from: -10000; to: 10000; stepSize: 10
                                    value: selectedObject ? selectedObject.posX * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.posX = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Y"; color: "#66ff66" }
                                SpinBox {
                                    from: -10000; to: 10000; stepSize: 10
                                    value: selectedObject ? selectedObject.posY * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.posY = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }

                                Label { text: "Z"; color: "#6666ff" }
                                SpinBox {
                                    from: -10000; to: 10000; stepSize: 10
                                    value: selectedObject ? selectedObject.posZ * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.posZ = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Item { Layout.columnSpan: 2 }

                                // Rotation
                                Label { text: "Rotation"; color: "#aaa"; font.bold: true; Layout.columnSpan: 4 }

                                Label { text: "X"; color: "#ff6666" }
                                SpinBox {
                                    from: -36000; to: 36000; stepSize: 500
                                    value: selectedObject ? selectedObject.rotX * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.rotX = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Y"; color: "#66ff66" }
                                SpinBox {
                                    from: -36000; to: 36000; stepSize: 500
                                    value: selectedObject ? selectedObject.rotY * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.rotY = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }

                                Label { text: "Z"; color: "#6666ff" }
                                SpinBox {
                                    from: -36000; to: 36000; stepSize: 500
                                    value: selectedObject ? selectedObject.rotZ * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.rotZ = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Item { Layout.columnSpan: 2 }

                                // Scale
                                Label { text: "Scale"; color: "#aaa"; font.bold: true; Layout.columnSpan: 4 }

                                Label { text: "X"; color: "#ff6666" }
                                SpinBox {
                                    from: 1; to: 10000; stepSize: 10
                                    value: selectedObject ? selectedObject.scaX * 100 : 100
                                    onValueModified: if (selectedObject) selectedObject.scaX = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Y"; color: "#66ff66" }
                                SpinBox {
                                    from: 1; to: 10000; stepSize: 10
                                    value: selectedObject ? selectedObject.scaY * 100 : 100
                                    onValueModified: if (selectedObject) selectedObject.scaY = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }

                                Label { text: "Z"; color: "#6666ff" }
                                SpinBox {
                                    from: 1; to: 10000; stepSize: 10
                                    value: selectedObject ? selectedObject.scaZ * 100 : 100
                                    onValueModified: if (selectedObject) selectedObject.scaZ = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Item { Layout.columnSpan: 2 }
                            }

                            Button {
                                text: "Reset Transform"
                                Layout.fillWidth: true
                                onClicked: {
                                    if (selectedObject) {
                                        selectedObject.posX = 0; selectedObject.posY = 0; selectedObject.posZ = 0
                                        selectedObject.rotX = 0; selectedObject.rotY = 0; selectedObject.rotZ = 0
                                        selectedObject.scaX = 1; selectedObject.scaY = 1; selectedObject.scaZ = 1
                                    }
                                }
                            }
                        }

                        // Material section (for primitives)
                        PropertySectionStandalone {
                            title: "Material"
                            Layout.fillWidth: true
                            visible: selectedObject && selectedObject.objectType === "primitive"

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // Color preview and sliders
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Rectangle {
                                        width: 50
                                        height: 50
                                        radius: 5
                                        color: selectedObject ?
                                            Qt.rgba(selectedObject.colR, selectedObject.colG, selectedObject.colB, 1) :
                                            "#808080"
                                        border.color: "#555"
                                        border.width: 1
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4

                                        // Red
                                        RowLayout {
                                            Label { text: "R"; color: "#ff6666"; Layout.preferredWidth: 15 }
                                            Slider {
                                                Layout.fillWidth: true
                                                from: 0; to: 1
                                                value: selectedObject ? selectedObject.colR : 0.5
                                                onMoved: if (selectedObject) selectedObject.colR = value
                                            }
                                        }

                                        // Green
                                        RowLayout {
                                            Label { text: "G"; color: "#66ff66"; Layout.preferredWidth: 15 }
                                            Slider {
                                                Layout.fillWidth: true
                                                from: 0; to: 1
                                                value: selectedObject ? selectedObject.colG : 0.5
                                                onMoved: if (selectedObject) selectedObject.colG = value
                                            }
                                        }

                                        // Blue
                                        RowLayout {
                                            Label { text: "B"; color: "#6666ff"; Layout.preferredWidth: 15 }
                                            Slider {
                                                Layout.fillWidth: true
                                                from: 0; to: 1
                                                value: selectedObject ? selectedObject.colB : 0.5
                                                onMoved: if (selectedObject) selectedObject.colB = value
                                            }
                                        }
                                    }
                                }

                                // Metalness
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Metalness"; color: "#aaa"; Layout.preferredWidth: 70 }
                                    Slider {
                                        Layout.fillWidth: true
                                        from: 0; to: 1
                                        value: selectedObject ? selectedObject.metalness : 0
                                        onMoved: if (selectedObject) selectedObject.metalness = value
                                    }
                                    Label {
                                        text: selectedObject ? selectedObject.metalness.toFixed(2) : "0.00"
                                        color: "#888"
                                        Layout.preferredWidth: 35
                                    }
                                }

                                // Roughness
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Roughness"; color: "#aaa"; Layout.preferredWidth: 70 }
                                    Slider {
                                        Layout.fillWidth: true
                                        from: 0; to: 1
                                        value: selectedObject ? selectedObject.roughness : 0.5
                                        onMoved: if (selectedObject) selectedObject.roughness = value
                                    }
                                    Label {
                                        text: selectedObject ? selectedObject.roughness.toFixed(2) : "0.50"
                                        color: "#888"
                                        Layout.preferredWidth: 35
                                    }
                                }

                                // Presets
                                Label { text: "Presets"; color: "#aaa" }
                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 5

                                    Button {
                                        text: "Metal"
                                        onClicked: {
                                            if (selectedObject) {
                                                selectedObject.metalness = 1.0
                                                selectedObject.roughness = 0.2
                                            }
                                        }
                                    }
                                    Button {
                                        text: "Plastic"
                                        onClicked: {
                                            if (selectedObject) {
                                                selectedObject.metalness = 0.0
                                                selectedObject.roughness = 0.4
                                            }
                                        }
                                    }
                                    Button {
                                        text: "Rubber"
                                        onClicked: {
                                            if (selectedObject) {
                                                selectedObject.metalness = 0.0
                                                selectedObject.roughness = 0.9
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Light section
                        PropertySectionStandalone {
                            title: "Light"
                            Layout.fillWidth: true
                            visible: selectedObject && selectedObject.objectType === "light"

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // Light type
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Type"; color: "#aaa"; Layout.preferredWidth: 70 }
                                    ComboBox {
                                        Layout.fillWidth: true
                                        model: ["Directional", "Point", "Spot"]
                                        currentIndex: selectedObject ? selectedObject.lightType : 0
                                        onCurrentIndexChanged: if (selectedObject) selectedObject.lightType = currentIndex
                                    }
                                }

                                // Intensity
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Intensity"; color: "#aaa"; Layout.preferredWidth: 70 }
                                    Slider {
                                        Layout.fillWidth: true
                                        from: 0; to: 5
                                        value: selectedObject ? selectedObject.intensity : 1
                                        onMoved: if (selectedObject) selectedObject.intensity = value
                                    }
                                    Label {
                                        text: selectedObject ? selectedObject.intensity.toFixed(2) : "1.00"
                                        color: "#888"
                                        Layout.preferredWidth: 35
                                    }
                                }

                                // Color
                                Label { text: "Color"; color: "#aaa" }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        radius: 3
                                        color: selectedObject ?
                                            Qt.rgba(selectedObject.colR, selectedObject.colG, selectedObject.colB, 1) :
                                            "#ffffff"
                                        border.color: "#555"
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Slider {
                                            Layout.fillWidth: true
                                            from: 0; to: 1
                                            value: selectedObject ? selectedObject.colR : 1
                                            onMoved: if (selectedObject) selectedObject.colR = value
                                        }
                                        Slider {
                                            Layout.fillWidth: true
                                            from: 0; to: 1
                                            value: selectedObject ? selectedObject.colG : 1
                                            onMoved: if (selectedObject) selectedObject.colG = value
                                        }
                                        Slider {
                                            Layout.fillWidth: true
                                            from: 0; to: 1
                                            value: selectedObject ? selectedObject.colB : 1
                                            onMoved: if (selectedObject) selectedObject.colB = value
                                        }
                                    }
                                }

                                // Cast shadows
                                CheckBox {
                                    text: "Cast Shadows"
                                    checked: selectedObject ? selectedObject.castShadows : true
                                    onCheckedChanged: if (selectedObject) selectedObject.castShadows = checked
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }

                    // No selection
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: selectedObject === null

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 10

                            Label {
                                text: "📦"
                                font.pixelSize: 48
                                Layout.alignment: Qt.AlignHCenter
                            }

                            Label {
                                text: "Select an object to\nedit its properties"
                                color: "#666666"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
            }
        }
    }

    // Rename dialog
    Dialog {
        id: renameDialog
        title: "Rename Object"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: parent

        TextField {
            id: renameField
            width: 200
            text: selectedObject ? selectedObject.name : ""
        }

        onAccepted: {
            if (selectedObject && renameField.text.length > 0) {
                selectedObject.name = renameField.text
            }
        }

        onAboutToShow: {
            renameField.text = selectedObject ? selectedObject.name : ""
            renameField.selectAll()
        }
    }

    // Helper functions
    function addPrimitive(type, baseName) {
        sceneObjects.append({
            name: baseName + "_" + sceneObjects.count,
            objectType: "primitive",
            primitiveType: type,
            visible: true,
            posX: Math.random() * 4 - 2,
            posY: 0.5,
            posZ: Math.random() * 4 - 2,
            rotX: 0, rotY: 0, rotZ: 0,
            scaX: 1, scaY: 1, scaZ: 1,
            colR: Math.random(),
            colG: Math.random(),
            colB: Math.random(),
            metalness: 0,
            roughness: 0.5,
            intensity: 1.0,
            castShadows: true
        })
        selectedIndex = sceneObjects.count - 1
    }

    function addLight(type, baseName) {
        sceneObjects.append({
            name: baseName + "_" + sceneObjects.count,
            objectType: "light",
            lightType: type,
            visible: true,
            posX: Math.random() * 4 - 2,
            posY: 3,
            posZ: Math.random() * 4 - 2,
            rotX: -45, rotY: 0, rotZ: 0,
            scaX: 1, scaY: 1, scaZ: 1,
            colR: 1.0, colG: 1.0, colB: 1.0,
            metalness: 0,
            roughness: 0.5,
            intensity: 1.0,
            castShadows: true
        })
        selectedIndex = sceneObjects.count - 1
    }

    // Inline PropertySection component for standalone version
    component PropertySectionStandalone: Rectangle {
        property string title: "Section"
        property bool expanded: true
        default property alias content: contentColumn.children

        Layout.fillWidth: true
        implicitHeight: headerRow.height + (expanded ? contentColumn.height + 15 : 0)
        color: "#252525"
        radius: 5
        clip: true

        Behavior on implicitHeight {
            NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: headerRow
                Layout.fillWidth: true
                height: 32
                color: headerMouse.containsMouse ? "#3d3d3d" : "#303030"
                radius: 5

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Label {
                        text: expanded ? "▼" : "▶"
                        color: "#888888"
                        font.pixelSize: 10
                    }

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

            ColumnLayout {
                id: contentColumn
                Layout.fillWidth: true
                Layout.margins: 10
                spacing: 10
                visible: expanded
            }
        }
    }
}
