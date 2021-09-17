#include "widget.h"
#include "ui_widget.h"

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QFileDialog>
#include <QTransform>
#include "visioncom.h"
#include "visionwidgets.h"
#include "simpleroi.h"

using namespace cv;
using std::vector;

template <typename T>
void printMat(Mat &src)
{
    for(int i =0; i <src.rows; ++i){
        for(int j =0; j <src.cols -1; ++j){
            std::cout<<src.at<T>(i, j)<<", ";
        }
        if(src.cols >0)  std::cout<<src.at<T>(i, src.cols -1);
        std::cout<<std::endl;
    }
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_imageView =new DispImageView(this);
    ui->showLayout->addWidget(m_imageView);

    m_ROI =new SimpleROI;
    m_ROI->setZValue(996);
    m_caliper =new CaliperTool;   
    m_caliper->setZValue(998);
    m_point =new SimpleMovablePoint;
    m_point->setZValue(999);
    m_imageView->myScene()->addItem(m_ROI);
    m_imageView->myScene()->addItem(m_caliper);
    m_imageView->myScene()->addItem(m_point);
    m_caliper->hide();
    m_point->hide();

    m_resItem =new QGraphicsSimpleTextItem("");
    m_resItem->setBrush(Qt::green);
    m_resItem->setFont(QFont("Microsoft YaHei"));
    m_resItem->setPos(100, 100);
    m_imageView->myScene()->addItem(m_resItem);

    connect(ui->ROIBox, SIGNAL(activated(int)), this, SLOT(changeROI(int)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::showImageOnLabel(Mat &mat)
{
    QImage image =cvMat2QImage(mat);
    m_imageView->setBackImage(image);
}

void Widget::on_getpicBt_clicked()
{
    QString path =QFileDialog::getOpenFileName(this, "get image", "D:/QtDemos/Pictures/QiHe", "Image(*jpg *jpeg *png *bmp)");
    Mat mat =imread(path.toLocal8Bit().toStdString(), IMREAD_ANYCOLOR);
    if(mat.channels() ==1)
        m_input =mat.clone();
    else
        cvtColor(mat, m_input, COLOR_BGR2GRAY);
    showImageOnLabel(m_input);
}

void Widget::changeROI(int index)
{
    QList<QGraphicsObject*> temp;
    temp <<m_ROI <<m_caliper <<m_point;
    temp[index]->show();
    temp[(index +1) %3]->hide();
    temp[(index +2) %3]->hide();
}
