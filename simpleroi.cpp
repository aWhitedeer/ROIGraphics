#include "simpleroi.h"

#include <cmath>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QDebug>

#define SHAPE_THICK 1 // 形状厚度
#define ROIRECT_SIZE 6 //ROI方形大小
#define SHAPE_CONTROL_THICK 1 //形状控制块厚度
#define SHAPE_CONTROL_SIZE 5 //形状控制块
#define SHAPE_RECT_MINLENGTH  18 //主矩形最小边长
#define ROI_TWO_POINT // ROI两点控制
#define DRAW_EIGHT_POINT   //ROI八点控制
#define SIMPLE_POINT_WIDTH 12    //点图形标识线长度

const double Pi =3.14159265;
const double g_minLen =20;
const double g_minAngle =5;

//auxiliary functions
qreal getDistance(const QPointF &pt1, const QPointF &pt2);
qreal getDistance(const QPointF &pt, const QPointF &A, const QPointF &B);
QPointF getRotatedPoint(const QPointF &pt, const QPointF &center, qreal angle);
QPolygonF getRotatedPolygon(const QPolygonF &poly, const QPointF &center, qreal angle);
QPolygonF getShearPolygon(const QPolygonF &poly, qreal angle);
QPolygonF getMovedPolygon(const QPolygonF &poly, qreal dx, qreal dy);
void addElementWithText(QDomDocument *document, QDomElement *parent, const QString &tagName, const QString &data);


//class SimpleROI

SimpleROI::SimpleROI()
{
    m_rect =QRect(0, 0, 100, 100);
    m_startPos =QPointF();
    setCursor(Qt::ArrowCursor);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

SimpleROI::~SimpleROI()
{

}


/**
 * @brief SimpleROI::getRect
 * 获取ROI矩形
 * @return
 */
QRect SimpleROI::getRect() const
{
    return m_rect;
}

QRectF SimpleROI::boundingRect() const
{
    QPointF origin(m_rect.x() -SHAPE_CONTROL_SIZE -1, m_rect.y() -SHAPE_CONTROL_SIZE -1);
    QSizeF size(m_rect.width() +2*SHAPE_CONTROL_SIZE +2, m_rect.height() +2*SHAPE_CONTROL_SIZE +2);
    return QRectF(origin, size);
}

/**
 * @brief SimpleROI::save
 * 保存ROI
 * @param document
 * @param parent
 */
void SimpleROI::save(QDomDocument *document, QDomElement *parent)
{
    addElementWithText(document, parent, "x", QString::number(m_rect.x()));
    addElementWithText(document, parent, "y", QString::number(m_rect.y()));
    addElementWithText(document, parent, "width", QString::number(m_rect.width()));
    addElementWithText(document, parent, "height", QString::number(m_rect.height()));
}

/**
 * @brief SimpleROI::load
 * 打开保存文件时加载ROI
 * @param source
 */
void SimpleROI::load(const QDomNode &source)
{
    QDomNodeList nodes =source.childNodes();
    if(nodes.size() !=4){
        qDebug()<<"SimpleROI::load: number of elements is incorrect, please check the code.";
        return;
    }
    QVector<int> list;
    for(int i =0; i <nodes.size(); ++i){
        list <<nodes.at(i).toElement().text().toInt();
    }
    prepareGeometryChange();
    m_rect =QRect(list.at(0), list.at(1), list.at(2), list.at(3));
    update();
}

/**
 * @brief CaliperTool::mousePressEvent
 * 根据鼠标按下位置，确定变换类型
 * @param event
 */
void SimpleROI::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() ==Qt::LeftButton){
        m_startPos =event->pos();
        m_curRegion =judgePosition(event->pos());
        if(m_curRegion ==SIMPLEROI_OUTSIDE)  return;
        if(m_curRegion ==SIMPLEROI_INSIDE){
            setCursor(Qt::ClosedHandCursor);
            m_bMove =true;
        }
        else{
            switch (m_curRegion) {
            case SIMPLEROI_TOP:
            case SIMPLEROI_BOTTOM:
                setCursor(Qt::SizeVerCursor);
                break;
            case SIMPLEROI_LEFT:
            case SIMPLEROI_RIGHT:
                setCursor(Qt::SizeHorCursor);
                break;
            case SIMPLEROI_TOPLEFT:
            case SIMPLEROI_BOTTOMRIGHT:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case SIMPLEROI_TOPRIGHT:
            case SIMPLEROI_BOTTOMLEFT:
                setCursor(Qt::SizeBDiagCursor);
                break;
            default:
                break;
            }
            m_bScale =true;
        }
    }
}

