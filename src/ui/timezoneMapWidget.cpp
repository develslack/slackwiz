#include "timezoneMapWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QTimeZone>
#include <QDateTime>
#include <QFileInfo>
#include <QtMath>
#include <limits>

TimezoneMapWidget::TimezoneMapWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(290);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);

    // Soporte opcional: si existe una imagen equirectangular externa, la utiliza como fondo
    const QStringList mapAssets = {
        "/usr/share/slackwiz/world_map.png",
        ":/assets/world_map.png"
    };
    for (const QString &path : mapAssets) {
        if (QFileInfo::exists(path)) {
            m_customMapPixmap.load(path);
            break;
        }
    }

    buildContinentPolygons();
}

void TimezoneMapWidget::buildContinentPolygons()
{
    // Siluetas continentales en coordenadas (longitud, latitud) para proyección equirectangular
    auto makePoly = [](std::initializer_list<QPointF> pts) {
        QPolygonF poly;
        for (const QPointF &p : pts) {
            poly << p;
        }
        return poly;
    };

    // América del Norte
    m_continentsGeo << makePoly({
        {-168, 66}, {-162, 71}, {-140, 70}, {-125, 70}, {-110, 74}, {-85, 73},
        {-78, 63},  {-64, 60},  {-55, 52},  {-66, 44},  {-75, 36},  {-81, 25},
        {-84, 30},  {-90, 29},  {-97, 26},  {-97, 19},  {-87, 21},  {-84, 15},
        {-78, 8},   {-82, 9},   {-92, 15},  {-105, 20}, {-114, 31}, {-122, 37},
        {-124, 49}, {-137, 58}, {-152, 60}, {-165, 60}
    });

    // Groenlandia
    m_continentsGeo << makePoly({
        {-52, 60}, {-43, 60}, {-22, 70}, {-18, 81}, {-45, 83}, {-68, 76}, {-56, 68}
    });

    // América del Sur
    m_continentsGeo << makePoly({
        {-78, 9},  {-71, 12}, {-61, 10}, {-50, 1},  {-35, -6},  {-38, -14},
        {-41, -22},{-50, -30},{-57, -37},{-63, -41},{-67, -54}, {-74, -53},
        {-73, -42},{-71, -30},{-71, -18},{-81, -5}, {-79, 2}
    });

    // Europa
    m_continentsGeo << makePoly({
        {-9, 36},  {-9, 43},  {-4, 48},  {2, 51},   {8, 54},   {10, 58},
        {5, 61},   {15, 68},  {28, 71},  {42, 67},  {58, 68},  {60, 55},
        {48, 46},  {40, 44},  {29, 41},  {24, 37},  {16, 38},  {12, 44},
        {5, 43},   {-2, 36}
    });

    // Reino Unido e Irlanda
    m_continentsGeo << makePoly({
        {-6, 50}, {1, 51}, {0, 58}, {-6, 58}, {-9, 52}
    });

    // África
    m_continentsGeo << makePoly({
        {-17, 15}, {-13, 28}, {-6, 35},  {10, 37},  {25, 32},  {33, 31},
        {43, 12},  {51, 12},  {41, -2},  {40, -15}, {35, -25}, {27, -33},
        {18, -34}, {15, -26}, {12, -16}, {12, -5},  {9, 4},    {2, 6},
        {-8, 5},   {-16, 11}
    });

    // Madagascar
    m_continentsGeo << makePoly({
        {44, -25}, {47, -25}, {50, -13}, {47, -15}
    });

    // Asia
    m_continentsGeo << makePoly({
        {28, 41},  {40, 44},  {48, 46},  {60, 55},  {58, 68},  {75, 72},
        {105, 77}, {135, 71}, {170, 66}, {162, 56}, {142, 53}, {135, 43},
        {122, 40}, {121, 31}, {114, 22}, {108, 16}, {103, 2},  {98, 16},
        {91, 22},  {85, 19},  {78, 8},   {72, 20},  {66, 25},  {56, 26},
        {59, 22},  {53, 16},  {44, 13},  {35, 29},  {36, 36}
    });

    // Japón
    m_continentsGeo << makePoly({
        {130, 31}, {136, 34}, {141, 41}, {145, 44}, {140, 43}, {137, 37}, {130, 33}
    });

    // Sudeste Asiático Insular (Sumatra / Java / Borneo / Nueva Guinea)
    m_continentsGeo << makePoly({
        {96, 5}, {105, -6}, {115, -8}, {118, -8}, {116, -2}, {104, 1}
    });
    m_continentsGeo << makePoly({
        {109, 1}, {117, 6}, {118, -3}, {111, -3}
    });
    m_continentsGeo << makePoly({
        {132, -1}, {141, -3}, {150, -10}, {141, -8}, {133, -4}
    });

    // Australia
    m_continentsGeo << makePoly({
        {114, -22}, {122, -17}, {131, -12}, {136, -13}, {142, -11}, {146, -19},
        {153, -28}, {150, -37}, {140, -38}, {131, -32}, {115, -34}, {113, -26}
    });

    // Nueva Zelanda
    m_continentsGeo << makePoly({
        {167, -46}, {174, -41}, {178, -37}, {173, -35}, {172, -41}, {169, -46}
    });
}

