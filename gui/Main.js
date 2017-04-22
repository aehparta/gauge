
function onControlButton()
{
	if (layoutSignals.visible) {
		layoutTools.Layout.fillHeight = false;
		layoutSignals.Layout.fillHeight = true;
	} else {
		layoutSignals.Layout.fillHeight = false;
		layoutTools.Layout.fillHeight = true;
	}
}