/**
 * @brief CaliperTool::mouseMoveEvent
 * 鼠标按下并移动时进行变换
 * @param event
 */
void SimpleROI::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons() &Qt::LeftButton){
        if(m_bMove){
            setCursor(Qt::ClosedHandCursor);
            moveShape(event->pos());
        }
        else if(m_bScale){
            scaleROI(event->pos());
        }
    }
}

/**
 * @brief CaliperTool::mouseReleaseEvent
 * 鼠标释放，一次变换完成
 * @param event
 */
void SimpleROI::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsObject::mouseReleaseEvent(event);
    m_startPos = QPointF();
    m_curRegion =SIMPLEROI_OUTSIDE;
    m_bScale = false;
    m_bMove = false;
    emit ROITransformFinished();
}

/**
 * @brief SimpleROI::judgePosition
 * 判断pos位于ROI的区域
 * @param pos
 * @return
 */
SimpleROI::SimpleROIRegion SimpleROI::judgePosition(const QPointF &pos)
{
    qreal px =pos.x(), py =pos.y();
    int delta =SHAPE_CONTROL_SIZE;
    if(std::abs(px -m_rect.x()) <=delta){
        if(std::abs(py -m_rect.y()) <=delta)
            return SIMPLEROI_TOPLEFT;
        else if(py >m_rect.y() +delta && py <m_rect.bottom() +1 -delta)
            return SIMPLEROI_LEFT;
        else if(std::abs(py -m_rect.bottom() -1) <=delta)
            return SIMPLEROI_BOTTOMLEFT;
        else
            return SIMPLEROI_OUTSIDE;
    }
    else if(px >m_rect.x() +delta && px <m_rect.right() +1 -delta){
        if(std::abs(py -m_rect.y()) <=delta)
            return SIMPLEROI_TOP;
        else if(py >m_rect.y() +delta && py <m_rect.bottom() +1 -delta)
            return SIMPLEROI_INSIDE;
        else if(std::abs(py -m_rect.bottom() -1) <=delta)
            return SIMPLEROI_BOTTOM;
        else
            return SIMPLEROI_OUTSIDE;
    }
    else if(std::abs(px -m_rect.right() -1) <=delta){
        if(std::abs(py -m_rect.y()) <=delta)
            return SIMPLEROI_TOPRIGHT;
        else if(py >m_rect.y() +delta && py <m_rect.bottom() +1 -delta)
            return SIMPLEROI_RIGHT;
        else if(std::abs(py -m_rect.bottom() -1) <=delta)
            return SIMPLEROI_BOTTOMRIGHT;
        else
            return SIMPLEROI_OUTSIDE;
    }
    else
        return SIMPLEROI_OUTSIDE;
}

/**
 * @brief SimpleROI::getTranslatedRect 平移变换
 * @param before
 * @param dx
 * @param dy
 * @return
 */
QRect SimpleROI::getTranslatedRect(const QRect &before, int dx, int dy)
{
    QTransform matrix(1, 0, 0, 1, dx, dy);
    return matrix.mapRect(before);
}

/**
 * @brief SimpleROI::getScaledRect 缩放变换
 * @param before
 * @param basePos
 * @param sx
 * @param sy
 * @return
 */
QRect SimpleROI::getScaledRect(const QRect &before, QPoint basePos, qreal sx, qreal sy)
{
    QTransform matrix(sx, 0, 0, sy, basePos.x() *(1 -sx), basePos.y() *(1 -sy));
    return matrix.mapRect(before);
}

