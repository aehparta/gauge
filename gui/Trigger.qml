import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.1
import "."

Row {
	spacing: 5

	SquareButton {
		id: triggerAdd
		text: '+'
		checkable: false
		visible: index == 0
		onClicked: {
			modelTrigger.append({ trigger: { label: 'x', enabled: false }});
		}
	}

	SquareButton {
		id: triggerMerging
		checkable: false
		visible: index != 0
		text: '&'
		onClicked: {
			switch (text)
			{
			case '&':
				text = '∥';
				break;
			case '∥':
				text = '⊻';
				break;
			case '⊻':
				text = '&';
				break;
			}
		}
	}

	CheckBox {
		checked: trigger.enabled
	}

	BasicComboBox {
		id: triggerDevice
		displayText: modelDevices.devices[currentIndex].label
		model: modelDevices.devices
		delegate: ItemDelegate {
			text: modelData.label
		}
	}
	BasicComboBox {
		id: triggerChannel
		displayText: modelDevices.devices[triggerDevice.currentIndex].channels[currentIndex].label
		model: modelDevices.devices[triggerDevice.currentIndex].channels
		visible: modelDevices.devices[triggerDevice.currentIndex].channels.length > 1 ? true : false
		delegate: ItemDelegate {
			text: modelData.label
		}
	}

	BasicComboBox {
		id: triggerType
		model: [ 'Edge', 'Pulse' ]
		indicator: null
		width: 70
		onCurrentIndexChanged: {
			switch (currentIndex) {
			case 0:
				triggerEdge.visible = true;
				layoutPulse.visible = false;
				break;
			case 1:
				triggerEdge.visible = false;
				layoutPulse.visible = true;
				break;
			}
		}
	}

	BasicComboBox {
		id: triggerEdge
		model: [ '↗', '↘' ]
		width: 40
		indicator: null
	}

	Row {
		id: layoutPulse
		visible: false
		spacing: 2
		height: parent.height
		BasicComboBox {
			id: triggerPulseType
			width: 70
			model: [ '⎤↔⎡', '>⎣⎦<', '⎦↔⎣', '>⎡⎤<' ]
			indicator: null
		}
		BasicComboBox {
			id: triggerPulseWidth
			width: 60
			model: [ '500', '250', '100', '50', '25', '10', '5', '1' ]
			indicator: null
		}
		BasicComboBox {
			id: triggerPulseWidthUnit
			width: 60
			model: [ 'S', 'mS', 'uS', 'nS' ]
			indicator: null
		}
	}

	SquareButton {
		id: triggerRemove
		text: '☒'
		visible: index != 0
		onClicked: {
			modelTrigger.remove(index);
		}
	}
}
