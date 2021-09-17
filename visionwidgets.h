#ifndef VISIONWIDGETS_H
#define VISIONWIDGETS_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QWheelEvent>
#include <QDockWidget>
#include <QToolBar>

class DispImageView :public QGraphicsView
{
    Q_OBJECT
public:
    DispImageView(QWidget *parent =nullptr);
    ~DispImageView();

    QGraphicsScene* myScene()  { return &m_scene;}
    void setBackImage(const QImage &img);
protected:
    void wheelEvent(QWheelEvent *event);

private:
    QGraphicsScene m_scene;
    QGraphicsPixmapItem m_pixmap;
    qreal m_zoomDelta;

    void zoom(qreal scaleFactor);
};

class QLabel;
class MyToolBar;

class DockWindow :public QDockWidget
{
    Q_OBJECT

public:
    DockWindow(QWidget *parent =nullptr, bool isClosable =true);
    ~DockWindow();

    void setTitle(const QString &text);
    MyToolBar* dockWindowToolBar() const;

private:
    MyToolBar *m_toolBar;
};

class MyToolBar :public QToolBar
{
    friend class DockWindow;

public:
    MyToolBar(QWidget *parent =nullptr);
    MyToolBar(const QString &title, QWidget *parent =nullptr);
    ~MyToolBar();

    void setTitleLabel(const QString &title);
    void addActionToLeft(QAction *action);
    QAction* addWidgetToLeft(QWidget *widget);
    QAction *addSeparatorToLeft();

    QSize sizeHint() const override;
private:
    QLabel *m_title =nullptr;
    void initialize();
};

#endif // VISIONWIDGETS_H
