#include "paintdemo.h"
#include <QPainter>
#include <QPen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <QVector2D>
#include <functional>

PaintDemo::PaintDemo(QWidget *parent)
    : QWidget{parent}
{
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
}

void PaintDemo::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::red, 2));
    for (const QLine &line : lines)
        painter.drawLine(line);

    if (isDrawing)
    {
        painter.setPen(QPen(Qt::darkGreen, 1, Qt::DashLine));
        painter.drawLine(startPoint, currentPoint);

        if (snapping)
        {
            painter.setBrush(Qt::cyan);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(snapPoint, 5, 5);
        }

    }
}

void PaintDemo::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        int index = findLineNear(event->pos());
        if (index != -1)
        {
            lines.removeAt(index);
            update();
        }
        return;
    }

    QPoint snapped = findSnapPoint(event->pos());
    startPoint = snapped;
    currentPoint = snapped;
    isDrawing = true;
    update();
}

void PaintDemo::mouseMoveEvent(QMouseEvent *event)
{
    if (isDrawing)
    {
        QPoint cursorPos = findSnapPoint(event->pos());
        currentPoint = applyAngleSnap(startPoint, cursorPos);
        update();
    }

    QString coords = QString("Координаты: (%1, %2)").arg(event->pos().x()).arg(event->pos().y());
    this->window()->setWindowTitle(coords);
}

void PaintDemo::mouseReleaseEvent(QMouseEvent *event)
{
    if (isDrawing)
    {
        QPoint snapped = findSnapPoint(event->pos());
        currentPoint = applyAngleSnap(startPoint, snapped);
        lines.append(QLine(startPoint, currentPoint));
        isDrawing = false;
        snapping = false;
        update();
    }
}

void PaintDemo::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_C)
    {
        if (hasClosedLoop())
            this->window()->setWindowTitle("Контур замкнут");
        else
            this->window()->setWindowTitle("Контур НЕ замкнут");
    }
}

QPoint PaintDemo::findSnapPoint(const QPoint &pos)
{
    snapping = false;

    for (const QLine &line : lines)
    {
        if ((line.p1() - pos).manhattanLength() <= SNAP_RADIUS)
        {
            snapping = true;
            snapPoint = line.p1();
            return line.p1();
        }

        if ((line.p2() - pos).manhattanLength() <= SNAP_RADIUS)
        {
            snapping = true;
            snapPoint = line.p2();
            return line.p2();
        }
    }

    return pos;
}

QPoint PaintDemo::applyAngleSnap(const QPoint &start, const QPoint &rawEnd)
{
    QPointF delta = rawEnd - start;

    if (delta.manhattanLength() < 2)
        return rawEnd;

    double angle = std::atan2(delta.y(), delta.x()) * 180.0 / M_PI;
    if (angle < 0) angle += 360.0;

    QList<int> angles = {0, 45, 90, 135, 180, 225, 270, 315};
    int snapAngle = -1;

    for (int a : angles)
    {
        if (std::abs(angle - a) <= 5)
        {
            snapAngle = a;
            break;
        }
    }

    if (snapAngle == -1)
        return rawEnd;

    double length = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
    double radians = snapAngle * M_PI / 180.0;

    QPointF snapped(
        start.x() + length * std::cos(radians),
        start.y() + length * std::sin(radians)
        );

    return snapped.toPoint();
}

int PaintDemo::findLineNear(const QPoint &pos, int threshold)
{
    for (int i = 0; i < lines.size(); ++i)
    {
        QLine line = lines[i];
        QPointF p1 = line.p1();
        QPointF p2 = line.p2();
        QPointF p = pos;

        double lenSq = QVector2D(p2 - p1).lengthSquared();
        if (lenSq == 0.0) continue;

        double t = QPointF::dotProduct(p - p1, p2 - p1) / lenSq;
        t = qBound(0.0, t, 1.0);
        QPointF proj = p1 + t * (p2 - p1);

        if (QVector2D(p - proj).length() <= threshold)
            return i;
    }
    return -1;
}

bool PaintDemo::hasClosedLoop() const
{
    if (lines.size() < 3)
        return false;

    QHash<QPoint, QSet<QPoint>> adjacency;


    for (const QLine &line : lines)
    {
        adjacency[line.p1()].insert(line.p2());
        adjacency[line.p2()].insert(line.p1());
    }

    QSet<QPoint> visited;
    QHash<QPoint, QPoint> parent;


    std::function<bool(QPoint, QPoint)> dfs = [&](QPoint current, QPoint from) {
        visited.insert(current);
        for (const QPoint &neighbor : adjacency[current])
        {
            if (neighbor == from)
                continue;

            if (visited.contains(neighbor))
                return true;

            parent[neighbor] = current;
            if (dfs(neighbor, current))
                return true;
        }
        return false;
    };

    for (const QPoint &start : adjacency.keys())
    {
        if (!visited.contains(start))
        {
            if (dfs(start, QPoint()))
                return true;
        }
    }

    return false;
}
