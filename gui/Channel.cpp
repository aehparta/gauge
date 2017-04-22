#include <QDebug>
#include "Device.h"
#include "Channel.h"

Channel::Channel(int index, bool digital, QObject *parent) : QObject(parent)
{
	pIndex = index;
	pLabel = getSetting("pLabel", "").toString();
	pEnabled = true;
	pVisible = getSetting("pVisible", true).toBool();
	pDigital = digital;
	pSelected = false;
	pSelectable = false;
	if (digital)
	{
		pY = getSetting("pY", QVariant(0.9 - (double)index / 7)).toDouble();
		pScaleY = getSetting("pScaleY", 0.05).toDouble();
		pColor = QColor(getSetting("color", "#3f7fff").toString());
	}
	else
	{
		pY = getSetting("pY", -0.5).toDouble();
		pScaleY = getSetting("pScaleY", 0.2).toDouble();
		pColor = QColor(getSetting("color", "#cf0f0f").toString());
	}
	pBaseUnit = "V";
}

const QVariant Channel::getSetting(const QString &key, const QVariant &defaultValue)
{
	Device *device = (Device *)parent();
	/* crete full key */
	QString fullKey = "/channel/" + QString::number(pIndex) + "/" + key;
	return device->getSetting(fullKey, defaultValue);
}

void Channel::setSetting(const QString &key, const QVariant &value)
{
	Device *device = (Device *)parent();
	/* crete full key */
	QString fullKey = "/channel/" + QString::number(pIndex) + "/" + key;
	device->setSetting(fullKey, value);
}

int Channel::getIndex() const
{
	return pIndex;
}

QString Channel::getLabel() const
{
	if (pLabel.isEmpty())
	{
		Device *device = (Device *)parent();
		return QString::number(pIndex) + '@' + device->getSerial();
//		return "Channel #" + QString::number(pIndex);
	}
	return pLabel;
}

void Channel::setLabel(const QString &value)
{
	if (pLabel != value)
	{
		pLabel = value;
		setSetting("pLabel", pLabel);
		emit changedLabel();
	}
}

bool Channel::isEnabled() const
{
	return pEnabled;
}

void Channel::setEnabled(const bool value)
{
	if (pEnabled != value)
	{
		pEnabled = value;
		emit changedEnabled();
	}
}

bool Channel::isVisible() const
{
	return pVisible;
}

void Channel::setVisible(const bool value)
{
	if (pVisible != value)
	{
		pVisible = value;
		setSetting("pVisible", pVisible);
		emit changedVisible();
	}
}

bool Channel::isDigital() const
{
	return pDigital;
}

double Channel::getY() const
{
	return pY;
}

void Channel::setY(const double value)
{
	if (pY != value)
	{
		pY = value;
		setSetting("pY", pY);
		emit changedY();
	}
}

double Channel::getScaleY() const
{
	return pScaleY;
}

void Channel::setScaleY(const double value)
{
	if (pScaleY != value)
	{
		pScaleY = value;
		setSetting("pScaleY", pScaleY);
		emit changedScaleY();
	}
}

bool Channel::isSelected()
{
	return pSelected;
}

void Channel::setSelected(const bool value)
{
	if (pSelected != value)
	{
		pSelected = value;
		emit changedSelected();
	}
}

void Channel::enableSelecting()
{
	if (pSelectable == false)
	{
		pSelectable = true;
		emit changedSelectable();
	}
}

void Channel::disableSelecting()
{
	if (pSelectable == true)
	{
		pSelectable = false;
		emit changedSelectable();
	}
}

bool Channel::isSelectable()
{
	return pSelectable;
}

QColor Channel::getColor() const
{
	return pColor;
}

void Channel::setColor(const QColor value)
{
	if (pColor != value)
	{
		pColor = value;
		setSetting("color", pColor.name());
		emit changedColor();
	}
}

QString Channel::getBaseUnit() const
{
	return pBaseUnit;
}

