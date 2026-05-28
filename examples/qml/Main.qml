// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Generic inspector. Every field/method row is rendered from the
// metadata produced by rosetta::QmlVisitor — no per-Person markup.
//
// Field row layout:    label  [editor]  GET  PUT     (PUT may be read-only)
// Method row layout:   label  [args...]  Call   <result>
//
// Annotations surface here as:
//   readonly  -> editor disabled, PUT button replaced with "ro" badge
//   range     -> hint shown next to label, error toast on invalid PUT
//   doc       -> tooltip on label

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: root
    width: 760
    height: 640
    visible: true
    title: inspector.typeName + " — QML inspector"

    background: Rectangle { color: "#1a1a1a" }

    function logLine(msg, ok) {
        var ts = new Date().toISOString().substr(11, 8);
        var color = ok ? "#4cb050" : "#e57373";
        logArea.append('<span style="color:#555">' + ts + '</span> '
                       + '<span style="color:' + color + '">' + msg + '</span>');
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: inspector.typeName + " — generated from one C++26 reflection walk"
            color: "#888"
            font.pixelSize: 13
        }

        // -------- Fields --------
        Rectangle {
            Layout.fillWidth: true
            color: "#262626"
            border.color: "#3a3a3a"
            radius: 6
            implicitHeight: fieldsCol.implicitHeight + 24

            ColumnLayout {
                id: fieldsCol
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: "FIELDS"
                    color: "#888"
                    font.pixelSize: 11
                    font.letterSpacing: 1
                }

                Repeater {
                    model: inspector.fields
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        // doc tooltip on hover
                        Label {
                            text: modelData.name
                                  + (modelData.hasRange
                                     ? " [" + modelData.min + ".." + modelData.max + "]"
                                     : "")
                            color: "#bbb"
                            Layout.preferredWidth: 160
                            ToolTip.visible: hoverHandler.hovered && modelData.doc.length > 0
                            ToolTip.text: modelData.doc
                            HoverHandler { id: hoverHandler }
                        }

                        Loader {
                            id: editorLoader
                            Layout.fillWidth: true
                            sourceComponent: {
                                if (modelData.type === "int")    return intEditor;
                                if (modelData.type === "double") return doubleEditor;
                                if (modelData.type === "bool")   return boolEditor;
                                return stringEditor;
                            }
                            property var fieldInfo: modelData
                            property var currentValue: modelData.value

                            Component {
                                id: stringEditor
                                TextField {
                                    text: editorLoader.currentValue !== undefined
                                          ? editorLoader.currentValue : ""
                                    enabled: !editorLoader.fieldInfo.readonly
                                    onTextChanged: editorLoader.currentValue = text
                                }
                            }
                            Component {
                                id: intEditor
                                SpinBox {
                                    from: editorLoader.fieldInfo.hasRange
                                          ? editorLoader.fieldInfo.min : -2147483648
                                    to:   editorLoader.fieldInfo.hasRange
                                          ? editorLoader.fieldInfo.max :  2147483647
                                    value: editorLoader.currentValue || 0
                                    enabled: !editorLoader.fieldInfo.readonly
                                    onValueModified: editorLoader.currentValue = value
                                }
                            }
                            Component {
                                id: doubleEditor
                                TextField {
                                    text: String(editorLoader.currentValue || 0)
                                    enabled: !editorLoader.fieldInfo.readonly
                                    validator: DoubleValidator {}
                                    onTextChanged: editorLoader.currentValue = parseFloat(text)
                                }
                            }
                            Component {
                                id: boolEditor
                                CheckBox {
                                    checked: !!editorLoader.currentValue
                                    enabled: !editorLoader.fieldInfo.readonly
                                    onToggled: editorLoader.currentValue = checked
                                }
                            }
                        }

                        Button {
                            text: "GET"
                            onClicked: {
                                var v = inspector.getField(modelData.name);
                                editorLoader.currentValue = v;
                                root.logLine("GET " + modelData.name + " = "
                                             + JSON.stringify(v), true);
                            }
                        }

                        Button {
                            text: modelData.readonly ? "ro" : "PUT"
                            enabled: !modelData.readonly
                            onClicked: {
                                var err = inspector.setField(modelData.name,
                                                             editorLoader.currentValue);
                                if (err.length === 0)
                                    root.logLine("PUT " + modelData.name + " ← "
                                                 + JSON.stringify(editorLoader.currentValue),
                                                 true);
                                else
                                    root.logLine("PUT " + modelData.name + " failed: "
                                                 + err, false);
                            }
                        }
                    }
                }
            }
        }

        // -------- Methods --------
        Rectangle {
            Layout.fillWidth: true
            color: "#262626"
            border.color: "#3a3a3a"
            radius: 6
            implicitHeight: methodsCol.implicitHeight + 24

            ColumnLayout {
                id: methodsCol
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: "METHODS"
                    color: "#888"
                    font.pixelSize: 11
                    font.letterSpacing: 1
                }

                Repeater {
                    model: inspector.methods
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: modelData.name + "(" + modelData.arity + ")"
                                  + (modelData.isStatic ? " [static]" : "")
                            color: "#bbb"
                            Layout.preferredWidth: 160
                            ToolTip.visible: methHover.hovered && modelData.doc.length > 0
                            ToolTip.text: modelData.doc
                            HoverHandler { id: methHover }
                        }

                        // One TextField per parameter. argsRepeater.itemAt(i) gives us back
                        // the field so the Call handler can collect values.
                        Repeater {
                            id: argsRepeater
                            model: modelData.paramTypes
                            delegate: TextField {
                                Layout.preferredWidth: 110
                                placeholderText: modelData
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            text: "Call"
                            onClicked: {
                                var args = [];
                                for (var i = 0; i < argsRepeater.count; ++i) {
                                    var t = argsRepeater.itemAt(i);
                                    var v = t.text;
                                    if (modelData.paramTypes[i] === "int")
                                        v = parseInt(v || "0");
                                    else if (modelData.paramTypes[i] === "double")
                                        v = parseFloat(v || "0");
                                    else if (modelData.paramTypes[i] === "bool")
                                        v = (v === "true" || v === "1");
                                    args.push(v);
                                }
                                var result = inspector.callMethod(modelData.name, args);
                                root.logLine("CALL " + modelData.name + "("
                                             + args.join(", ") + ") → "
                                             + JSON.stringify(result), true);
                            }
                        }
                    }
                }
            }
        }

        // -------- Log --------
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#0e0e0e"
            border.color: "#3a3a3a"
            radius: 6

            ScrollView {
                anchors.fill: parent
                anchors.margins: 8
                TextArea {
                    id: logArea
                    readOnly: true
                    textFormat: TextEdit.RichText
                    color: "#e8e8e8"
                    font.family: "Menlo"
                    font.pixelSize: 12
                    background: null
                    wrapMode: TextEdit.Wrap
                }
            }
        }
    }
}
