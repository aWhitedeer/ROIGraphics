#include "visionwidgets.h"
#include "visioncom.h"

#include <QDebug>
#include <QLayout>
#include <QSplitter>
#include <QLabel>

//class DispImageView  图像显示窗口

DispImageView::DispImageView(QWidget *p) :QGraphicsView(p), m_zoomDelta(0.1)
{
    m_scene.addItem(&m_pixmap);
    setScene(&m_scene);
}

DispImageView::~DispImageView()
{

}

void DispImageView::setBackImage(const QImage &img)
{
    m_pixmap.setPixmap(QPixmap::fromImage(img));
}

void DispImageView::wheelEvent(QWheelEvent *event)
{
    QPointF delta =event->angleDelta();
    delta.y() >0? zoom(1 +m_zoomDelta) :zoom(1 -m_zoomDelta);
    scene()->update();
}

void DispImageView::zoom(qreal scaleFactor)
{
    qreal factor =transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if(factor <0.05 || factor >50)  return;
    scale(scaleFactor, scaleFactor);
}


//class DockWindow  自定义悬靠窗口

DockWindow::DockWindow(QWidget *p, bool b) :QDockWidget(p)
{
    m_toolBar =new MyToolBar(this);
    QWidget *pTitle =titleBarWidget();
    setTitleBarWidget(m_toolBar);
    if(pTitle)  delete pTitle;

    if(b){
        QWidget *spacer =new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_toolBar->addWidget(spacer);
        QAction *hideAction =m_toolBar->addAction(QIcon(), QString());
        hideAction->setIcon(QIcon(":/icons/icon-delete"));
        connect(hideAction, &QAction::triggered, this, &QWidget::hide);
    }

    setFeatures(0);
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

DockWindow::~DockWindow()
{

}

void DockWindow::setTitle(const QString &title)
{
    m_toolBar->setTitleLabel(title);
}

MyToolBar* DockWindow::dockWindowToolBar() const
{
    return m_toolBar;
}


//class MyToolBar  自定义工具栏

MyToolBar::MyToolBar(QWidget *p) :QToolBar(p)
{
    initialize();
}

MyToolBar::MyToolBar(const QString &title, QWidget *p) :QToolBar(title, p)
{
    initialize();
}

MyToolBar::~MyToolBar()
{

}

void MyToolBar::setTitleLabel(const QString &title)
{
    if(m_title){
        m_title->setText(title +" ");
    }
    else{
        QAction *separator0;
        if(actions().isEmpty())
            separator0 =addSeparator();
        else
            separator0 =insertSeparator(actions().at(0));
        m_title =new QLabel(title +" ", this);
        insertWidget(separator0, m_title);
    }
}

void MyToolBar::addActionToLeft(QAction *action)
{
    if(actions().isEmpty()){
        addAction(action);
        return;
    }
    QAction *after;
    if(m_title){
        if(actions().size() <=2)
            addAction(action);
        else{
            after =actions().at(2);
            insertAction(after, action);
        }
    }
    else{
        after =actions().at(0);
        insertAction(after, action);
    }
}

QAction* MyToolBar::addWidgetToLeft(QWidget *widget)
{
    if(actions().isEmpty())
        return addWidget(widget);
    QAction *after;
    if(m_title){
        if(actions().size() <=2)
            return addWidget(widget);
        else{
            after =actions().at(2);
            return insertWidget(after, widget);
        }
    }
    else{
        after =actions().at(0);
        return insertWidget(after, widget);
    }
}

QAction* MyToolBar::addSeparatorToLeft()
{
    if(actions().isEmpty())
        return addSeparator();
    QAction *after;
    if(m_title){
        if(actions().size() <=2)
            return addSeparator();
        else{
            after =actions().at(2);
            return insertSeparator(after);
        }
    }
    else{
        after =actions().at(0);
        return insertSeparator(after);
    }
}

QSize MyToolBar::sizeHint() const
{
    return QSize(84, 25);
}

void MyToolBar::initialize()
{
    setStyleSheet("background-color:lightblue");
}

