import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1
import "."

Row {
	anchors.left: parent.left
	anchors.right: parent.right

	TextField {
		id: samplingTime
		width: 100
		height: samplingTimeUnit.height
		font: samplingTimeUnit.font
		ToolTip.text: qsTr('Sampling time')
		ToolTip.visible: hovered
		ToolTip.delay: Style.toolTipDelayDefault
		horizontalAlignment: TextField.AlignRight
		text: modelDevices.duration * Math.pow(1000, samplingTimeUnit.currentIndex)
		onTextChanged: {
			var time = parseFloat(text);
			time /= Math.pow(1000, samplingTimeUnit.currentIndex);
			if (time === NaN || time < 1e-6 || time > 10)
			{
				samplingTime.color = '#f00';
			}
			else
			{
				samplingTime.color = '#000';
				modelDevices.duration = time;
			}
		}
	}
	BasicComboBox {
		id: samplingTimeUnit
		width: 60
		model: [ 'S', 'mS', 'uS' ]
		indicator: null
		currentIndex: 0
		onCurrentIndexChanged: {
			samplingTime.textChanged();
		}
	}
}
