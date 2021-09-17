#ifndef SIMPLEROI_H
#define SIMPLEROI_H

/**
可交互ROI图形，用于通过鼠标选取图像处理范围、卡尺测量、获取位置等功能
**/

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPolygon>
#include <QDomDocument>

/**
 * @brief The SimpleROI class
 * 简单矩形ROI，交互功能只包含平移和缩放，使用简单
 */
class SimpleROI :public QGraphicsObject
{
    Q_OBJECT
public:
    SimpleROI();
    ~SimpleROI();

    QRect getRect() const;
    QRectF boundingRect() const override;
    void save(QDomDocument *document, QDomElement *parent);
    void load(const QDomNode &source);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
private:
    enum SimpleROIRegion {SIMPLEROI_OUTSIDE,
                         SIMPLEROI_INSIDE,
                         SIMPLEROI_TOP, SIMPLEROI_RIGHT, SIMPLEROI_BOTTOM, SIMPLEROI_LEFT,
                         SIMPLEROI_TOPLEFT, SIMPLEROI_TOPRIGHT,
                         SIMPLEROI_BOTTOMRIGHT, SIMPLEROI_BOTTOMLEFT};
    QRect m_rect;
    SimpleROIRegion m_curRegion;
    QPointF m_startPos;
    bool m_bMove;
    bool m_bScale;

    SimpleROIRegion judgePosition(const QPointF &pos);
    QRect getTranslatedRect(const QRect &before, int dx, int dy);
    QRect getScaledRect(const QRect &before, QPoint basePos, qreal sx, qreal sy);

    void scaleROI(const QPointF &mousePoint);
    void moveShape(const QPointF &mousePoint);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
signals:
    void ROITransformFinished();
};


/**
 * @brief The CaliperTool class
 * 旋转矩形ROI，交互功能包括平移、旋转、缩放、倾斜，同时可用于卡尺测量
 */
class CaliperTool :public QGraphicsObject
{
    Q_OBJECT
public:
    CaliperTool();
    ~CaliperTool();

    QRectF boundingRect() const override;
    void reInitialize();
    std::vector<QPointF> vertexes() const;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
private:
    enum CaliperRegion {CALIPER_NONE,
                        CALIPER_TOPLEFT, CALIPER_TOPRIGHT, CALIPER_BOTTOMRIGHT, CALIPER_BOTTOMLEFT,
                        CALIPER_RIGHTMIDDLE, CALIPER_BOTTOMMIDDLE,
                        CALIPER_TOP, CALIPER_RIGHT, CALIPER_BOTTOM, CALIPER_LEFT};
    CaliperRegion m_curRegion;
    QPolygonF m_mainShape;
    bool m_bMove;
    bool m_bScale;
    bool m_bRotate;
    bool m_bShear;
    QPointF m_startPos;
    qreal m_angle;
    qreal m_shearAngle;

    CaliperRegion judgePosition(const QPointF &pos);
    QPointF centre() const;
    void move(const QPointF &pos);
    void scale(const QPointF &pos);
    void rotate(const QPointF &pos);
    void shear(const QPointF &pos);
};


/**
 * @brief The SimpleMovablePoint class
 * 交互点图像，用于通过鼠标获取某位置
 */
class SimpleMovablePoint :public QGraphicsObject
{
    Q_OBJECT

public:
    SimpleMovablePoint();
    ~SimpleMovablePoint();

    QRectF boundingRect() const;
    QPoint positionOnScene() const;
    void moveTo(int x, int y);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
private:
    QPoint m_point;
    bool m_isMoving;

    void moveShape(const QPointF &pos);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
signals:
    void positionChanged();
};


#endif // SIMPLEROI_H