/**
 * @brief SimpleROI::scaleROI
 * 根据拖动位置对ROI进行缩放
 * @param mousePoint
 */
void SimpleROI::scaleROI(const QPointF &mousePoint)
{
    int curX =qRound(mousePoint.x());
    int curY =qRound(mousePoint.y());
    int coord, coord1;
    int minLen =SHAPE_RECT_MINLENGTH;
    prepareGeometryChange();
    switch (m_curRegion) {
    case SIMPLEROI_TOP:
        coord =qMin(curY, m_rect.bottom() +1 -minLen);
        m_rect.setTop(coord);
        break;
    case SIMPLEROI_RIGHT:
        coord =qMax(curX, m_rect.left() +minLen);
        m_rect.setRight(coord -1);
        break;
    case SIMPLEROI_BOTTOM:
        coord =qMax(curY, m_rect.top() +minLen);
        m_rect.setBottom(coord -1);
        break;
    case SIMPLEROI_LEFT:
        coord =qMin(curX, m_rect.right() +1 -minLen);
        m_rect.setLeft(coord);
        break;
    case SIMPLEROI_TOPLEFT:
        coord =qMin(curY, m_rect.bottom() +1 -minLen);
        coord1 =qMin(curX, m_rect.right() +1 -minLen);
        m_rect.setTopLeft(QPoint(coord1, coord));
        break;
    case SIMPLEROI_TOPRIGHT:
        coord =qMin(curY, m_rect.bottom() +1 -minLen);
        coord1 =qMax(curX, m_rect.left() +minLen);
        m_rect.setTopRight(QPoint(coord1 -1, coord));
        break;
    case SIMPLEROI_BOTTOMRIGHT:
        coord =qMax(curY, m_rect.top() +minLen);
        coord1 =qMax(curX, m_rect.left() +minLen);
        m_rect.setBottomRight(QPoint(coord1 -1, coord -1));
        break;
    case SIMPLEROI_BOTTOMLEFT:
        coord =qMax(curY, m_rect.top() +minLen);
        coord1 =qMin(curX, m_rect.right() +1 -minLen);
        m_rect.setBottomLeft(QPoint(coord1, coord -1));
        break;
    default:
        break;
    }

    update();
}

/**
 * @brief SimpleROI::moveShape
 * 拖动鼠标对ROI进行平移
 * @param mousePoint
 */
void SimpleROI::moveShape(const QPointF &mousePoint)
{
    prepareGeometryChange();
    QPointF distance =mousePoint -m_startPos;
    m_rect =getTranslatedRect(m_rect, qRound(distance.x()), qRound(distance.y()));
    m_startPos =mousePoint;
    update();
}

void SimpleROI::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(Qt::darkBlue);
    pen.setWidth(SHAPE_THICK);
    painter->setPen(pen);
    painter->drawRect(m_rect);

#ifdef DRAW_EIGHT_POINT
    painter->setBrush(Qt::red);
    pen.setColor(Qt::darkRed);
    pen.setWidthF(0.1 *SHAPE_THICK);
    painter->setPen(pen);
    QPointF dx(m_rect.width()/2, 0);
    QPointF dy(0, m_rect.height()/2);
    QPoint topR =m_rect.topRight() +QPoint(1, 0);
    QPoint bottomL =m_rect.bottomLeft() +QPoint(0, 1);
    QPoint bottomR =m_rect.bottomRight() +QPoint(1, 1);
    QVector<QPointF> centers;
    centers <<m_rect.topLeft() <<m_rect.topLeft() +dx <<topR
           <<m_rect.topLeft() +dy <<topR +dy
          <<bottomL <<bottomL +dx <<bottomR;
    QPointF offset(ROIRECT_SIZE /2, ROIRECT_SIZE /2);
    for(int i =0; i <centers.size(); ++i){
        QRectF little(centers[i] -offset, centers[i] +offset);
        painter->drawRect(little);
    }
#endif
}



//class CaliperTool

