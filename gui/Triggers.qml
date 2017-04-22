import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import "."

RowLayout {
	anchors.left: parent.left
	anchors.right: parent.right

	Column {
		Layout.alignment: Qt.AlignTop | Qt.AlignLeft
		spacing: 3
		Repeater {
			id: triggers
			anchors.fill: parent
			model: modelTrigger
			delegate: Trigger {

			}
		}
		ListModel {
			id: modelTrigger
			Component.onCompleted: {
				modelTrigger.append({ trigger: { label: 'x', enabled: false }});
			}
		}
	}
}
