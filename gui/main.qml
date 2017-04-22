import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
//import QtQuick.Extras 1.4
//import QtQuick.Controls 1.4
import Qt.labs.settings 1.0
import QtQuick 2.7
import QtQuick.Controls 2.1
import "Main.js" as MainJs
import "."

ApplicationWindow {
	id: window
	visible: true
	width: 640
	height: 480
	title: qsTr('Scope')

	/* this color is overridden in SignalCanvas.cpp,
	 * but it must be set here to other than white
	 * (some Qt bug?)
	 */
	color: '#000'

	RowLayout {
		anchors.fill: parent

		Column {
			id: mainControls
			Layout.alignment: Qt.AlignTop
			Layout.topMargin: 5
			Layout.leftMargin: 5
			spacing: 5

			BasicButtonLarge {
				id: buttonShowTriggers
				checkable: true
				text: '⥍'
				ToolTip.text: checked ? qsTr('Hide triggers and filters') : qsTr('Show triggers and filters')
			}
			BasicButtonLarge {
				id: buttonShowSignals
				text: '⎎'
				checkable: true
				onClicked: MainJs.onControlButton()
				ToolTip.text: checked ? qsTr('Hide signals') : qsTr('Show signals')
			}
			BasicButtonLarge {
				id: buttonShowTools
				text: '⚙'
				checkable: true
				checked: false
				onClicked: MainJs.onControlButton()
				ToolTip.text: checked ? qsTr('Hide tools') : qsTr('Show tools')
			}

			/* space */
			Item {
				width: parent.width
				height: 10
			}

			BasicButtonLarge {
				id: sampling
				text: 'S'
				checkable: true
				checked: modelDevices.sampling
				onCheckedChanged: {
					modelDevices.sampling = checked;
					console.log("sampling: " + modelDevices.sampling);
				}
				ToolTip.text: qsTr('Sampling')
			}
			BasicButtonLarge {
				id: forceTrigger
				text: 'T'
				onClicked: modelDevices.triggerAll()
				ToolTip.text: qsTr('Trigger now')
			}

			Settings {
				category: 'mainControls'
				property alias signals: buttonShowSignals.checked
				property alias tools: buttonShowTools.checked
				property alias sampling: buttonShowTriggers.checked
			}
		}

		ColumnLayout {
			Column {
				visible: buttonShowTriggers.checked
				GroupBox {
					title: qsTr('Sampling')
					Sampling {
					}
				}
				GroupBox {
					title: qsTr('Triggers')
					Triggers {
					}
				}
			}
			SignalsLayout {
				visible: buttonShowSignals.checked
				Layout.fillHeight: true
				Layout.fillWidth: true
			}
			/*
			SplitView {
				orientation: Qt.Vertical
				Layout.fillHeight: true
				Layout.fillWidth: true
			SignalsLayout {
				visible: buttonShowSignals.checked
			}
				ToolsLayout {
					visible: buttonShowTools.checked
				}
			}
			*/
		}
	}

	/*
	BasicButtonLarge {
		id: buttonFullscreen
		text: '⇱'
		ToolTip.text: qsTr('Toggle fullscreen')
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.topMargin: 5
		anchors.rightMargin: 5
		onClicked: {
			window.visibility = window.visibility === ApplicationWindow.FullScreen ? ApplicationWindow.Windowed : ApplicationWindow.FullScreen;
		}
	}
	*/

	Settings {
		id: windowSettings
		property alias x: window.x
		property alias y: window.y
		property alias width: window.width
		property alias height: window.height
		property int visibility: ApplicationWindow.Windowed
	}

	Component.onCompleted: {
		window.visibility = windowSettings.visibility;
	}

	onClosing: {
		windowSettings.visibility = window.visibility;
	}
}

