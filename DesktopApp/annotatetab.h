#ifndef ANNOTATETAB_H
#define ANNOTATETAB_H
#define NUM_ANNOTATIONS 6

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "helper.h"
#include "desktopapp.h"
#include <k4a/k4a.hpp>
#include <opencv2/opencv.hpp>
#include "draganddropgraphicsscene.h"
#include "capturetab.h"

enum class ImageType {Color, DepthToColor};

class AnnotateTab : public QWidget
{
    Q_OBJECT

public:
    AnnotateTab(DesktopApp* parent);

    void reloadCurrentImage();

    cv::Mat getDepthToColorImage();

    QImage* getQColorImage();
    QImage* getAnnotatedColorImage();
    QImage* getAnnotatedDepthToColorColorizedImage();
    std::map<std::string, QPointF>* getAnnotations();
    void setAnnotationsText();
    void recopyAnnotatedImage();
    DesktopApp* getParent();
    DragAndDropGraphicsScene* getColorScene();
    DragAndDropGraphicsScene* getDepthToColorScene();
    int* getScalingFactor();
    std::map<std::string, QVector3D>* getAnnotations3D();
    void computeMetrics();

private:
    cv::Mat depthToColorImage;

    QImage qColorImage;
    QImage qDepthToColorColorizedImage;

    QImage annotatedColorImage;
    QImage annotatedDepthToColorColorizedImage;
    std::map<std::string, QPointF> annotations;
    std::map<std::string, QVector3D> annotations3D;
    DesktopApp* parent;
    DragAndDropGraphicsScene* colorScene;
    DragAndDropGraphicsScene* depthToColorScene;
    int scalingFactor;
    void drawAnnotations();
    QJsonDocument getAnnotationsJson();

    // Metrics
    float distance1; // Distance between b1 and b2 in cm
    float distance2; // Distance between c1 and c2 in cm
    float angle1; // Distance between b1-b2 line and xy-plane in deg
    float angle2; // Angle between c1-c2 line and xy-plane in deg
};

// Helper functions
QPointF getRandomPoint(int maxWidth, int maxHeight);
#endif
