import QtQuick 2.6
import QtQuick.Controls 2.1
import "."

Button {
	checkable: false
	checked: true
	font.family: 'Helvetica'
	font.pointSize: 12
	font.bold: true
	padding: 3
	flat: true
	ToolTip.visible: ToolTip.text.length > 0 ? hovered : false
	ToolTip.delay: Style.toolTipDelayDefault
}
