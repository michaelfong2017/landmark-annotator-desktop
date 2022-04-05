#include "desktopapp.h"
#include "stdafx.h"
#include "patientdatatab.h"
#include "viewtab.h"
#include "capturetab.h"
#include "annotatetab.h"
#include "alignmenttab.h"

DesktopApp::DesktopApp(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // Setup azure kinect device

    uint32_t count = k4a_device_get_installed_count();
    this->deviceCount = count;

    if (count == 0) {
        this->setTextOnGraphicsViews("No kinect device found");

        return;
    }

    if (K4A_FAILED(k4a_device_open(K4A_DEVICE_DEFAULT, &this->device))) {
        this->setTextOnGraphicsViews("Can't connect to kinect device");

        return;
    }

    deviceConfig.camera_fps = K4A_FRAMES_PER_SECOND_30;
    deviceConfig.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_720P;
    deviceConfig.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    deviceConfig.depth_delay_off_color_usec = 0;

    if (K4A_FAILED(k4a_device_start_cameras(device, &this->deviceConfig))) {
        this->setTextOnGraphicsViews("Failed to start cameras");

        k4a_device_close(device);
        return;
    }

    if (K4A_FAILED(k4a_device_start_imu(device))) {
        this->setTextOnGraphicsViews("Failed to start IMU");

        k4a_device_close(device);
        return;
    }

    this->patientDataTab = new PatientDataTab(this);
    this->viewTab = new ViewTab(this);
    this->captureTab = new CaptureTab(this);
    this->annotateTab = new AnnotateTab(this);
    this->alignmentTab = new AlignmentTab(this);

    if (this->ui.tabWidget->currentIndex() == 1) viewTab->timer->start(0);
    if (this->ui.tabWidget->currentIndex() == 2) captureTab->timer->start(0);

    QObject::connect(ui.tabWidget, &QTabWidget::currentChanged, [this]() {
        switch (this->ui.tabWidget->currentIndex()) {
            case 1:
                // current tab is viewTab
                if(!this->patient.getValidity()) //If patient data is not ready
                    this->ui.tabWidget->setCurrentIndex(0);

                if (this->captureTab->getRecorder()->getRecordingStatus()) //If capture tab is recording
                    this->ui.tabWidget->setCurrentIndex(2);
                this->captureTab->timer->stop();
                this->viewTab->timer->start(0);
                break;
            case 2:
                // current tab is captureTab
                if(!this->patient.getValidity()) //If patient data is not ready
                    this->ui.tabWidget->setCurrentIndex(0);
                this->viewTab->timer->stop();
                this->captureTab->timer->start(0);
                break;
            case 3:
                // current tab is annotateTab
                if(!this->patient.getValidity()) //If patient data is not ready
                    this->ui.tabWidget->setCurrentIndex(0);

                if (this->captureTab->getRecorder()->getRecordingStatus()) //If capture tab is recording
                    this->ui.tabWidget->setCurrentIndex(2);
                this->viewTab->timer->stop();
                this->captureTab->timer->stop();
                break;
            case 4:
                // current tab is alignmentTab
                if(!this->patient.getValidity()) //If patient data is not ready
                    this->ui.tabWidget->setCurrentIndex(0);

                if (this->captureTab->getRecorder()->getRecordingStatus()) //If capture tab is recording
                    this->ui.tabWidget->setCurrentIndex(2);
                this->viewTab->timer->stop();
                this->captureTab->timer->stop();
            default:
                if (this->captureTab->getRecorder()->getRecordingStatus()) //If capture tab is recording
                    this->ui.tabWidget->setCurrentIndex(2);
                this->viewTab->timer->stop();
                this->captureTab->timer->stop();
                break;
        }
    });

}

