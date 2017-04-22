import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import "."

SplitView {
	orientation: Qt.Vertical
	Layout.fillHeight: true

	SignalsView {
		Layout.minimumHeight: 50
	}
	//SignalsView {
	//	Layout.minimumHeight: 50
	//}
}