CaliperTool::CaliperTool()
{
    QVector<QPointF> vec;
    vec <<QPointF(0, 0) <<QPointF(160, 0) <<QPointF(160, 40) <<QPointF(0, 40) <<QPointF(0, 0);
    m_curRegion =CALIPER_NONE;
    m_bMove =m_bScale =m_bRotate =m_bShear =false;
    m_startPos =QPointF(0, 0);
    m_angle =0;
    m_shearAngle =0;
    m_mainShape =QPolygonF(vec);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

CaliperTool::~CaliperTool()
{

}

QRectF CaliperTool::boundingRect() const
{
    QRectF rect =m_mainShape.boundingRect();
    QPointF origin(rect.x() -SHAPE_CONTROL_SIZE, rect.y() -SHAPE_CONTROL_SIZE);
    QSizeF size(rect.width() +2 *SHAPE_CONTROL_SIZE +2, rect.height() +2 *SHAPE_CONTROL_SIZE +2);
    return QRectF(origin, size);
}

/**
 * @brief CaliperTool::reInitialize  重新初始化，只针对旋转和倾斜
 */
void CaliperTool::reInitialize()
{
    if(m_angle ==0 && m_shearAngle ==0)  return;
    prepareGeometryChange();
    if(m_angle !=0){
        m_mainShape =getRotatedPolygon(m_mainShape, centre(), -m_angle);
        m_angle =0;
    }
    if(m_shearAngle !=0){
        m_mainShape =getShearPolygon(m_mainShape, -m_shearAngle);
        m_shearAngle =0;
    }
    update();
}

/**
 * @brief CaliperTool::vertexes  返回卡尺ROI四个顶点
 * @return
 */
std::vector<QPointF> CaliperTool::vertexes() const
{
    QVector<QPointF> vec;
    for(int i =0; i <4; i++){
        vec <<m_mainShape[i];
    }
    return std::vector<QPointF>(vec.begin(), vec.end());
}

/**
 * @brief CaliperTool::mousePressEvent
 * 根据鼠标按下位置，确定变换类型
 * @param event
 */
void CaliperTool::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() ==Qt::LeftButton){
        m_startPos =event->pos();
        m_curRegion =judgePosition(event->pos());
        switch (m_curRegion) {
        case CALIPER_TOPLEFT:
        case CALIPER_TOPRIGHT:
        case CALIPER_BOTTOMRIGHT:
        case CALIPER_BOTTOMLEFT:
            setCursor(Qt::CrossCursor);
            m_bScale =true;
            break;
        case CALIPER_RIGHTMIDDLE:
            setCursor(Qt::PointingHandCursor);
            m_bRotate =true;
            break;
        case CALIPER_BOTTOMMIDDLE:
            setCursor(Qt::PointingHandCursor);
            m_bShear =true;
            break;
        case CALIPER_TOP:
        case CALIPER_RIGHT:
        case CALIPER_BOTTOM:
        case CALIPER_LEFT:
            setCursor(Qt::SizeAllCursor);
            m_bMove =true;
            break;
        default:
            break;
        }
    }
}


/**
 * @brief CaliperTool::mouseMoveEvent
 * 鼠标按下并移动时进行变换
 * @param event
 */
void CaliperTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        QPointF mousePos =event->pos();
        if(m_bMove){
            setCursor(Qt::ClosedHandCursor);
            move(mousePos);
        }
        else if(m_bScale){
            scale(mousePos);
        }
        else if(m_bRotate){
            rotate(mousePos);
        }
        else if(m_bShear)
        {
            shear(mousePos);
        }
    }
}

/**
 * @brief CaliperTool::mouseReleaseEvent
 * 鼠标释放，一次变换完成
 * @param event
 */
void CaliperTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsObject::mouseReleaseEvent(event);
    m_curRegion =CALIPER_NONE;
    m_bMove =m_bScale =m_bRotate =m_bShear =false;
}

/**
 * @brief CaliperTool::paint  绘制卡尺ROI
 * @param painter
 * @param option
 * @param widget
 */
