pragma Singleton
import QtQuick 2.5

QtObject {
	property string analogSignalColor: '#c08040'
	property string analogSignalBackgroundColor: '#002000'

	property color signalsViewTextColor: '#c0f0c0'

	property int signalGridTilesX: 8
	property int signalGridTilesY: 8

	property string analogSignalGridXZeroColor: '#40c040'
	property string analogSignalGridXColor: '#006000'
	property string analogSignalGridXHalfColor: '#003000'

	property string analogSignalGridYZeroColor: '#40c040'
	property string analogSignalGridYColor: '#006000'
	property string analogSignalGridYHalfColor: '#003000'

	property string analogSignalGridTextFont: '12px monospace'
	property color analogSignalGridTextColor: '#707030'

	property int digitalSignalGridX: 100

	property color channelControlBorder: 'transparent'
	property color channelControlBorderSelected: '#e0f0e0'
	property color channelControlBorderHidden: '#fff'

	property int toolTipDelayDefault: 700
}