void DesktopApp::setTextOnGraphicsViews(std::string text) {
    QGraphicsTextItem* graphicsText = new QGraphicsTextItem;
    graphicsText->setPlainText(QString::fromStdString(text));
    
    QGraphicsScene* scene = new QGraphicsScene;
    scene->addItem(graphicsText);

    ui.graphicsViewVideo->setScene(scene);
    ui.graphicsViewVideo2->setScene(scene);
    ui.graphicsViewVideo3->setScene(scene);
    ui.graphicsViewVideo4->setScene(scene);
    ui.graphicsViewVideo5->setScene(scene);
    ui.graphicsViewImage->setScene(scene);
    ui.graphicsViewAnnotation->setScene(scene);
}

#pragma region QImageMethods

QImage DesktopApp::getQColorImage() {
    k4a_image_t k4aColorImage = this->colorImageQueue.back();

    cv::Mat matColorImage = cv::Mat(k4a_image_get_height_pixels(k4aColorImage), k4a_image_get_width_pixels(k4aColorImage), CV_8UC4, k4a_image_get_buffer(k4aColorImage));

    cv::Mat temp;

    cvtColor(matColorImage, temp, cv::COLOR_BGR2RGB);

    //If recording mode is on, send temp to the output file stream
    if (this->captureTab->getRecorder()->getRecordingStatus()) {
        *(this->captureTab->getRecorder()->getColorVideoWriter()) << matColorImage;
    }

    QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    qImage.bits();

    return qImage;
}

QImage DesktopApp::getQDepthImage() {
    k4a_image_t k4aDepthImage = this->depthImageQueue.back();

    /*
    double min, max;
    cv::Mat matDepthImageRaw = cv::Mat(k4a_image_get_height_pixels(k4aDepthImage), k4a_image_get_width_pixels(k4aDepthImage), CV_16U, k4a_image_get_buffer(k4aDepthImage), cv::Mat::AUTO_STEP);

    cv::minMaxIdx(matDepthImageRaw, &min, &max);
    cv::Mat matDepthImage;
    cv::convertScaleAbs(matDepthImageRaw, matDepthImage, 255 / max);

    //If recording mode is on, send matDepthImage to the output file stream
    if (this->captureTab->getRecorder()->getRecordingStatus()) {
        *(this->captureTab->getRecorder()->getDepthVideoWriter()) << matDepthImage;
    }
    */
    uint8_t* buffer = k4a_image_get_buffer(k4aDepthImage);
    int rows = k4a_image_get_height_pixels(k4aDepthImage);
    int cols = k4a_image_get_width_pixels(k4aDepthImage);

    cv::Mat matDepthImage(rows, cols, CV_16U, (void*)buffer, cv::Mat::AUTO_STEP);

    matDepthImage.convertTo(matDepthImage, CV_8U, 255.0 / 5000.0, 0.0);

    if (this->captureTab->getRecorder()->getRecordingStatus()) {
        *(this->captureTab->getRecorder()->getDepthVideoWriter()) << matDepthImage;
    }

    /** Colorize depth image */
    //cv::Mat temp;
    //colorizeDepth(matDepthImage, temp);
    /** Colorize depth image END */
    cv::Mat temp;
    cvtColor(matDepthImage, temp, cv::COLOR_GRAY2RGB);

    QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    qImage.bits();

    return qImage;
}