void CaliperTool::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int ctrlLen =SHAPE_CONTROL_SIZE -1;

    QPen pen(Qt::blue);
    pen.setWidthF(0.5);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPolygon(m_mainShape);

    pen.setColor(QColor(102, 204, 102));
    painter->setPen(pen);
    QPointF rmid =(m_mainShape[1] +m_mainShape[2]) /2;
    QPointF bmid =(m_mainShape[2] +m_mainShape[3]) /2;
    QPointF tmid =(m_mainShape[0] +m_mainShape[1]) /2;
    QPointF lmid =(m_mainShape[3] +m_mainShape[0]) /2;
    QPointF arrowPt(rmid.x(), rmid.y() +ctrlLen);
    QPointF ass1(tmid.x() -3, tmid.y() -2);
    QPointF ass2(tmid.x() -3, tmid.y() +2);
    QPointF ass3(lmid.x() -2, lmid.y() -3);
    QPointF ass4(lmid.x() +2, lmid.y() -3);
    QVector<QLineF> lines;
    lines <<QLineF(arrowPt, QPointF(arrowPt.x() +1, arrowPt.y() -2))
          <<QLineF(arrowPt, QPointF(arrowPt.x() +2, arrowPt.y() +1))
          <<QLineF(tmid, getRotatedPoint(ass1, tmid, m_angle))
          <<QLineF(tmid, getRotatedPoint(ass2, tmid, m_angle))
          <<QLineF(lmid, getRotatedPoint(ass3, lmid, m_angle +m_shearAngle))
          <<QLineF(lmid, getRotatedPoint(ass4, lmid, m_angle +m_shearAngle));
    painter->drawLines(lines);
    painter->drawArc(qRound(rmid.x() -ctrlLen), qRound(rmid.y() -ctrlLen),
                     2 *ctrlLen, 2 *ctrlLen, 270 *16, 270 *16);
    QVector<QPointF> vec1;
    QPointF offset1(ctrlLen /2, 0);
    vec1 <<QPointF(bmid.x() -ctrlLen, bmid.y() -ctrlLen) +offset1
         <<QPointF(bmid.x() +ctrlLen, bmid.y() -ctrlLen) +offset1
         <<QPointF(bmid.x() +ctrlLen, bmid.y() +ctrlLen) -offset1
         <<QPointF(bmid.x() -ctrlLen, bmid.y() +ctrlLen) -offset1
         <<QPointF(bmid.x() -ctrlLen, bmid.y() -ctrlLen) +offset1;
    painter->drawPolygon(QPolygonF(vec1));
}

/**
 * @brief CaliperTool::judgePosition
 * 判断点在ROI哪个位置
 * @param pos
 * @return
 */
CaliperTool::CaliperRegion CaliperTool::judgePosition(const QPointF &pos)
{
    QPointF rmid =(m_mainShape[1] +m_mainShape[2]) /2;
    if(getDistance(rmid, pos) <=SHAPE_CONTROL_SIZE)  return CALIPER_RIGHTMIDDLE;

    QPointF bmid =(m_mainShape[2] +m_mainShape[3]) /2;
    if(getDistance(bmid, pos) <=SHAPE_CONTROL_SIZE)  return CALIPER_BOTTOMMIDDLE;

    for(int i =0; i <4; i++){
        qreal dist =getDistance(pos, m_mainShape[i]);
        if(dist <=SHAPE_CONTROL_SIZE)
            return CaliperRegion(1 +i);
    }

    for(int i =0; i <4; i++){
        qreal dist =getDistance(pos, m_mainShape[i], m_mainShape[i +1]);
        if(dist <=SHAPE_CONTROL_SIZE)
            return CaliperRegion(7 +i);
    }

    return CALIPER_NONE;
}

/**
 * @brief CaliperTool::centre  ROI中心
 * @return
 */
QPointF CaliperTool::centre() const
{
    QPointF c1 =(m_mainShape[0] +m_mainShape[2]) /2;
    QPointF c2 =(m_mainShape[1] +m_mainShape[3]) /2;
    return (c1 +c2) /2;
}

