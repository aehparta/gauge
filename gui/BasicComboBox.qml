import QtQuick 2.7
import QtQuick.Controls 2.1
import "."

ComboBox {
	id: combobox
	flat: false
	font.pointSize: 12
	anchors.verticalCenter: parent.verticalCenter
	background: Rectangle {
		color: '#9c9'
	}
	indicator: null
	ToolTip.visible: ToolTip.text.length > 0 ? hovered : false
	ToolTip.delay: Style.toolTipDelayDefault
}
