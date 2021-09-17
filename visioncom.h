#ifndef VISIONCOM_H
#define VISIONCOM_H

#include <opencv2/imgproc/imgproc.hpp>
#include <QImage>

QImage cvMat2QImage(const cv::Mat &mat);
cv::Mat QImage2cvMat(const QImage &image);

#endif // VISIONCOM_H