/**
 * @brief CaliperTool::move  移动
 * @param pos
 */
void CaliperTool::move(const QPointF &pos)
{
    QPointF delta =pos -m_startPos;
    QTransform translt(1, 0, 0, 1, delta.x(), delta.y());
    prepareGeometryChange();
    m_mainShape =translt.map(m_mainShape);
    m_startPos =pos;
    update();
}

/**
 * @brief CaliperTool::scale  缩放
 * @param pos
 */
void CaliperTool::scale(const QPointF &pos)
{
    QPolygonF tempPoly0 =getRotatedPolygon(m_mainShape, centre(), -m_angle);
    QPolygonF tempPoly =getShearPolygon(tempPoly0, -m_shearAngle);
    QPointF pos1 =getRotatedPoint(pos, centre(), -m_angle);
    QPointF newPos, basePos;
    int regress =0;
    if(m_curRegion ==CALIPER_TOPLEFT){
        newPos =tempPoly[0] +pos1 -tempPoly0[0];
        basePos =tempPoly[2];
        if(basePos.x() -newPos.x() <g_minLen)
            newPos.rx() =basePos.x() -g_minLen;
        if(basePos.y() -newPos.y() <g_minLen /2)
            newPos.ry() =basePos.y() -g_minLen /2;
        tempPoly[0] =newPos;
        tempPoly[1].ry() =newPos.y();
        tempPoly[3].rx() =newPos.x();
        regress =2;
    }
    else if(m_curRegion ==CALIPER_TOPRIGHT){
        newPos =tempPoly[1] +pos1 -tempPoly0[1];
        basePos =tempPoly[3];
        if(newPos.x() -basePos.x() <g_minLen)
            newPos.rx() =basePos.x() +g_minLen;
        if(basePos.y() -newPos.y() <g_minLen /2)
            newPos.ry() =basePos.y() -g_minLen /2;
        tempPoly[1] =newPos;
        tempPoly[2].rx() =newPos.x();
        tempPoly[0].ry() =newPos.y();
        regress =3;
    }
    else if(m_curRegion ==CALIPER_BOTTOMRIGHT){
        newPos =tempPoly[2] +pos1 -tempPoly0[2];
        basePos =tempPoly[0];
        if(newPos.x() -basePos.x() <g_minLen)
            newPos.rx() =basePos.x() +g_minLen;
        if(newPos.y() -basePos.y() <g_minLen /2)
            newPos.ry() =basePos.y() +g_minLen /2;
        tempPoly[2] =newPos;
        tempPoly[3].ry() =newPos.y();
        tempPoly[1].rx() =newPos.x();
        regress =0;
    }
    else if(m_curRegion ==CALIPER_BOTTOMLEFT){
        newPos =tempPoly[3] +pos1 -tempPoly0[3];
        basePos =tempPoly[1];
        if(basePos.x() -newPos.x() <g_minLen)
            newPos.rx() =basePos.x() -g_minLen;
        if(newPos.y() -basePos.y() <g_minLen /2)
            newPos.ry() =basePos.y() +g_minLen /2;
        tempPoly[3] =newPos;
        tempPoly[0].rx() =newPos.x();
        tempPoly[2].ry() =newPos.y();
        regress =1;
    }
    tempPoly[4] =tempPoly[0];
    tempPoly =getShearPolygon(tempPoly, m_shearAngle);
    tempPoly =getRotatedPolygon(tempPoly, centre(), m_angle);
    QPointF delta =m_mainShape[regress] -tempPoly[regress];
    prepareGeometryChange();
    m_mainShape =getMovedPolygon(tempPoly, delta.x(), delta.y());
    m_startPos =pos;
    update();
}

/**
 * @brief CaliperTool::rotate  旋转
 * @param pos
 */
void CaliperTool::rotate(const QPointF &pos)
{
    qreal oldAngle =m_angle;
    m_angle =atan2(pos.y() -centre().y(), pos.x() -centre().x());
    qreal delta =m_angle -oldAngle;
    prepareGeometryChange();
    m_mainShape =getRotatedPolygon(m_mainShape, centre(), delta);
    update();
}