QImage DesktopApp::getQColorToDepthImage() {
    QImage qEmptyImage;

    // We create a new capture here instead of retrieving color and depth images from the queue
    // Because we want the color and depth image to represent the same point in time (same capture)
    // Obtaining the last item of the depth and color queue doesn't guarantee that
    // See the full documentation here
    // https://microsoft.github.io/Azure-Kinect-Sensor-SDK/master/group___functions_gaf3a941f07bb0185cd7a72699a648fc29.html#gaf3a941f07bb0185cd7a72699a648fc29

    k4a_capture_t newCapture;
    if (k4a_device_get_capture(this->device, &newCapture, K4A_WAIT_INFINITE) != K4A_RESULT_SUCCEEDED) {
        k4a_capture_release(newCapture);
        return qEmptyImage;
    }

    if (!newCapture) {
        k4a_capture_release(newCapture);
        return qEmptyImage;
    }

    k4a_image_t k4aColorImage = k4a_capture_get_color_image(newCapture);
    k4a_image_t k4aDepthImage = k4a_capture_get_depth_image(newCapture);

    if (k4aDepthImage != NULL) {
        k4a_calibration_t calibration;
        if (k4a_device_get_calibration(this->device, this->deviceConfig.depth_mode, this->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
            return qEmptyImage;
        }

        k4a_transformation_t transformationHandle = k4a_transformation_create(&calibration);
        k4a_image_t alignmentImage;

        if (k4a_image_create(K4A_IMAGE_FORMAT_COLOR_BGRA32,
            k4a_image_get_width_pixels(k4aDepthImage),
            k4a_image_get_height_pixels(k4aDepthImage),
            k4a_image_get_width_pixels(k4aDepthImage) * 4 * (int)sizeof(uint8_t),
            &alignmentImage) != K4A_RESULT_SUCCEEDED) {
            k4a_capture_release(newCapture);
            k4a_image_release(k4aColorImage);
            k4a_image_release(k4aDepthImage);
            k4a_transformation_destroy(transformationHandle);
            k4a_image_release(alignmentImage);
            return qEmptyImage;
        }

        if (k4a_transformation_color_image_to_depth_camera(transformationHandle, k4aDepthImage, k4aColorImage, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
            k4a_capture_release(newCapture);
            k4a_image_release(k4aColorImage);
            k4a_image_release(k4aDepthImage);
            k4a_transformation_destroy(transformationHandle);
            k4a_image_release(alignmentImage);
            return qEmptyImage;
        }

        double min, max;
        cv::Mat matAlignmentImageRaw = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_8UC4, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP);

        cv::Mat temp;
        cv::cvtColor(matAlignmentImageRaw, temp, cv::COLOR_BGR2RGB);

        QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        qImage.bits();

        k4a_capture_release(newCapture);
        k4a_image_release(k4aColorImage);
        k4a_image_release(k4aDepthImage);
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        return qImage;
    }

    k4a_capture_release(newCapture);
    k4a_image_release(k4aColorImage);
    k4a_image_release(k4aDepthImage);
    return qEmptyImage;
}

QImage DesktopApp::getQDepthToColorImage() {
    k4a_image_t k4aDepthImage = this->depthImageQueue.back(), k4aColorImage = this->colorImageQueue.back();

    QImage qEmptyImage;

    if (k4aDepthImage != NULL && k4aColorImage != NULL) {
        k4a_calibration_t calibration;
        if (k4a_device_get_calibration(this->device, this->deviceConfig.depth_mode, this->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
            return qEmptyImage;
        }

        k4a_transformation_t transformationHandle = k4a_transformation_create(&calibration);
        k4a_image_t alignmentImage;

        if (k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,
            k4a_image_get_width_pixels(k4aColorImage),
            k4a_image_get_height_pixels(k4aColorImage),
            k4a_image_get_width_pixels(k4aColorImage) * (int)sizeof(uint16_t),
            &alignmentImage) != K4A_RESULT_SUCCEEDED) {
            k4a_transformation_destroy(transformationHandle);
            k4a_image_release(alignmentImage);
            return qEmptyImage;
        }

        if (k4a_transformation_depth_image_to_color_camera(transformationHandle, k4aDepthImage, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
            k4a_transformation_destroy(transformationHandle);
            k4a_image_release(alignmentImage);
            return qEmptyImage;
        }

        double min, max;
        //cv::Mat matAlignmentImageRaw = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16U, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP);
        cv::Mat matDepthImage = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16U, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP);

        matDepthImage.convertTo(matDepthImage, CV_8U, 255.0 / 5000.0, 0.0);

        if (this->captureTab->getRecorder()->getRecordingStatus()) {
            *(this->captureTab->getRecorder()->getDepthVideoWriter()) << matDepthImage;
        }

        /** Colorize depth image */
        //cv::Mat temp;
        //colorizeDepth(matDepthImage, temp);
        /** Colorize depth image END */
        cv::Mat temp;
        cvtColor(matDepthImage, temp, cv::COLOR_GRAY2RGB);

        QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        qImage.bits();

        // Get point cloud alignment and copy as a member variable of captureTab (this->captureTab->k4aPointCloud)
        if (this->alignk4APointCloud(&alignmentImage, this->captureTab->getK4aPointCloud()) != K4A_RESULT_SUCCEEDED) {
            qDebug() << "Failed to align point cloud";
        }

        // Get depth to color alignment image and copt as a member variable of captureTab (this->captureTab->k4aDepthToColor)
        if (this->copyk4aImage(&alignmentImage, this->captureTab->getK4aDepthToColor()) != K4A_RESULT_SUCCEEDED) {
            qDebug() << "Failed to copy depth to color alignment image";
        }

        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        return qImage;
    }

    return qEmptyImage;
}

#pragma endregion

#pragma region cvMatRawDataMethods

/*
* Retrieves image from colorImageQueue and transform from k4a_image_t to cv::Mat
* @return cv::Mat(1080, 1920, 8UC4), empty cv::Mat if error
*/
cv::Mat DesktopApp::getRawColorImage() {

    k4a_image_t k4aColorImage = this->colorImageQueue.back();

    if (k4aColorImage == NULL) {
        return cv::Mat{};
    }

    uint8_t* buffer = k4a_image_get_buffer(k4aColorImage);
    int rows = k4a_image_get_height_pixels(k4aColorImage);
    int cols = k4a_image_get_width_pixels(k4aColorImage);

    cv::Mat matColorImage = cv::Mat(k4a_image_get_height_pixels(k4aColorImage), k4a_image_get_width_pixels(k4aColorImage), CV_8UC4, k4a_image_get_buffer(k4aColorImage));

    return matColorImage;

}

/*
* Retrieves image from depthImageQueue and transform from k4a_image_t to cv::Mat
* @return cv::Mat(576, 640, 16UC1), empty cv::Mat if error
*/
cv::Mat DesktopApp::getRawDepthImage() {


    k4a_image_t k4aDepthImage = this->depthImageQueue.back();

    if (k4aDepthImage == NULL) {
        return cv::Mat{};
    }

    int rows = k4a_image_get_height_pixels(k4aDepthImage);
    int cols = k4a_image_get_width_pixels(k4aDepthImage);
    uint8_t* buffer = k4a_image_get_buffer(k4aDepthImage);

    cv::Mat matDepthImage = cv::Mat(rows, cols, CV_16U, (void*)buffer, cv::Mat::AUTO_STEP);
    return matDepthImage;
}

/*
* Takes a new capture and aligned the Color onto Depth
* @return cv::Mat(576, 640, 8UC4), empty cv::Mat if error
*/
cv::Mat DesktopApp::getRawColorToDepthImage() {

    k4a_capture_t newCapture;
    if (k4a_device_get_capture(this->device, &newCapture, K4A_WAIT_INFINITE) != K4A_RESULT_SUCCEEDED) {
        k4a_capture_release(newCapture);
        return cv::Mat{};
    }
    if (!newCapture) {
        k4a_capture_release(newCapture);
        return cv::Mat{};
    }

    k4a_image_t k4aColorImage = k4a_capture_get_color_image(newCapture);
    k4a_image_t k4aDepthImage = k4a_capture_get_depth_image(newCapture);

    if (k4aDepthImage == NULL || k4aColorImage == NULL) {
        return cv::Mat{};
    }

    k4a_calibration_t calibration;
    if (k4a_device_get_calibration(this->device, this->deviceConfig.depth_mode, this->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
        return cv::Mat{};
    }

    k4a_transformation_t transformationHandle = k4a_transformation_create(&calibration);

    k4a_image_t alignmentImage;

    if (k4a_image_create(K4A_IMAGE_FORMAT_COLOR_BGRA32,
        k4a_image_get_width_pixels(k4aDepthImage),
        k4a_image_get_height_pixels(k4aDepthImage),
        k4a_image_get_width_pixels(k4aDepthImage) * 4 * (int)sizeof(uint8_t),
        &alignmentImage) != K4A_RESULT_SUCCEEDED) {
            k4a_capture_release(newCapture);
            k4a_image_release(k4aColorImage);
            k4a_image_release(k4aDepthImage);
            k4a_transformation_destroy(transformationHandle);
            k4a_image_release(alignmentImage);
            return cv::Mat{};
    }

    if (k4a_transformation_color_image_to_depth_camera(transformationHandle, k4aDepthImage, k4aColorImage, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
        k4a_capture_release(newCapture);
        k4a_image_release(k4aColorImage);
        k4a_image_release(k4aDepthImage);
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        return cv::Mat{};
    }

    cv::Mat matColorImage = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_8UC4, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP).clone();

    k4a_capture_release(newCapture);
    k4a_image_release(k4aColorImage);
    k4a_image_release(k4aDepthImage);
    k4a_transformation_destroy(transformationHandle);
    k4a_image_release(alignmentImage);
    
    return matColorImage;
}

/*
* Takes a new capture and aligned the Depth onto Color
* @return cv::Mat(1080, 1920, 16UC1), empty cv::Mat if error
*/
cv::Mat DesktopApp::getRawDepthToColorImage() {

    k4a_capture_t newCapture;
    if (k4a_device_get_capture(this->device, &newCapture, K4A_WAIT_INFINITE) != K4A_RESULT_SUCCEEDED) {
        k4a_capture_release(newCapture);
        qDebug() << "Capture Failed";
        return cv::Mat{};
    }
    if (!newCapture) {
        k4a_capture_release(newCapture);
        qDebug() << "Capture Failed";
        return cv::Mat{};
    }
    k4a_image_t k4aColorImage = k4a_capture_get_color_image(newCapture);
    k4a_image_t k4aDepthImage = k4a_capture_get_depth_image(newCapture);
    if (k4aDepthImage == NULL || k4aColorImage == NULL) {
        qDebug() << "Capture NULL";
        return cv::Mat{};
    }

    k4a_calibration_t calibration;
    if (k4a_device_get_calibration(this->device, this->deviceConfig.depth_mode, this->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
        return cv::Mat{};
    }
    k4a_transformation_t transformationHandle = k4a_transformation_create(&calibration);

    k4a_image_t alignmentImage;
    if (k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, k4a_image_get_width_pixels(k4aColorImage), k4a_image_get_height_pixels(k4aColorImage), k4a_image_get_width_pixels(k4aColorImage) * (int)sizeof(uint16_t),
        &alignmentImage) != K4A_RESULT_SUCCEEDED) {
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        return cv::Mat{};
    }

    if (k4a_transformation_depth_image_to_color_camera(transformationHandle, k4aDepthImage, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        return cv::Mat{};
    }

    cv::Mat matDepthImage = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16U, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP).clone();

    k4a_capture_release(newCapture);
    k4a_image_release(k4aColorImage);
    k4a_image_release(k4aDepthImage);
    k4a_transformation_destroy(transformationHandle);
    k4a_image_release(alignmentImage);

    return matDepthImage;
}

#pragma endregion

cv::Mat DesktopApp::getCVDepthImage()
{
    k4a_image_t k4aDepthImage = this->depthImageQueue.back();
    cv::Mat cvEmptyImage;

    if (k4aDepthImage != NULL) {
        uint8_t* buffer = k4a_image_get_buffer(k4aDepthImage);
        int rows = k4a_image_get_height_pixels(k4aDepthImage);
        int cols = k4a_image_get_width_pixels(k4aDepthImage);

        cv::Mat matDepthImage(rows, cols, CV_16U, (void*)buffer, cv::Mat::AUTO_STEP);

        matDepthImage.convertTo(matDepthImage, CV_8U, 255.0 / 5000.0, 0.0);

        return matDepthImage;
    }

    return cvEmptyImage;
}

void colorizeDepth(const cv::Mat& gray, cv::Mat& rgb)
{
    double maxDisp = 255;
    float S = 1.f;
    float V = 1.f;

    rgb.create(gray.size(), CV_8UC3);
    rgb = cv::Scalar::all(0);

    if (maxDisp < 1)
        return;

    for (int y = 0; y < gray.rows; y++)
    {
        for (int x = 0; x < gray.cols; x++)
        {
            uchar d = gray.at<uchar>(y, x);
            
            if (d == 0) {
                rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(0.f, 0.f, 0.f);
                continue;
            }

            unsigned int H = 255 - ((uchar)maxDisp - d) * 280 / (uchar)maxDisp;
            unsigned int hi = (H / 60) % 6;

            float f = H / 60.f - H / 60;
            float p = V * (1 - S); // 0.f
            float q = V * (1 - f * S); // 1 - f
            float t = V * (1 - (1 - f) * S); // f

            cv::Point3f res;

            //qDebug() << d << " " << H << " " << hi << f;
            if (hi == 0) // R = V, G = t,  B = p
                res = cv::Point3f(p, t, V);
            if (hi == 1) // R = q, G = V,  B = p
                res = cv::Point3f(p, V, q);
            if (hi == 2) // R = p, G = V,  B = t
                res = cv::Point3f(t, V, p);
            if (hi == 3) // R = p, G = q,  B = V
                res = cv::Point3f(V, q, p);
            if (hi == 4) // R = t, G = p,  B = V
                res = cv::Point3f(V, p, t);
            if (hi == 5) // R = V, G = p,  B = q
                res = cv::Point3f(q, p, V);

            uchar b = (uchar)(std::max(0.f, std::min(res.x, 1.f)) * 255.f);
            uchar g = (uchar)(std::max(0.f, std::min(res.y, 1.f)) * 255.f);
            uchar r = (uchar)(std::max(0.f, std::min(res.z, 1.f)) * 255.f);

            rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(b, g, r);

        }
    }
}

QImage DesktopApp::getQIRImage() {
    k4a_image_t k4aIRImage = this->irImageQueue.back();

    double min, max;
    cv::Mat matIRImageRaw = cv::Mat(k4a_image_get_height_pixels(k4aIRImage), k4a_image_get_width_pixels(k4aIRImage), CV_16U, k4a_image_get_buffer(k4aIRImage), cv::Mat::AUTO_STEP);

    cv::minMaxIdx(matIRImageRaw, &min, &max);
    cv::Mat matIRImage;
    cv::convertScaleAbs(matIRImageRaw, matIRImage, 255 / max);

    cv::Mat temp;
    cvtColor(matIRImage, temp, cv::COLOR_GRAY2RGB);

    QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    qImage.bits();

    return qImage;
}

cv::Mat DesktopApp::getCVDepthToColorImage()
{
    k4a_image_t k4aDepthImage = this->depthImageQueue.back(), k4aColorImage = this->colorImageQueue.back();

    cv::Mat cvEmptyImage;

    if (k4aDepthImage != NULL && k4aColorImage != NULL) {
        k4a_calibration_t calibration;
        if (k4a_device_get_calibration(this->device, this->deviceConfig.depth_mode, this->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
            return cvEmptyImage;
        }

        k4a_transformation_t transformationHandle = k4a_transformation_create(&calibration);
        k4a_image_t alignmentImage;

        if (k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,
            k4a_image_get_width_pixels(k4aColorImage),
            k4a_image_get_height_pixels(k4aColorImage),
            k4a_image_get_width_pixels(k4aColorImage) * (int)sizeof(uint16_t),
            &alignmentImage) != K4A_RESULT_SUCCEEDED) {
            k4a_transformation_destroy(transformationHandle);
            k4a_image_release(alignmentImage);
            return cvEmptyImage;
        }

        if (k4a_transformation_depth_image_to_color_camera(transformationHandle, k4aDepthImage, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
            k4a_transformation_destroy(transformationHandle);
            k4a_image_release(alignmentImage);
            return cvEmptyImage;
        }

        double min, max;
        //cv::Mat matAlignmentImageRaw = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16U, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP);
        cv::Mat matDepthImage = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16U, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP);

        matDepthImage.convertTo(matDepthImage, CV_8U, 255.0 / 5000.0, 0.0);
        
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);

        return matDepthImage;

        //cv::minMaxIdx(matAlignmentImageRaw, &min, &max);
        //cv::Mat matAlignmentImage;
        //cv::convertScaleAbs(matAlignmentImageRaw, matAlignmentImage, 255 / max);

        //cv::Mat temp;
        //cv::applyColorMap(matAlignmentImage, temp, cv::COLORMAP_RAINBOW);

        //QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        //qImage.bits();

        //// Get point cloud alignment and copy as a member variable of captureTab (this->captureTab->k4aPointCloud)
        //if (this->alignk4APointCloud(&alignmentImage, this->captureTab->getK4aPointCloud()) != K4A_RESULT_SUCCEEDED) {
        //    qDebug() << "Failed to align point cloud";
        //}

        //// Get depth to color alignment image and copt as a member variable of captureTab (this->captureTab->k4aDepthToColor)
        //if (this->copyk4aImage(&alignmentImage, this->captureTab->getK4aDepthToColor()) != K4A_RESULT_SUCCEEDED) {
        //    qDebug() << "Failed to copy depth to color alignment image";
        //}

        //k4a_transformation_destroy(transformationHandle);
        //k4a_image_release(alignmentImage);
        //return qImage;
    }

    return cv::Mat();
}

k4a_result_t DesktopApp::copyk4aImage(k4a_image_t* src, k4a_image_t* target) {
    return k4a_image_create_from_buffer(
        k4a_image_get_format(*src),
        k4a_image_get_width_pixels(*src),
        k4a_image_get_height_pixels(*src),
        k4a_image_get_stride_bytes(*src),
        k4a_image_get_buffer(*src),
        k4a_image_get_size(*src),
        NULL,
        NULL,
        target
    );
}

k4a_result_t DesktopApp::alignk4APointCloud(k4a_image_t* k4aDepthImage, k4a_image_t* target) {
    if (this->captureTab->getCaptureCount() > 0) {
        k4a_image_release(*target);
    }

    this->captureTab->setCaptureCount(this->captureTab->getCaptureCount() + 1);

    if (k4a_image_create(
            K4A_IMAGE_FORMAT_CUSTOM,
            k4a_image_get_width_pixels(*k4aDepthImage),
            k4a_image_get_height_pixels(*k4aDepthImage),
            6 * k4a_image_get_width_pixels(*k4aDepthImage),
            target
        ) != K4A_RESULT_SUCCEEDED) return K4A_RESULT_FAILED;

    k4a_calibration_t calibration;
    if (k4a_device_get_calibration(this->device, this->deviceConfig.depth_mode, this->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
        return K4A_RESULT_FAILED;
    }

    k4a_transformation_t transformationHandle = k4a_transformation_create(&calibration);

    k4a_result_t output =
        k4a_transformation_depth_image_to_point_cloud(
            transformationHandle,
            *k4aDepthImage,
            K4A_CALIBRATION_TYPE_COLOR,
            *target
        );

    k4a_transformation_destroy(transformationHandle);

    return output;
}
