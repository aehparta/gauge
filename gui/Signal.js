function nanosecondsToHuman(t)
{
	if (t == 0)
	{
		return '0';
	}
	var ta = Math.abs(t);
	if (ta < 1e3)
	{
		return t + ' ns';
	}
	else if (ta < 1e6)
	{
		return t / 1e3 + ' us';
	}
	else if (ta < 1e9)
	{
		return t / 1e6 + ' ms';
	}
	return t / 1e9 + ' s';
}

function intervalStepUp(interval)
{
	/* convert to nanoseconds per grid division */
	interval *= (1e9 * 2 / Style.signalGridTilesX);
	if (interval >= (1e10 * 2 / Style.signalGridTilesX))
	{
		/* return default */
		return interval /= (1e9 * 2 / Style.signalGridTilesX);
	}
	var multiplier = 1;
	for (; interval >= 100; interval /= 10)
	{
		multiplier *= 10;
	}
	if (interval <= 10)
	{
		interval = 20;
	}
	else if (interval <= 20)
	{
		interval = 50;
	}
	else
	{
		interval = 100;
	}
	/* convert back to opengl window scale */
	return (interval * multiplier) / (1e9 * 2 / Style.signalGridTilesX);
}

function intervalStepDown(interval)
{
	/* convert to nanoseconds per grid division */
	interval *= (1e9 * 2 / Style.signalGridTilesX);
	if (interval <= (10 * 2 / Style.signalGridTilesX))
	{
		/* return default */
		return interval /= (1e9 * 2 / Style.signalGridTilesX);
	}
	var multiplier = 1;
	for (; interval >= 100; interval /= 10)
	{
		multiplier *= 10;
	}
	if (interval <= 10)
	{
		interval = 5;
	}
	else if (interval <= 20)
	{
		interval = 10;
	}
	else
	{
		interval = 20;
	}
	/* convert back to opengl window scale */
	return (interval * multiplier) / (1e9 * 2 / Style.signalGridTilesX);
}

function digitalScaleY(dirUp, scale)
{
	var digitalScales = [ 1.0, 0.75, 0.5, 0.25, 0.1, 0.05 ];
	var index = -1;
	for (var i = 0; i < digitalScales.length; i++)
	{
		if (digitalScales[i] === scale)
		{
			index = i;
		}
	}
	if (index < 0)
	{
		/* invalid, default to 1.0 */
		return 1.0;
	}
	else if (dirUp && (index - 1) >= 0)
	{
		return digitalScales[index - 1];
	}
	else if (!dirUp && (index + 1) < digitalScales.length)
	{
		return digitalScales[index + 1];
	}
	/* min or max reached */
	return scale;
}

function analogScaleY(dirUp, scale)
{
	var analogScales =
	[
		2.0 / Style.signalGridTilesY * 1000, /* 0.001 units per grid */
		2.0 / Style.signalGridTilesY * 500, /* 0.002 units per grid */
		2.0 / Style.signalGridTilesY * 200, /* 0.005 units per grid */
		2.0 / Style.signalGridTilesY * 100, /* 0.01 units per grid */
		2.0 / Style.signalGridTilesY * 50, /* 0.02 units per grid */
		2.0 / Style.signalGridTilesY * 20, /* 0.05 units per grid */
		2.0 / Style.signalGridTilesY * 10, /* 0.1 units per grid */
		2.0 / Style.signalGridTilesY * 5, /* 0.2 units per grid */
		2.0 / Style.signalGridTilesY * 2, /* 0.5 units per grid */
		2.0 / Style.signalGridTilesY * 1, /* 1.0 units per grid */
		2.0 / Style.signalGridTilesY * 0.5, /* 2.0 units per grid */
		2.0 / Style.signalGridTilesY * 0.2, /* 5.0 units per grid */
		2.0 / Style.signalGridTilesY * 0.1, /* 10.0 units per grid */
		2.0 / Style.signalGridTilesY * 0.05, /* 20.0 units per grid */
		2.0 / Style.signalGridTilesY * 0.02, /* 50.0 units per grid */
		2.0 / Style.signalGridTilesY * 0.01, /* 100.0 units per grid */
	];
	var index = -1;
	for (var i = 0; i < analogScales.length; i++)
	{
		if (analogScales[i] === scale)
		{
			index = i;
		}
	}
	if (index < 0)
	{
		/* invalid, default to 1.0 unit per grid */
		return 2.0 / Style.signalGridTilesY * 1;
	}
	else if (dirUp && (index - 1) >= 0)
	{
		return analogScales[index - 1];
	}
	else if (!dirUp && (index + 1) < analogScales.length)
	{
		return analogScales[index + 1];
	}
	/* min or max reached */
	return scale;
}

