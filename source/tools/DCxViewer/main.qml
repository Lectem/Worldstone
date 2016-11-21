import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import DCxViewer 1.0

ApplicationWindow {
    id: applicationWindow1
    visible: true
    width:640
    height:480
    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem {
                text: "&Open"
                shortcut: StandardKey.Open
                onTriggered: mpqFileDialog.open()
            }
            MenuItem {
                text: "Add list file"
                onTriggered: listfileFileDialog.open()
            }
            MenuItem {
                text: "E&xit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: "&Help"
            MenuItem {
                text: "About..."
                onTriggered: aboutDialog.open()
            }
        }
    }

    title: "DCxViewer"
    SplitView {
        id: splitView1
        anchors.fill: parent
        orientation: Qt.Horizontal
        ColumnLayout {
            id: column1
            width: 275
            Layout.minimumWidth: 100
            Text {
                id: text1
                text: "File number:" + app.mpqFiles.length
            }

            TableView {
                id: fileList
                Layout.minimumWidth: parent.Layout.minimumWidth
                Layout.fillWidth: true
                Layout.fillHeight: true

                TableViewColumn {
                    role: "filename"
                    title: "File"
                }
                model: app.mpqFiles
                onActivated: {
                    app.fileActivated(model[row])
                    dc6.Decode(model[row],app.mpqFileName)
                }
            }

        }

        Rectangle {
            QDC6{
                id: dc6
            }
            Text {
                text: "Version: " + dc6.header.version +"."+dc6.header.sub_version + "\n"
                + "Directions: " + dc6.header.directions + "\n"
                + "Frames per dir: " + dc6.header.frames_per_dir + "\n"
            }
            id: row2
            color: "lightgray"
        }
    }

    FileDialog {
        id: mpqFileDialog
        nameFilters: [ "MoPaQ files (*.mpq)", "All files (*)" ]
        onAccepted: { app.openMpq(fileUrl) }
    }

    FileDialog {
        id: listfileFileDialog
        nameFilters: [ "All files (*)" ]
        onAccepted: { app.addListFile(fileUrl) }
    }

}
