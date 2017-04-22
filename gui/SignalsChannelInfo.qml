import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.1
import "Signal.js" as Signal
import "."

Rectangle {
	width: 80
	height: channelInfoContentLayout.height
	color: 'transparent'
	clip: true
	Row {
		id: channelInfoContentLayout
		Column {
			TextInput {
				id: channelLabelText
				text: modelData.label
				color: modelData.color
				font.bold: modelData.selected
				readOnly: true
				rightPadding: 1
				onEditingFinished: {
					readOnly = true;
					modelData.label = text.trim();
					text = modelData.label;
				}
				Keys.onEscapePressed: {
					readOnly = true;
					text = modelData.label;
				}
			}
			Text {
				id: channelInfoText
				text: {
					if (modelData.digital)
					{
						return "digital";
					}
					var unitsPerGrid = (1 / modelData.scaleY) / (Style.signalGridTilesY / 2);
					return Signal.autoValueWithUnit(unitsPerGrid, modelData.baseUnit) + ' / div';
				}
				color: modelData.color
				font.bold: modelData.selected
			}
		}
	}
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
			channelLabelText.readOnly = false;
			channelLabelText.focus = true;
			channelLabelText.selectAll();
		}
	}
}