/**
 * @brief CaliperTool::shear  倾斜
 * @param pos
 */
void CaliperTool::shear(const QPointF &pos)
{
    qreal nowAngle =atan2(pos.y() -centre().y(), pos.x() -centre().x()) -Pi /2 -m_angle;
    qreal delta =nowAngle -m_shearAngle;
    QPolygonF tempPoly =getShearPolygon(m_mainShape, delta);
    qreal height =getDistance(tempPoly[0], tempPoly[2], tempPoly[3]);
    qreal length =getDistance(tempPoly[0], tempPoly[3]);
    QPointF p0 =getRotatedPoint(tempPoly[0], centre(), -m_angle);
    QPointF p1 =getRotatedPoint(tempPoly[3], centre(), -m_angle);
    if(p0.y() <=p1.y() && height >=length *sin(g_minAngle *Pi /180)){
        prepareGeometryChange();
        m_mainShape =getShearPolygon(m_mainShape, delta);
        m_shearAngle =nowAngle;
    }
    update();
}



//class SimpleMovablePoint

SimpleMovablePoint::SimpleMovablePoint()
{
    m_point =QPoint(20, 20);
}

SimpleMovablePoint::~SimpleMovablePoint()
{

}

QRectF SimpleMovablePoint::boundingRect() const
{
    QPointF origin =m_point -QPointF(SIMPLE_POINT_WIDTH +1, SIMPLE_POINT_WIDTH +1);
    return QRectF(origin, QSize(SIMPLE_POINT_WIDTH +1, SIMPLE_POINT_WIDTH +1) *2);
}

/**
 * @brief SimpleMovablePoint::positionOnScene  坐标转换到Scene坐标系
 * @return
 */
QPoint SimpleMovablePoint::positionOnScene() const
{
    return mapToScene(m_point).toPoint();
}

/**
 * @brief SimpleMovablePoint::moveTo  移动
 * @param x
 * @param y
 */
void SimpleMovablePoint::moveTo(int x, int y)
{
    moveShape(QPointF(x, y));
}

/**
 * @brief SimpleMovablePoint::mousePressEvent  鼠标按下判断是否进入操作范围
 * @param event
 */
void SimpleMovablePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        QPointF startPos =event->pos();
        QPointF origin =m_point -QPointF(SIMPLE_POINT_WIDTH/2, SIMPLE_POINT_WIDTH/2);
        QRectF rect(origin, QSize(SIMPLE_POINT_WIDTH, SIMPLE_POINT_WIDTH));
        if(rect.contains(startPos)){
            m_isMoving =true;
            setCursor(Qt::CrossCursor);
        }
        else  return;
    }
}

/**
 * @brief SimpleMovablePoint::mouseMoveEvent  鼠标按下时移动，进行变换
 * @param event
 */
void SimpleMovablePoint::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        if(m_isMoving){
            moveShape(event->pos());
        }
    }
}

/**
 * @brief SimpleMovablePoint::mouseReleaseEvent  鼠标释放，变换完成
 * @param event
 */
void SimpleMovablePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mouseReleaseEvent(event);
    setCursor(Qt::ArrowCursor);
    m_isMoving =false;
}

/**
 * @brief SimpleMovablePoint::moveShape  移动实现
 * @param pos
 */
void SimpleMovablePoint::moveShape(const QPointF &pos)
{
    prepareGeometryChange();
    m_point =pos.toPoint();
    emit positionChanged();
    update();
}

/**
 * @brief SimpleMovablePoint::paint
 * 绘制点和标识线
 * @param painter
 * @param option
 * @param widget
 */
void SimpleMovablePoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(QColor(0, 205, 0));
    pen.setWidth(1);
    painter->setPen(pen);
    QPointF origin =m_point -QPointF(SIMPLE_POINT_WIDTH/2, SIMPLE_POINT_WIDTH/2);
    QRectF rect(origin, QSize(SIMPLE_POINT_WIDTH, SIMPLE_POINT_WIDTH));
    painter->drawRect(rect);

    int x0 =m_point.x(), y0 =m_point.y();
    QLine horline(QPoint(x0 -SIMPLE_POINT_WIDTH, y0), QPoint(x0 +SIMPLE_POINT_WIDTH, y0));
    QLine verline(QPoint(x0, y0 -SIMPLE_POINT_WIDTH), QPoint(x0, y0 +SIMPLE_POINT_WIDTH));
    painter->drawLine(horline);
    painter->drawLine(verline);
}



// auxiliary functions

/**
 * @brief getDistance  两点距离
 * @param pt1
 * @param pt2
 * @return
 */
qreal getDistance(const QPointF &pt1, const QPointF &pt2)
{
    return sqrt(pow(pt2.x() -pt1.x(), 2) +pow(pt2.y() -pt1.y(), 2));
}

/**
 * @brief getDistance  点到直线距离
 * @param pt
 * @param A
 * @param B
 * @return
 */
qreal getDistance(const QPointF &pt, const QPointF &A, const QPointF &B)
{
    if(A == B) return -1;
    qreal mz =(pt.x() -A.x()) *(B.y() -A.y()) - (pt.y() -A.y()) *(B.x() -A.x());
    mz *=mz;
    qreal mm =pow(B.x() -A.x(), 2) +pow(B.y() -A.y(), 2);
    return sqrt(mz /mm);
}

/**
 * @brief getRotatedPoint  旋转某点
 * @param pt
 * @param center
 * @param angle
 * @return
 */
QPointF getRotatedPoint(const QPointF &pt, const QPointF &center, qreal angle)
{
    QTransform translt1(1, 0, 0, 1, -center.x(), -center.y());
    QTransform rotat(cos(angle), sin(angle), 0,
                     -sin(angle), cos(angle), 0,
                     0, 0, 1);
    QTransform translt2(1, 0, 0, 1, center.x(),center.y());
    translt1 *=rotat;
    translt1 *=translt2;
    return translt1.map(pt);
}

/**
 * @brief getRotatedPolygon  旋转多边形
 * @param poly
 * @param center
 * @param angle
 * @return
 */
QPolygonF getRotatedPolygon(const QPolygonF &poly, const QPointF &center, qreal angle)
{
    QPolygonF res;
    for(int i =0; i <poly.size(); i++){
        res.append(getRotatedPoint(poly[i], center, angle));
    }
    return res;
}

/**
 * @brief getShearPolygon  倾斜多边形
 * @param poly
 * @param angle
 * @return
 */
QPolygonF getShearPolygon(const QPolygonF &poly, qreal angle)
{
    QPolygonF res;
    QPointF lmid =(poly[0] +poly[3]) /2;
    QPointF rmid =(poly[1] +poly[2]) /2;
    res <<getRotatedPoint(poly[0], lmid, angle)
        <<getRotatedPoint(poly[1], rmid, angle)
        <<getRotatedPoint(poly[2], rmid, angle)
        <<getRotatedPoint(poly[3], lmid, angle)
        <<getRotatedPoint(poly[0], lmid, angle);
    return res;
}

/**
 * @brief getMovedPolygon  移动多边形
 * @param poly
 * @param dx
 * @param dy
 * @return
 */
QPolygonF getMovedPolygon(const QPolygonF &poly, qreal dx, qreal dy)
{
    QTransform translt(1, 0, 0, 1, dx, dy);
    return translt.map(poly);
}

/**
 * @brief GlobalFuncs::addElementWithText XML工具，为parent添加一个子节点，该子节点包含一个text节点作为数据(参数data)
 * @param document  文件指针
 * @param parent  父节点
 * @param tagName  子节点标签
 * @param data  子节点text
 */
void addElementWithText(QDomDocument *document, QDomElement *parent, const QString &tagName, const QString &data)
{
    QDomElement elem =document->createElement(tagName);
    QDomText text =document->createTextNode(data);
    elem.appendChild(text);
    parent->appendChild(elem);
}