function onInit()
{
}

function onTrigger()
{
}

function onPressed(mouse)
{
	if (mouse.button === Qt.LeftButton)
	{
		canvas.offsetMouse.x = mouse.x;
		canvas.offsetMouse.y = mouse.y;
		canvas.offsetPoint.x = canvas.offsetX;
		canvas.offsetPoint.y = mouse.y;
		canvas.cursor = Qt.point(mouse.x, mouse.y);
		canvas.requestPaint(true);
	}
}

function onReleased(mouse)
{
	var p = Qt.point(mouse.x, mouse.y);
	p.x -= canvas.cursor.x;
	p.y -= canvas.cursor.y;
	if (mouse.button === Qt.LeftButton && Math.sqrt(p.x * p.x + p.y * p.y) < 10)
	{
		var redraw = false;
		var channels = canvas.channels;
		for (var j = 0; j < channels.length; j++)
		{
			var channel = channels[j];
			if (channel.selectable)
			{
				if (!channel.selected)
				{
					channel.selected = true;
					redraw = true;
				}
				else if (mouse.modifiers === Qt.ControlModifier)
				{
					channel.selected = false;
					redraw = true;
				}
			}
			else if (channel.selected && mouse.modifiers !== Qt.ControlModifier)
			{
				channel.selected = false;
				redraw = true;
			}
		}
		canvas.requestPaint();
	}
}

function onPositionChanged(mouse)
{
	if (mouse.buttons === Qt.LeftButton)
	{
		/* move vertically */
		var has_selection = false;
		if (mouse.modifiers !== Qt.ControlModifier)
		{
			var channels = canvas.channels;
			for (var j = 0; j < channels.length; j++)
			{
				var channel = channels[j];
				if (channel.selected)
				{
					channel.y = channel.y - (mouse.y - canvas.offsetPoint.y) / canvas.height * 2;
					has_selection = true;
				}
			}
			canvas.offsetPoint.y = mouse.y;
		}
		if (!has_selection)
		{
			/* move horizontally */
			canvas.offsetX = canvas.offsetPoint.x + (mouse.x - canvas.offsetMouse.x) / canvas.width * 2;
			canvas.requestPaint();
		}
		canvas.requestPaint();
	}
}

function onWheel(wheel)
{
	/* zoom vertically */
	var has_selection = false;
	if (wheel.modifiers !== Qt.ControlModifier)
	{
		var channels = canvas.channels;
		for (var j = 0; j < channels.length; j++)
		{
			var channel = channels[j];
			if (channel.selected)
			{
				if (channel.digital)
				{
					channel.scaleY = digitalScaleY(wheel.angleDelta.y > 0, channel.scaleY);
				}
				else
				{
					channel.scaleY = analogScaleY(wheel.angleDelta.y > 0, channel.scaleY);
				}
				has_selection = true;
				console.log(channel.scaleY);
			}
		}
	}
	if (!has_selection)
	{
		/* zoom horizontally */
		var intervalXOld = canvas.intervalX;
		canvas.intervalX = wheel.angleDelta.y > 0 ? intervalStepDown(canvas.intervalX) : intervalStepUp(canvas.intervalX);
		canvas.offsetX *= intervalXOld / canvas.intervalX;
	}

	canvas.requestPaint();
}

function autoValueWithUnit(value, unit)
{
	var unitPrefix = "";
	var multiplier = 1.0;
	if (value >= 1e9)
	{
		unitPrefix = "G";
		multiplier = 1e-9;
	}
	else if (value >= 1e6)
	{
		unitPrefix = "M";
		multiplier = 1e-6;
	}
	else if (value >= 1e3)
	{
		unitPrefix = "k";
		multiplier = 1e-3;
	}
	else if (value >= 1e0)
	{
		unitPrefix = "";
		multiplier = 1e0;
	}
	else if (value >= 1e-3)
	{
		unitPrefix = "m";
		multiplier = 1e3;
	}
	else if (value >= 1e-6)
	{
		unitPrefix = "μ";
		multiplier = 1e6;
	}
	else if (value >= 1e-9)
	{
		unitPrefix = "n";
		multiplier = 1e9;
	}
	else
	{
		unitPrefix = "p";
		multiplier = 1e12;
	}
	return (value * multiplier) + ' ' + unitPrefix + unit;
}
