#ifndef CAPTURETAB_H
#define CAPTURETAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "helper.h"
#include "annotatetab.h"
#include "patienttab.h"
#include "desktopapp.h"
#include "recorder.h"
#include <k4a/k4a.hpp>
#include <opencv2/opencv.hpp>
#include "util/networkutil.h"
#include "qnetworkclient.h"
#include "twolinesdialog.h"
#include "noteditabledelegate.h"
#include "capturehistory.h"
#include "clipgraphicsscene.h"
#include "clipgraphicspixmapitem.h"
#include "uploadprogressdialog.h"

class CaptureTab: public QWidget
{
    Q_OBJECT

public:
    CaptureTab(DesktopApp* parent);
    DesktopApp* getParent();
    QTimer* timer;
    int getCaptureCount();
    void setCaptureCount(int newCaptureCount);
    Recorder* getRecorder();
    QString getCaptureFilepath();
    void setCaptureFilepath(QString captureFilepath);

    bool isUploading;
    cv::Mat getCapturedColorImage();
    cv::Mat getCapturedDepthImage();
    cv::Mat getCapturedColorToDepthImage();
    cv::Mat getCapturedDepthToColorImage();

    cv::Mat getFourChannelPNG();

    cv::Mat computeNormalizedDepthImage(cv::Mat);

    QImage getQColorImage();
    QImage getQDepthImage();
    QImage getQColorToDepthImage();
    QImage getQDepthToColorImage();
    //QImage getQDepthToColorColorizedImage();

    void clearCaptureHistories();

    std::vector<CaptureHistory> getCaptureHistories();
    void setCaptureHistories(int selectedImageIndex, CaptureHistory captureHistory);
    int getSelectedImageIndex();

    /** Crop image */
    int max_clip_width = 0;
    int max_clip_height = 0;
    QRect clip_rect;
    QPoint drag_offset = QPoint();
    std::tuple<QPoint, QPoint, QPoint, QPoint> handle_offsets = std::make_tuple(QPoint(0, 0), QPoint(-8, 0), QPoint(0, -8), QPoint(-8, -8));

    QRect corner(int number);

    cv::Rect cropRect;
    /** Crop image END */

    void onEnterOfflineMode();
    void onExitOfflineMode();

private:
    DesktopApp* parent;

    // Only captured images have to be stored
    // Live preview images are not stored
    cv::Mat capturedColorImage;
    cv::Mat capturedDepthImage;
    cv::Mat capturedColorToDepthImage;
    cv::Mat capturedDepthToColorImage;

    cv::Mat FourChannelPNG;
    // stored captured images END

    QImage qColorImage;
    QImage qDepthImage;
    QImage qColorToDepthImage;
    QImage qDepthToColorImage;
    //QImage qDepthToColorColorizedImage;

    cv::Mat RANSACImage;

    int imageType = -1; // Update on image selection
    QString imageName = "";
    int imageTypeBeingAnalyzed = -1; // Update on analysis button pressed

    /** Store histories of images for selection */
    std::vector<CaptureHistory> captureHistories;
    /** Store histories of images for selection END */

    int captureCount;
    Recorder* recorder;
    QString captureFilepath;
    QElapsedTimer recordingElapsedTimer;
    QNetworkAccessManager manager;
    bool noImageCaptured;
    void afterSensorImagesAcquired();
    void setDefaultCaptureMode();
    void disableButtonsForUploading();
    void enableButtonsForUploading();
    void registerRadioButtonOnClicked(QRadioButton* radioButton, QImage* image);
    void alertIfMoving(float gyroX, float gyroY, float gyroZ, float accX, float accY, float accZ);
    void onManagerFinished(QNetworkReply* reply);
    void displayCapturedImages();
    int getImageTypeFromDescription(QString description);

    /** For sending findLandmarkPredictions() more than once */
    int landmarkRequestSent = 0;
    int MAX_LANDMARK_REQUEST_SENT = 5;
    std::string currentCapturingPatientName;
    int currentImageId;
    /** For sending findLandmarkPredictions() more than once END */

    /** Select image table view */
    int selectedImageIndex = -1;
    int imageBeingAnalyzedTableViewRow = -1;
    int storedTableViewRow = -1;
    QTableView* tableView;
    QStandardItemModel* dataModel;
    /** Select image table view END */

    UploadProgressDialog* uploadProgressDialog;

private slots:
    void onUploadImage(QNetworkReply* reply);
    void onBindImageUrl(QNetworkReply* reply);
    void onFindLandmarkPredictions(QNetworkReply* reply);
    void onSlotRowSelected(const QModelIndex& current, const QModelIndex& previous);
};
#endif
