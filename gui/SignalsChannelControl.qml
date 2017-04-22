import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.1
import "."

Row {
	CheckBox {
		id: channelEnabled
		padding: 1
		checked: modelData.visible
		indicator: Rectangle {
			implicitWidth: 20
			implicitHeight: 20
			color: '#000'
			border.color: '#fff'
			border.width: 2
			Rectangle {
				width: 14
				height: 14
				x: 3
				y: 3
				color: modelData.color
				Rectangle {
					width: 10
					height: 10
					x: 2
					y: 2
					color: modelData.visible ? '#fff' : '#000'
					opacity: modelData.visible ? 0.0 : 0.7
				}
			}
		}
		onCheckedChanged: {
			modelData.visible = checked;
			if (!checked)
			{
				modelData.selected = false;
			}
		}
		onDoubleClicked: {
			checked: !checked
			selectChannelColorDialog.open();
		}
	}

	/*
	Rectangle {
		id: channelColor
		border.width: 3
		border.color: modelData.selected ? Style.channelControlBorderSelected : Style.channelControlBorder
		color: modelData.color
		width: channelEnabled.height
		height: channelEnabled.height

		MouseArea {
			anchors.fill: parent
			onClicked: {
				if (modelData.visible)
				{
					modelData.selected = !modelData.selected;
				}
			}
			onDoubleClicked: {
				mouse.accepted = true;
				if (modelData.visible)
				{
					modelData.selected = !modelData.selected;
				}
				selectChannelColorDialog.open();
			}
		}
	}
	*/

	ColorDialog {
		id: selectChannelColorDialog
		title: "Select new channel color"
		modality: Qt.ApplicationModal
		color: modelData.color
		onAccepted: {
			modelData.color = color;
		}
	}
}
