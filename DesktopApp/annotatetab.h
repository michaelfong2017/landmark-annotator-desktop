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
#include <QtNetwork>
#include "qnetworkclient.h"
#include "twolinesdialog.h"

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
    float* getScalingFactor();
    std::map<std::string, QVector3D>* getAnnotations3D();
    void computeMetrics();
    void setAiImageUrl(QString aiImageUrl);

    float scalingFactorFromRightToLeft;

    // Landmark predictions
    // Conceptually should be private but in order to avoid long script, public is used.
    int imageId;
    float predictedA1X;
    float predictedA1Y;
    float predictedA2X;
    float predictedA2Y;
    float predictedB1X;
    float predictedB1Y;
    float predictedB2X;
    float predictedB2Y;
    float predictedCX;
    float predictedCY;
    float predictedDX;
    float predictedDY;
    // Landmark predictions END

private:
    cv::Mat depthToColorImage;

    QImage qColorImage;
    QImage qDepthToColorColorizedImage;

    QImage annotatedColorImage;
    QImage annotatedDepthToColorColorizedImage;
    std::map<std::string, QPointF> annotationsOnRight;
    std::map<std::string, QVector3D> annotations3D;
    DesktopApp* parent;
    DragAndDropGraphicsScene* colorScene;
    DragAndDropGraphicsScene* depthToColorScene;
    float scalingFactorForRight;

    QString aiImageUrl;

    void drawAnnotations();
    QJsonDocument getAnnotationsJson();

    // Metrics
    float distance1; // Distance between b1 and b2 in cm
    float distance2; // Distance between c1 and c2 in cm
    float angle1; // Distance between b1-b2 line and xy-plane in deg
    float angle2; // Angle between c1-c2 line and xy-plane in deg

private slots:
    void onConfirmLandmarks(QNetworkReply* reply);
    //void onDownloadImage(QNetworkReply* reply);
};

// Helper functions
QPointF getRandomPoint(int maxWidth, int maxHeight);
#endif
