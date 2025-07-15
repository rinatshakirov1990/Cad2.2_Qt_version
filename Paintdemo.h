#ifndef PAINTDEMO_H
#define PAINTDEMO_H

#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QLine>
#include <QSet>
#include <QHash>



class PaintDemo : public QWidget
{
    Q_OBJECT

public:
    explicit PaintDemo(QWidget *parent = nullptr);



protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QPoint startPoint;
    QPoint currentPoint;
    QVector<QLine> lines;
    bool isDrawing = false;
    bool snapping = false;

    QPoint snapPoint;

    const int SNAP_RADIUS = 10;

    QPoint findSnapPoint(const QPoint &pos);
    QPoint applyAngleSnap(const QPoint &start, const QPoint &rawEnd);
    int findLineNear(const QPoint &pos, int threshold = 6);

    bool hasClosedLoop() const;
    QVector<QPoint> extractPolygon() const;
};

#endif // PAINTDEMO_H