void TimezoneMapWidget::setTimezones(const QList<TimezoneEntry> &zones)
{
    m_zones = zones;
    setSelectedTimezone(m_selectedId);
}

void TimezoneMapWidget::setSelectedTimezone(const QString &timezoneId)
{
    m_selectedId = timezoneId;
    m_selectedIndex = -1;
    for (int i = 0; i < m_zones.size(); ++i) {
        if (m_zones[i].id == timezoneId) {
            m_selectedIndex = i;
            break;
        }
    }
    update();
}

QPointF TimezoneMapWidget::geoToPixel(double lat, double lon, const QRectF &mapRect) const
{
    const double xRatio = (lon + 180.0) / 360.0;
    const double yRatio = (90.0 - lat) / 180.0;
    return QPointF(
        mapRect.left() + xRatio * mapRect.width(),
        mapRect.top() + yRatio * mapRect.height()
    );
}

void TimezoneMapWidget::pixelToGeo(const QPointF &pt, const QRectF &mapRect, double &latOut, double &lonOut) const
{
    const double clampedX = qBound(mapRect.left(), pt.x(), mapRect.right());
    const double clampedY = qBound(mapRect.top(), pt.y(), mapRect.bottom());

    const double xRatio = (clampedX - mapRect.left()) / mapRect.width();
    const double yRatio = (clampedY - mapRect.top()) / mapRect.height();

    lonOut = (xRatio * 360.0) - 180.0;
    latOut = 90.0 - (yRatio * 180.0);
}

int TimezoneMapWidget::findNearestZoneIndex(double lat, double lon) const
{
    if (m_zones.isEmpty()) {
        return -1;
    }

    int bestIdx = 0;
    double bestDistSq = std::numeric_limits<double>::max();

    for (int i = 0; i < m_zones.size(); ++i) {
        const double dLat = m_zones[i].latitude - lat;
        double dLon = m_zones[i].longitude - lon;
        if (dLon > 180.0) dLon -= 360.0;
        if (dLon < -180.0) dLon += 360.0;

        // Ponderación coseno para proximidad visual en proyección equirectangular
        const double distSq = (dLat * dLat) + (dLon * dLon);
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void TimezoneMapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || m_zones.isEmpty()) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->pos();
#endif

    const QRectF mapRect = rect().adjusted(2, 2, -2, -2);
    double lat = 0.0, lon = 0.0;
    pixelToGeo(pos, mapRect, lat, lon);

    const int nearestIdx = findNearestZoneIndex(lat, lon);
    if (nearestIdx >= 0 && nearestIdx < m_zones.size()) {
        m_selectedIndex = nearestIdx;
        m_selectedId = m_zones[nearestIdx].id;
        update();
        emit timezoneSelected(m_selectedId);
    }
}

void TimezoneMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_zones.isEmpty()) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
    const QPoint globalPos = event->globalPosition().toPoint();
#else
    const QPointF pos = event->pos();
    const QPoint globalPos = event->globalPos();
#endif

    const QRectF mapRect = rect().adjusted(2, 2, -2, -2);
    double lat = 0.0, lon = 0.0;
    pixelToGeo(pos, mapRect, lat, lon);

    const int idx = findNearestZoneIndex(lat, lon);
    if (idx != m_hoveredIndex) {
        m_hoveredIndex = idx;
        update();
    }

    if (idx >= 0 && idx < m_zones.size()) {
        const TimezoneEntry &z = m_zones[idx];
        const QTimeZone qtz(z.id.toUtf8());
        const QString offStr = qtz.isValid()
                                   ? TimeManager::formatUtcOffset(qtz.offsetFromUtc(QDateTime::currentDateTimeUtc()))
                                   : QString();
        const QString tip = QString("%1 (%2)").arg(z.id, offStr);
        QToolTip::showText(globalPos, tip, this);
    }
}

void TimezoneMapWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hoveredIndex = -1;
    update();
}

void TimezoneMapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF mapRect = rect().adjusted(2, 2, -2, -2);

    // Recorte con bordes redondeados estilo Ubuntu / Mint
    QPainterPath clipPath;
    clipPath.addRoundedRect(mapRect, 8.0, 8.0);
    p.setClipPath(clipPath);

    // 1. Fondo oceánico
    QLinearGradient oceanGrad(mapRect.topLeft(), mapRect.bottomLeft());
    oceanGrad.setColorAt(0.0, QColor("#162638"));
    oceanGrad.setColorAt(0.5, QColor("#1b3148"));
    oceanGrad.setColorAt(1.0, QColor("#142233"));
    p.fillRect(mapRect, oceanGrad);

    // 2. Bandas verticales de las 24 zonas horarias UTC
    const double bandWidth = mapRect.width() / 24.0;
    int selectedBandIndex = -1;
    if (m_selectedIndex >= 0 && m_selectedIndex < m_zones.size()) {
        const QTimeZone qtz(m_zones[m_selectedIndex].id.toUtf8());
        if (qtz.isValid()) {
            const int offsetHours = qRound(qtz.offsetFromUtc(QDateTime::currentDateTimeUtc()) / 3600.0);
            selectedBandIndex = qBound(0, offsetHours + 12, 23);
        }
    }

    for (int i = 0; i < 24; ++i) {
        const QRectF bandRect(mapRect.left() + i * bandWidth, mapRect.top(), bandWidth, mapRect.height());
        if (i == selectedBandIndex) {
            p.fillRect(bandRect, QColor(46, 160, 67, 55)); // Franja resaltada de la zona activa
        } else if (i % 2 == 0) {
            p.fillRect(bandRect, QColor(255, 255, 255, 6));
        }

        p.setPen(QPen(QColor(255, 255, 255, 18), 1.0));
        p.drawLine(QPointF(bandRect.left(), mapRect.top()), QPointF(bandRect.left(), mapRect.bottom()));
    }

    // 3. Línea del Ecuador y trópicos
    const double equatorY = mapRect.top() + mapRect.height() * 0.5;
    p.setPen(QPen(QColor(255, 255, 255, 32), 1.0, Qt::DashLine));
    p.drawLine(QPointF(mapRect.left(), equatorY), QPointF(mapRect.right(), equatorY));

    // 4. Continentes (imagen externa si existe o siluetas vectoriales)
    if (!m_customMapPixmap.isNull()) {
        p.drawPixmap(mapRect.toRect(), m_customMapPixmap);
    } else {
        p.setPen(QPen(QColor("#52796f"), 1.1));
        p.setBrush(QColor("#354f52"));
        for (const QPolygonF &geoPoly : m_continentsGeo) {
            QPolygonF pxPoly;
            pxPoly.reserve(geoPoly.size());
            for (const QPointF &geoPt : geoPoly) {
                pxPoly << geoToPixel(geoPt.y(), geoPt.x(), mapRect);
            }
            p.drawPolygon(pxPoly);
        }
    }

    // 5. Puntos de ciudades disponibles en zone1970.tab
    p.setPen(Qt::NoPen);
    for (int i = 0; i < m_zones.size(); ++i) {
        if (i == m_selectedIndex) {
            continue;
        }
        const QPointF pt = geoToPixel(m_zones[i].latitude, m_zones[i].longitude, mapRect);
        if (i == m_hoveredIndex) {
            p.setBrush(QColor("#f9c74f"));
            p.drawEllipse(pt, 4.0, 4.0);
        } else {
            p.setBrush(QColor(255, 255, 255, 75));
            p.drawEllipse(pt, 1.6, 1.6);
        }
    }

    // 6. Marcador destacado (Pin) de la zona seleccionada
    if (m_selectedIndex >= 0 && m_selectedIndex < m_zones.size()) {
        const TimezoneEntry &sel = m_zones[m_selectedIndex];
        const QPointF pinPt = geoToPixel(sel.latitude, sel.longitude, mapRect);

        // Ejes de mira suaves
        p.setPen(QPen(QColor(255, 107, 107, 120), 1.0, Qt::DotLine));
        p.drawLine(QPointF(pinPt.x(), mapRect.top()), QPointF(pinPt.x(), mapRect.bottom()));
        p.drawLine(QPointF(mapRect.left(), pinPt.y()), QPointF(mapRect.right(), pinPt.y()));

        // Halo exterior
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(230, 57, 70, 70));
        p.drawEllipse(pinPt, 13.0, 13.0);

        // Círculo principal del pin
        p.setPen(QPen(Qt::white, 2.0));
        p.setBrush(QColor("#e63946"));
        p.drawEllipse(pinPt, 6.0, 6.0);

        // Etiqueta flotante con ciudad y desplazamiento UTC
        const QTimeZone qtz(sel.id.toUtf8());
        const QString utcLabel = qtz.isValid()
                                     ? TimeManager::formatUtcOffset(qtz.offsetFromUtc(QDateTime::currentDateTimeUtc()))
                                     : QString();
        const QString cityShort = sel.id.section('/', -1).replace('_', ' ');
        const QString badgeText = QString("%1 (%2)").arg(cityShort, utcLabel);

        QFont badgeFont = font();
        badgeFont.setPointSize(9);
        badgeFont.setBold(true);
        p.setFont(badgeFont);

        const QFontMetrics fm(badgeFont);
        const int textW = fm.horizontalAdvance(badgeText) + 14;
        const int textH = fm.height() + 6;

        double boxX = pinPt.x() + 12.0;
        if (boxX + textW > mapRect.right() - 6.0) {
            boxX = pinPt.x() - textW - 12.0;
        }
        double boxY = qBound(mapRect.top() + 6.0, pinPt.y() - (textH / 2.0), mapRect.bottom() - textH - 6.0);

        const QRectF badgeRect(boxX, boxY, textW, textH);
        p.setPen(QPen(QColor(255, 255, 255, 80), 1.0));
        p.setBrush(QColor(18, 28, 40, 220));
        p.drawRoundedRect(badgeRect, 5.0, 5.0);

        p.setPen(Qt::white);
        p.drawText(badgeRect, Qt::AlignCenter, badgeText);
    }

    // Borde exterior del widget
    p.setClipping(false);
    p.setPen(QPen(QColor("#495057"), 1.2));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(mapRect, 8.0, 8.0);
}
