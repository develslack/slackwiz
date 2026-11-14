#ifndef TIMEZONEMAPWIDGET_H
#define TIMEZONEMAPWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QList>
#include <QPolygonF>
#include "timeManager.h"

class TimezoneMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimezoneMapWidget(QWidget *parent = nullptr);

    void setTimezones(const QList<TimezoneEntry> &zones);
    void setSelectedTimezone(const QString &timezoneId);
    QString selectedTimezone() const { return m_selectedId; }

signals:
    void timezoneSelected(const QString &timezoneId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPointF geoToPixel(double lat, double lon, const QRectF &mapRect) const;
    void pixelToGeo(const QPointF &pt, const QRectF &mapRect, double &latOut, double &lonOut) const;
    int findNearestZoneIndex(double lat, double lon) const;
    void buildContinentPolygons();

    QList<TimezoneEntry> m_zones;
    QList<QPolygonF> m_continentsGeo; // Coordenadas (lon, lat)
    QPixmap m_customMapPixmap;

    QString m_selectedId = "America/Argentina/Buenos_Aires";
    int m_selectedIndex = -1;
    int m_hoveredIndex = -1;
};

#endif // TIMEZONEMAPWIDGET_H
