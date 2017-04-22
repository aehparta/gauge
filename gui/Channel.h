#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>
#include <QPointF>
#include <QVariant>
#include <QColor>
#include <devlib.h>

class Channel : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString index READ getIndex)
	Q_PROPERTY(QString label READ getLabel WRITE setLabel NOTIFY changedLabel)

	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY changedEnabled)
	Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY changedVisible)
	Q_PROPERTY(bool digital READ isDigital NOTIFY changedDigital)
	Q_PROPERTY(double y READ getY WRITE setY NOTIFY changedY)
	Q_PROPERTY(double scaleY READ getScaleY WRITE setScaleY NOTIFY changedScaleY)

	Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY changedSelected)
	Q_PROPERTY(bool selectable READ isSelectable NOTIFY changedSelectable)

	Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY changedColor)

	Q_PROPERTY(QString baseUnit READ getBaseUnit NOTIFY changedBaseUnit)

public:
	Channel(int index, bool digital, QObject *parent = 0);

	const QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant());
	void setSetting(const QString &key, const QVariant &value);
	void setSetting(const QString &key, const QString &value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const int value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const double value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const bool value) { setSetting(key, QVariant(value)); }

	int getIndex() const;

	QString getLabel() const;
	void setLabel(const QString &value);

	bool isEnabled() const;
	void setEnabled(const bool value);
	bool isVisible() const;
	void setVisible(const bool value);
	bool isDigital() const;
	double getY() const;
	void setY(const double value);
	double getScaleY() const;
	void setScaleY(const double value);

	bool isSelected();
	void setSelected(const bool value);
	void enableSelecting();
	void disableSelecting();
	bool isSelectable();

	QColor getColor() const;
	void setColor(const QColor value);

	QString getBaseUnit() const;

signals:
	void changedLabel();

	void changedEnabled();
	void changedVisible();
	void changedDigital();
	void changedY();
	void changedScaleY();

	void changedSelected();
	void changedSelectable();

	void changedColor();

	void changedBaseUnit();

	void changedSamples();

public slots:

private:
	int pIndex;
	QString pLabel;

	bool pEnabled;
	bool pVisible;
	bool pDigital;
	double pY;
	double pScaleY;
	bool pSelected;
	bool pSelectable;
	QColor pColor;

	QString pBaseUnit;

	int pSampleDataSize;
	uint8_t *pSampleData;
};

#endif // CHANNEL_H
