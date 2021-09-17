# ROIGraphics
在QGraphicsView基础上实现的机器视觉软件中用到的ROI工具

## 功能
- SimpleROI：简单的矩形ROI，交互功能包括平移、缩放，用于鼠标选取处理范围
- CaliperROI：旋转矩形ROI，交互功能包括平移、缩放、旋转、倾斜，可用于选取处理范围，同时用于卡尺测量
- SimpleMovablePoint：可交互点，通过鼠标移动获取/显示某个位置
- DispImageView：可交互视图窗口，用于显示图像和其他图形，是上述ROI图形的容器

## 环境
Qt 5.14.1
OpenCV 3.4.10