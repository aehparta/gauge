import QtQuick 2.7
import QtQuick.Layouts 1.0
import SignalCanvas 1.0
import "Signal.js" as Signal
import "."

Item {
	SignalCanvas {
		property point offsetMouse;
		property point offsetPoint;

		id: canvas
		devices: modelDevices
		anchors.fill: parent

		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			onPressed: Signal.onPressed(mouse)
			onReleased: Signal.onReleased(mouse)
			onPositionChanged: Signal.onPositionChanged(mouse)
			onWheel: Signal.onWheel(wheel)
			hoverEnabled: true
		}

		/* keep after canvas mousearea so this can catch mouse events too,
		 * also model is not set here: it causes crashing
		 */
		Flow {
			width: canvas.width
			anchors.left: canvas.left
			anchors.bottom: canvas.bottom
			anchors.bottomMargin: 5
			anchors.leftMargin: 5
			spacing: 10
			Repeater {
				id: channelsInfo
				delegate: SignalsChannelInfo {
					visible: modelData.visible
				}
			}
		}

		Component.onCompleted: {
			Signal.onInit();
			/* if set in repeater, causes crash */
			channelsInfo.model = canvas.channels;
			signalsChannelControls.model = canvas.channels;
		}
	}

	ListView {
		id: signalsChannelControls
		anchors.left: canvas.left
		anchors.top: canvas.top
		anchors.bottom: canvas.bottom
		anchors.leftMargin: 5
		anchors.topMargin: 5
		spacing: 5
		delegate: SignalsChannelControl {
		}
	}

	Text {
		id: signalsIntervalInfo
		anchors.right: canvas.right
		anchors.top: canvas.top
		anchors.rightMargin: 5
		anchors.topMargin: 5
		color: Style.signalsViewTextColor
		text: Signal.autoValueWithUnit(canvas.intervalX * 2 / Style.signalGridTilesX, 'S') + ' / div'
	}

    Text {
		visible: !canvas.hasValidOpenGl
        anchors.centerIn: canvas
        text: qsTr('You graphics card does not support OpenGL 3.0,\nwhich is required!')
        horizontalAlignment: Text.AlignHCenter
        color: 'white'
        font.pointSize: 24
        style: Text.Outline
        styleColor: 'red'
    }
}
