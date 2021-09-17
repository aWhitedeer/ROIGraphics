#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <opencv2/core/core.hpp>

using cv::Mat;
class DispImageView;
class SimpleROI;
class CaliperTool;
class SimpleMovablePoint;
class QGraphicsSimpleTextItem;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    DispImageView *m_imageView;
    SimpleROI *m_ROI;
    CaliperTool *m_caliper;
    SimpleMovablePoint *m_point;
    QGraphicsSimpleTextItem *m_resItem;
    Mat m_input;
    Mat m_output;

    void showImageOnLabel(Mat &mat);

private slots:
    void on_getpicBt_clicked();
    void changeROI(int);
};
#endif // WIDGET_H
