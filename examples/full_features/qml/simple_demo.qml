import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    width: 100
    height: 250

    background: Rectangle {
        color: "transparent"
    }

    property SystemPalette systemPalette: SystemPalette {}

    Column {
        CheckBox {
            id: checkBox

            text: "CheckBox"

            contentItem: Text {
                text: checkBox.text
                font: checkBox.font

                verticalAlignment: Text.AlignVCenter
                leftPadding: checkBox.indicator.width + checkBox.spacing
                color: systemPalette.text
            }

            indicator: Rectangle {
                implicitHeight: 26
                implicitWidth: 26
                x: checkBox.leftPadding
                y: checkBox.height / 2 - height / 2
                color: "transparent"
                Image {
                    source: checkBox.checked ? "icon:/primary/checkbox_checked.svg" :
                                               "icon:/primary/checkbox_unchecked.svg"
                    cache: false
                }
            }
        }
        Repeater {
            model: 3

            RadioButton {
                id: radioButton

                text: "RadioButton"

                contentItem: Text {
                    text: radioButton.text
                    font: radioButton.font

                    verticalAlignment: Text.AlignVCenter
                    leftPadding: radioButton.indicator.width + radioButton.spacing
                    color: systemPalette.text
                }

                indicator: Rectangle {
                    implicitHeight: 26
                    implicitWidth: 26
                    x: radioButton.leftPadding
                    y: radioButton.height / 2 - height / 2
                    color: "transparent"
                    Image {
                        source: radioButton.checked ?
                                    "icon:/primary/radiobutton_checked.svg" :
                                    "icon:/primary/radiobutton_unchecked.svg"
                        cache: false
                    }
                }
            }
        }
    }
}
