// ============================================================================
// Main QML - 3D Scene Editor with Rosetta Property Editing
// ============================================================================
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import Rosetta3D 1.0

ApplicationWindow {
    id: root
    width: 1400
    height: 900
    visible: true
    title: "Rosetta 3D Scene Editor"
    color: "#1e1e1e"

    // Scene data
    Scene3D {
        id: scene
    }

    // Currently selected object
    property SceneObject3D selectedObject: null
    property int selectedIndex: -1

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
                anchors.margins: 5
                spacing: 5

                // Header
                Label {
                    text: "Scene Hierarchy"
                    font.bold: true
                    font.pixelSize: 14
                    color: "#ffffff"
                }

                // Add buttons
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Button {
                        text: "+"
                        Layout.preferredWidth: 30
                        onClicked: addMenu.open()

                        Menu {
                            id: addMenu
                            MenuItem {
                                text: "Cube"
                                onTriggered: addPrimitive(0)
                            }
                            MenuItem {
                                text: "Sphere"
                                onTriggered: addPrimitive(1)
                            }
                            MenuItem {
                                text: "Cylinder"
                                onTriggered: addPrimitive(2)
                            }
                            MenuItem {
                                text: "Cone"
                                onTriggered: addPrimitive(3)
                            }
                            MenuItem {
                                text: "Torus"
                                onTriggered: addPrimitive(4)
                            }
                            MenuItem {
                                text: "Plane"
                                onTriggered: addPrimitive(5)
                            }
                            MenuSeparator {}
                            MenuItem {
                                text: "Point Light"
                                onTriggered: addLight(1)
                            }
                            MenuItem {
                                text: "Spot Light"
                                onTriggered: addLight(2)
                            }
                        }
                    }

                    Button {
                        text: "-"
                        Layout.preferredWidth: 30
                        enabled: selectedIndex >= 0
                        onClicked: {
                            if (selectedIndex >= 0) {
                                scene.removeObject(selectedIndex)
                                selectedObject = null
                                selectedIndex = -1
                            }
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
                    model: scene.objectCount

                    delegate: ItemDelegate {
                        width: objectList.width
                        height: 30
                        highlighted: index === selectedIndex

                        background: Rectangle {
                            color: highlighted ? "#0078d4" : (hovered ? "#3d3d3d" : "transparent")
                        }

                        contentItem: RowLayout {
                            spacing: 5

                            // Visibility toggle
                            CheckBox {
                                checked: scene.objectAt(index) ? scene.objectAt(index).visible : true
                                onCheckedChanged: {
                                    var obj = scene.objectAt(index)
                                    if (obj) obj.visible = checked
                                }
                            }

                            // Object name
                            Label {
                                text: scene.objectAt(index) ? scene.objectAt(index).name : ""
                                color: "#ffffff"
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }

                        onClicked: {
                            selectedIndex = index
                            selectedObject = scene.objectAt(index)
                        }
                    }
                }

                // Camera info
                Rectangle {
                    Layout.fillWidth: true
                    height: 60
                    color: "#252525"
                    radius: 3

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 5

                        Label {
                            text: "Camera"
                            font.bold: true
                            color: "#aaaaaa"
                            font.pixelSize: 11
                        }

                        Label {
                            text: "FOV: " + scene.camera().fieldOfView.toFixed(0) + "°"
                            color: "#888888"
                            font.pixelSize: 10
                        }
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
                environment: sceneEnvironment

                SceneEnvironment {
                    id: sceneEnvironment
                    clearColor: scene.ambientColor
                    backgroundMode: SceneEnvironment.Color
                    antialiasingMode: SceneEnvironment.MSAA
                    antialiasingQuality: SceneEnvironment.High
                }

                // Camera
                PerspectiveCamera {
                    id: camera
                    position: Qt.vector3d(0, 2, 8)
                    eulerRotation.x: -15
                    fieldOfView: scene.camera().fieldOfView
                    clipNear: scene.camera().nearPlane
                    clipFar: scene.camera().farPlane
                }

                // Grid helper
                Model {
                    visible: scene.showGrid
                    source: "#Rectangle"
                    scale: Qt.vector3d(20, 20, 1)
                    eulerRotation.x: -90
                    materials: [
                        DefaultMaterial {
                            diffuseColor: "#333333"
                            opacity: 0.5
                        }
                    ]
                }

                // Axes helper
                Node {
                    visible: scene.showAxes

                    // X axis (red)
                    Model {
                        source: "#Cylinder"
                        scale: Qt.vector3d(0.02, 2, 0.02)
                        position: Qt.vector3d(1, 0, 0)
                        eulerRotation.z: -90
                        materials: DefaultMaterial { diffuseColor: "#ff0000" }
                    }

                    // Y axis (green)
                    Model {
                        source: "#Cylinder"
                        scale: Qt.vector3d(0.02, 2, 0.02)
                        position: Qt.vector3d(0, 1, 0)
                        materials: DefaultMaterial { diffuseColor: "#00ff00" }
                    }

                    // Z axis (blue)
                    Model {
                        source: "#Cylinder"
                        scale: Qt.vector3d(0.02, 2, 0.02)
                        position: Qt.vector3d(0, 0, 1)
                        eulerRotation.x: 90
                        materials: DefaultMaterial { diffuseColor: "#0000ff" }
                    }
                }

                // Dynamic scene objects
                Repeater3D {
                    model: scene.objectCount

                    delegate: Loader3D {
                        id: objectLoader
                        property var sceneObj: scene.objectAt(index)

                        sourceComponent: {
                            if (!sceneObj) return null
                            if (sceneObj instanceof Primitive3D) return primitiveComponent
                            if (sceneObj instanceof Light3D) return lightComponent
                            return null
                        }
                    }
                }

                // Primitive component
                Component {
                    id: primitiveComponent

                    Model {
                        property var obj: objectLoader.sceneObj

                        visible: obj ? obj.visible : true
                        position: obj ? Qt.vector3d(obj.positionX, obj.positionY, obj.positionZ) : Qt.vector3d(0,0,0)
                        eulerRotation: obj ? Qt.vector3d(obj.rotationX, obj.rotationY, obj.rotationZ) : Qt.vector3d(0,0,0)
                        scale: obj ? Qt.vector3d(obj.scaleX, obj.scaleY, obj.scaleZ) : Qt.vector3d(1,1,1)

                        source: {
                            if (!obj) return "#Cube"
                            switch (obj.primitiveType) {
                                case Primitive3D.Cube: return "#Cube"
                                case Primitive3D.Sphere: return "#Sphere"
                                case Primitive3D.Cylinder: return "#Cylinder"
                                case Primitive3D.Cone: return "#Cone"
                                default: return "#Cube"
                            }
                        }

                        materials: [
                            PrincipledMaterial {
                                baseColor: obj ? Qt.rgba(obj.colorR, obj.colorG, obj.colorB, 1) : "#808080"
                                metalness: obj ? obj.metalness : 0
                                roughness: obj ? obj.roughness : 0.5
                            }
                        ]

                        // Selection highlight
                        Rectangle {
                            visible: obj === selectedObject
                            anchors.centerIn: parent
                            width: 10
                            height: 10
                            color: "yellow"
                            opacity: 0.5
                        }
                    }
                }

                // Light component
                Component {
                    id: lightComponent

                    Node {
                        property var obj: objectLoader.sceneObj

                        visible: obj ? obj.visible : true
                        position: obj ? Qt.vector3d(obj.positionX, obj.positionY, obj.positionZ) : Qt.vector3d(0,0,0)
                        eulerRotation: obj ? Qt.vector3d(obj.rotationX, obj.rotationY, obj.rotationZ) : Qt.vector3d(0,0,0)

                        PointLight {
                            visible: obj && obj.lightType === Light3D.Point
                            color: obj ? Qt.rgba(obj.colorR, obj.colorG, obj.colorB, 1) : "#ffffff"
                            brightness: obj ? obj.intensity : 1
                            castsShadow: obj ? obj.castShadows : true
                        }

                        SpotLight {
                            visible: obj && obj.lightType === Light3D.Spot
                            color: obj ? Qt.rgba(obj.colorR, obj.colorG, obj.colorB, 1) : "#ffffff"
                            brightness: obj ? obj.intensity : 1
                            coneAngle: obj ? obj.spotAngle : 45
                            castsShadow: obj ? obj.castShadows : true
                        }

                        DirectionalLight {
                            visible: obj && obj.lightType === Light3D.Directional
                            color: obj ? Qt.rgba(obj.colorR, obj.colorG, obj.colorB, 1) : "#ffffff"
                            brightness: obj ? obj.intensity : 1
                            castsShadow: obj ? obj.castShadows : true
                        }

                        // Light gizmo
                        Model {
                            source: "#Sphere"
                            scale: Qt.vector3d(0.1, 0.1, 0.1)
                            materials: DefaultMaterial {
                                diffuseColor: obj ? Qt.rgba(obj.colorR, obj.colorG, obj.colorB, 1) : "#ffff00"
                                lighting: DefaultMaterial.NoLighting
                            }
                        }
                    }
                }
            }

            // Camera controls
            WasdController {
                controlledObject: camera
                speed: 0.5
                shiftSpeed: 2
            }

            // Mouse zoom and orbit controls
            MouseArea {
                id: viewMouseArea
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                hoverEnabled: true

                property real lastX: 0
                property real lastY: 0
                property bool isDragging: false

                // Zoom with mouse wheel
                onWheel: function(wheel) {
                    var zoomFactor = 1.0 - wheel.angleDelta.y / 1200.0
                    var forward = camera.forward
                    var distance = camera.position.length() * (zoomFactor - 1.0)
                    
                    camera.position = Qt.vector3d(
                        camera.position.x + forward.x * distance * 2,
                        camera.position.y + forward.y * distance * 2,
                        camera.position.z + forward.z * distance * 2
                    )
                }

                onPressed: function(mouse) {
                    lastX = mouse.x
                    lastY = mouse.y
                    isDragging = true
                }

                onReleased: function(mouse) {
                    isDragging = false
                }

                onPositionChanged: function(mouse) {
                    if (!isDragging) return

                    var deltaX = mouse.x - lastX
                    var deltaY = mouse.y - lastY
                    lastX = mouse.x
                    lastY = mouse.y

                    // Right button or Middle button: Orbit camera
                    if (mouse.buttons & Qt.RightButton || mouse.buttons & Qt.MiddleButton) {
                        camera.eulerRotation.y += deltaX * 0.3
                        camera.eulerRotation.x = Math.max(-89, Math.min(89, camera.eulerRotation.x - deltaY * 0.3))
                    }
                    // Left button: Pan camera
                    else if (mouse.buttons & Qt.LeftButton) {
                        var right = camera.right
                        var up = camera.up
                        var panSpeed = 0.01

                        camera.position = Qt.vector3d(
                            camera.position.x - right.x * deltaX * panSpeed + up.x * deltaY * panSpeed,
                            camera.position.y - right.y * deltaX * panSpeed + up.y * deltaY * panSpeed,
                            camera.position.z - right.z * deltaX * panSpeed + up.z * deltaY * panSpeed
                        )
                    }
                }
            }

            // View controls overlay
            RowLayout {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 10
                spacing: 5

                Button {
                    text: "Grid"
                    checkable: true
                    checked: scene.showGrid
                    onCheckedChanged: scene.showGrid = checked
                }

                Button {
                    text: "Axes"
                    checkable: true
                    checked: scene.showAxes
                    onCheckedChanged: scene.showAxes = checked
                }

                Button {
                    text: "Reset Camera"
                    onClicked: {
                        camera.position = Qt.vector3d(0, 2, 8)
                        camera.eulerRotation = Qt.vector3d(-15, 0, 0)
                    }
                }
            }

            // FPS counter
            Label {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.margins: 10
                text: "Objects: " + scene.objectCount
                color: "#888888"
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
                        height: 40
                        color: "#252525"

                        Label {
                            anchors.centerIn: parent
                            text: selectedObject ? selectedObject.name : "No Selection"
                            font.bold: true
                            font.pixelSize: 14
                            color: "#ffffff"
                        }
                    }

                    // Properties (shown when object selected)
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.margins: 10
                        spacing: 10
                        visible: selectedObject !== null

                        // Transform section
                        PropertySection {
                            title: "Transform"
                            Layout.fillWidth: true

                            GridLayout {
                                columns: 4
                                columnSpacing: 5
                                rowSpacing: 5
                                Layout.fillWidth: true

                                // Position
                                Label { text: "Position"; color: "#aaa"; Layout.columnSpan: 4 }
                                Label { text: "X"; color: "#ff6666" }
                                SpinBox {
                                    from: -10000; to: 10000
                                    value: selectedObject ? selectedObject.positionX * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.positionX = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Y"; color: "#66ff66" }
                                SpinBox {
                                    from: -10000; to: 10000
                                    value: selectedObject ? selectedObject.positionY * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.positionY = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Z"; color: "#6666ff" }
                                SpinBox {
                                    from: -10000; to: 10000
                                    value: selectedObject ? selectedObject.positionZ * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.positionZ = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Item { Layout.columnSpan: 1 }

                                // Rotation
                                Label { text: "Rotation"; color: "#aaa"; Layout.columnSpan: 4 }
                                Label { text: "X"; color: "#ff6666" }
                                SpinBox {
                                    from: -36000; to: 36000
                                    value: selectedObject ? selectedObject.rotationX * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.rotationX = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Y"; color: "#66ff66" }
                                SpinBox {
                                    from: -36000; to: 36000
                                    value: selectedObject ? selectedObject.rotationY * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.rotationY = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Z"; color: "#6666ff" }
                                SpinBox {
                                    from: -36000; to: 36000
                                    value: selectedObject ? selectedObject.rotationZ * 100 : 0
                                    onValueModified: if (selectedObject) selectedObject.rotationZ = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Item { Layout.columnSpan: 1 }

                                // Scale
                                Label { text: "Scale"; color: "#aaa"; Layout.columnSpan: 4 }
                                Label { text: "X"; color: "#ff6666" }
                                SpinBox {
                                    from: 1; to: 10000
                                    value: selectedObject ? selectedObject.scaleX * 100 : 100
                                    onValueModified: if (selectedObject) selectedObject.scaleX = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Y"; color: "#66ff66" }
                                SpinBox {
                                    from: 1; to: 10000
                                    value: selectedObject ? selectedObject.scaleY * 100 : 100
                                    onValueModified: if (selectedObject) selectedObject.scaleY = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Label { text: "Z"; color: "#6666ff" }
                                SpinBox {
                                    from: 1; to: 10000
                                    value: selectedObject ? selectedObject.scaleZ * 100 : 100
                                    onValueModified: if (selectedObject) selectedObject.scaleZ = value / 100
                                    editable: true
                                    Layout.fillWidth: true
                                }
                                Item { Layout.columnSpan: 1 }
                            }

                            Button {
                                text: "Reset Transform"
                                Layout.fillWidth: true
                                onClicked: if (selectedObject) selectedObject.resetTransform()
                            }
                        }

                        // Material section
                        PropertySection {
                            title: "Material"
                            Layout.fillWidth: true

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // Color
                                Label { text: "Color"; color: "#aaa" }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 5

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        color: selectedObject ?
                                            Qt.rgba(selectedObject.colorR, selectedObject.colorG, selectedObject.colorB, 1) :
                                            "#808080"
                                        border.color: "#555"
                                        border.width: 1

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: colorDialog.open()
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Slider {
                                            Layout.fillWidth: true
                                            from: 0; to: 1
                                            value: selectedObject ? selectedObject.colorR : 0.5
                                            onMoved: if (selectedObject) selectedObject.colorR = value

                                            background: Rectangle {
                                                color: "#ff0000"
                                                opacity: 0.3
                                                radius: 2
                                            }
                                        }
                                        Slider {
                                            Layout.fillWidth: true
                                            from: 0; to: 1
                                            value: selectedObject ? selectedObject.colorG : 0.5
                                            onMoved: if (selectedObject) selectedObject.colorG = value

                                            background: Rectangle {
                                                color: "#00ff00"
                                                opacity: 0.3
                                                radius: 2
                                            }
                                        }
                                        Slider {
                                            Layout.fillWidth: true
                                            from: 0; to: 1
                                            value: selectedObject ? selectedObject.colorB : 0.5
                                            onMoved: if (selectedObject) selectedObject.colorB = value

                                            background: Rectangle {
                                                color: "#0000ff"
                                                opacity: 0.3
                                                radius: 2
                                            }
                                        }
                                    }
                                }

                                // Metalness
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Metalness"; color: "#aaa"; Layout.preferredWidth: 80 }
                                    Slider {
                                        Layout.fillWidth: true
                                        from: 0; to: 1
                                        value: selectedObject ? selectedObject.metalness : 0
                                        onMoved: if (selectedObject) selectedObject.metalness = value
                                    }
                                    Label {
                                        text: selectedObject ? selectedObject.metalness.toFixed(2) : "0.00"
                                        color: "#888"
                                        Layout.preferredWidth: 40
                                    }
                                }

                                // Roughness
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Roughness"; color: "#aaa"; Layout.preferredWidth: 80 }
                                    Slider {
                                        Layout.fillWidth: true
                                        from: 0; to: 1
                                        value: selectedObject ? selectedObject.roughness : 0.5
                                        onMoved: if (selectedObject) selectedObject.roughness = value
                                    }
                                    Label {
                                        text: selectedObject ? selectedObject.roughness.toFixed(2) : "0.50"
                                        color: "#888"
                                        Layout.preferredWidth: 40
                                    }
                                }

                                // Material presets
                                Label { text: "Presets"; color: "#aaa" }
                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 5

                                    Button {
                                        text: "Metal"
                                        onClicked: {
                                            if (selectedObject) {
                                                selectedObject.metalness = 1.0
                                                selectedObject.roughness = 0.3
                                            }
                                        }
                                    }
                                    Button {
                                        text: "Plastic"
                                        onClicked: {
                                            if (selectedObject) {
                                                selectedObject.metalness = 0.0
                                                selectedObject.roughness = 0.5
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

                        // Light-specific properties (when light selected)
                        PropertySection {
                            title: "Light"
                            Layout.fillWidth: true
                            visible: selectedObject instanceof Light3D

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // Intensity
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Intensity"; color: "#aaa"; Layout.preferredWidth: 80 }
                                    Slider {
                                        Layout.fillWidth: true
                                        from: 0; to: 5
                                        value: selectedObject && selectedObject.intensity !== undefined ?
                                               selectedObject.intensity : 1
                                        onMoved: if (selectedObject) selectedObject.intensity = value
                                    }
                                }

                                // Cast shadows
                                CheckBox {
                                    text: "Cast Shadows"
                                    checked: selectedObject && selectedObject.castShadows !== undefined ?
                                             selectedObject.castShadows : true
                                    onCheckedChanged: if (selectedObject) selectedObject.castShadows = checked
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }

                    // No selection message
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: selectedObject === null

                        Label {
                            anchors.centerIn: parent
                            text: "Select an object to edit its properties"
                            color: "#666666"
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }

    // Color picker dialog
    ColorDialog {
        id: colorDialog
        title: "Select Color"
        onAccepted: {
            if (selectedObject) {
                selectedObject.colorR = color.r
                selectedObject.colorG = color.g
                selectedObject.colorB = color.b
            }
        }
    }

    // Helper functions
    function addPrimitive(type) {
        var prim = scene.addPrimitive(type)
        selectedObject = prim
        selectedIndex = scene.objectCount - 1
    }

    function addLight(type) {
        var light = scene.addLight(type)
        selectedObject = light
        selectedIndex = scene.objectCount - 1
    }
}
