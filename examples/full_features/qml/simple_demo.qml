import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    width: 100
    height: 250

    Column {
        CheckBox {
            id: checkBox

            indicator: Rectangle {
                implicitHeight: 26
                implicitWidth: 26
                x: checkBox.leftPadding
                y: checkBox.height / 2 - height / 2
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

                    indicator: Rectangle {
                        implicitHeight: 26
                        implicitWidth: 26
                        x: radioButton.leftPadding
                        y: radioButton.height / 2 - height / 2
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
