#include "capturetab.h"

CaptureTab::CaptureTab(DesktopApp* parent)
{
    this->parent = parent;
    this->recorder = new Recorder(parent);
    this->parent->ui.recordingIndicatorText->setVisible(false);

    this->setDefaultCaptureMode();

    this->registerRadioButtonOnClicked(this->parent->ui.radioButton, &this->colorImage);
    this->registerRadioButtonOnClicked(this->parent->ui.radioButton2, &this->depthImage);
    this->registerRadioButtonOnClicked(this->parent->ui.radioButton3, &this->colorToDepthImage);
    this->registerRadioButtonOnClicked(this->parent->ui.radioButton4, &this->depthToColorImage);

    this->captureCount = 0;
    this->timer = new QTimer;

    this->parent->ui.showInExplorer->hide();
    this->captureFilepath = QString();

    QObject::connect(this->parent->ui.saveButtonCaptureTab, &QPushButton::clicked, [this]() {
        QString dateTimeString = Helper::getCurrentDateTimeString();
        QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
        QString colorSavePath = visitFolderPath + "/color_" + dateTimeString + ".png";
        QString depthToColorSavePath = visitFolderPath + "/rgbd_" + dateTimeString + ".png";
        QString depthSavePath = visitFolderPath + "/depth_" + dateTimeString + ".png";
        QString colorToDepthSavePath = visitFolderPath + "/color_aligned_" + dateTimeString + ".png";

        QImageWriter colorWriter(colorSavePath);
        QImageWriter depthToColorWriter(depthToColorSavePath);
        QImageWriter depthWriter(depthSavePath);
        QImageWriter colorToDepthWriter(colorToDepthSavePath);

        if (
            !colorWriter.write(this->colorImage) | 
            !depthToColorWriter.write(this->depthToColorImage) | 
            !depthWriter.write(this->depthImage) | 
            !colorToDepthWriter.write(this->colorToDepthImage)
            ) {
            qDebug() << colorWriter.errorString();
            qDebug() << depthToColorWriter.errorString();
            qDebug() << depthWriter.errorString();
            qDebug() << depthToColorWriter.errorString();
            this->parent->ui.saveInfoCaptureTab->setText("Something went wrong, cannot save images.");

            this->parent->ui.showInExplorer->hide();
            return;
        }

        this->parent->ui.saveInfoCaptureTab->setText("Images saved under " + visitFolderPath + "\n at " + dateTimeString);

        this->parent->ui.showInExplorer->show();
        this->setCaptureFilepath(colorSavePath);
    });

    QObject::connect(this->parent->ui.saveVideoButton, &QPushButton::clicked, [this]() {
        if (this->recorder->getRecordingStatus()) {
            // Current status is recording
            QString dateTimeString = Helper::getCurrentDateTimeString();
            QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);

            // Modify UI to disable recording status
            this->parent->ui.recordingIndicatorText->setVisible(false);
            this->parent->ui.captureTab->setStyleSheet("");

            this->recorder->stopRecorder();
            this->parent->ui.saveVideoButton->setText("start recording");

            this->parent->ui.saveInfoCaptureTab->setText("Recording is saved under " + visitFolderPath + "\nat " + dateTimeString);

            this->parent->ui.showInExplorer->show();
            this->setCaptureFilepath(this->recorder->getColorOutputFilename());
        }
        else {
            // Current status is NOT recording
 
            // Modify UI to indicate recording status
            this->parent->ui.recordingIndicatorText->setVisible(true);
            this->parent->ui.captureTab->setStyleSheet("#captureTab {border: 2px solid red}");

            this->recorder->prepareRecorder();
            this->parent->ui.saveVideoButton->setText("stop recording");

            this->recorder->timer->start(1000);
        }
    });

    /** Michael Fong Show In Explorer
    BEGIN */
    QObject::connect(this->parent->ui.showInExplorer, &QPushButton::clicked, [this]() {
        QString filepath = this->getCaptureFilepath();

        QStringList args;

        args << "/select," << QDir::toNativeSeparators(filepath);

        QProcess* process = new QProcess(this);
        process->startDetached("explorer.exe", args);
    });
    /** Michael Fong Show In Explorer
    END */

    QObject::connect(this->parent->ui.captureButton, &QPushButton::clicked, [this]() {
        this->colorImage = this->parent->getQColorImage();
        this->depthImage = this->parent->getQDepthImage();
        this->colorToDepthImage = this->parent->getQColorToDepthImage();
        this->depthToColorImage = this->parent->getQDepthToColorImage();

        QImage image;

        if (this->parent->ui.radioButton->isChecked()) {
            image = this->colorImage;
        }
        else if (this->parent->ui.radioButton2->isChecked()) {
            image = this->depthImage;
        }
        else if (this->parent->ui.radioButton3->isChecked()) {
            image = this->colorToDepthImage;
        }
        else {
            image = this->depthToColorImage;
        }

        int width = this->parent->ui.graphicsViewImage->width(), height = this->parent->ui.graphicsViewImage->height();

        QImage imageScaled = image.scaled(width, height, Qt::KeepAspectRatio);

        // Deallocate heap memory used by previous GGraphicsScene object
        if (this->parent->ui.graphicsViewImage->scene()) {
            delete this->parent->ui.graphicsViewImage->scene();
        }

        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
        QGraphicsScene* scene = new QGraphicsScene;
        scene->addItem(item);

        this->parent->ui.graphicsViewImage->setScene(scene);
        this->parent->ui.graphicsViewImage->show();
    });

    QObject::connect(this->parent->ui.annotateButtonCaptureTab, &QPushButton::clicked, [this]() {
        // Move to annotate tab whose index is 3
        this->parent->annotateTab->reloadCurrentImage();
        this->parent->ui.tabWidget->setCurrentIndex(3);
        this->parent->ui.annotateButtonAnnotateTab->click();
    });

    QObject::connect(timer, &QTimer::timeout, [this]() {
        if (this->parent->deviceCount > 0) {
            switch (k4a_device_get_capture(this->parent->device, &this->parent->capture, K4A_WAIT_INFINITE)) {
                case K4A_WAIT_RESULT_SUCCEEDED:
                    break;
            }

            if (this->parent->capture) {
                k4a_image_t k4aColorImage = k4a_capture_get_color_image(this->parent->capture);

                if (k4aColorImage != NULL) {
                    this->parent->colorImageQueue.push(k4aColorImage);

                    int width = this->parent->ui.graphicsViewVideo4->width(), height = this->parent->ui.graphicsViewVideo4->height();
                    QImage qColorImage = (this->parent->getQColorImage()).scaled(width, height, Qt::KeepAspectRatio);

                    // Deallocate heap memory used by previous GGraphicsScene object
                    if (this->parent->ui.graphicsViewVideo4->scene()) {
                        delete this->parent->ui.graphicsViewVideo4->scene();
                    }
                    
                    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qColorImage));
                    QGraphicsScene* scene = new QGraphicsScene;
                    scene->addItem(item);

                    this->parent->ui.graphicsViewVideo4->setScene(scene);
                    this->parent->ui.graphicsViewVideo4->show();

                    while (this->parent->colorImageQueue.size() > MAX_IMAGE_QUEUE_SIZE) {
                        k4a_image_release(this->parent->colorImageQueue.front());
                        this->parent->colorImageQueue.pop();
                    }
                }

                k4a_image_t k4aDepthImage = k4a_capture_get_depth_image(this->parent->capture);

                if (k4aDepthImage != NULL) {
                    this->parent->depthImageQueue.push(k4aDepthImage);

                    int width = this->parent->ui.graphicsViewVideo5->width(), height = this->parent->ui.graphicsViewVideo5->height();
                    QImage qDepthImage = (this->parent->getQDepthImage()).scaled(width, height, Qt::KeepAspectRatio);

                    // Deallocate heap memory used by previous GGraphicsScene object
                    if (this->parent->ui.graphicsViewVideo5->scene()) {
                        delete this->parent->ui.graphicsViewVideo5->scene();
                    }

                    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qDepthImage));
                    QGraphicsScene* scene = new QGraphicsScene;
                    scene->addItem(item);

                    this->parent->ui.graphicsViewVideo5->setScene(scene);
                    this->parent->ui.graphicsViewVideo5->show();

                    while (this->parent->depthImageQueue.size() > MAX_IMAGE_QUEUE_SIZE) {
                        k4a_image_release(this->parent->depthImageQueue.front());
                        this->parent->depthImageQueue.pop();
                    }
                }

                k4a_capture_release(this->parent->capture);
            }
            else {
                qDebug() << "No capture found\n";
            }

            // Capture a imu sample
            switch (k4a_device_get_imu_sample(this->parent->device, &this->parent->imuSample, K4A_WAIT_INFINITE)) {
                case K4A_WAIT_RESULT_SUCCEEDED:
                    break;
            }

            if (&this->parent->imuSample != NULL) {
                this->parent->gyroSampleQueue.push_back(this->parent->imuSample.gyro_sample);
                this->parent->accSampleQueue.push_back(this->parent->imuSample.acc_sample);

                QString text;
                text += ("Temperature: " + QString::number(this->parent->imuSample.temperature, 0, 2) + " C\n");
                this->parent->ui.imuText->setText(text);
            }

            while (this->parent->gyroSampleQueue.size() > MAX_GYROSCOPE_QUEUE_SIZE) this->parent->gyroSampleQueue.pop_front();

            while (this->parent->accSampleQueue.size() > MAX_ACCELEROMETER_QUEUE_SIZE) this->parent->accSampleQueue.pop_front();

            if (this->parent->gyroSampleQueue.size() >= MAX_GYROSCOPE_QUEUE_SIZE) this->drawGyroscopeData();

            if (this->parent->accSampleQueue.size() >= MAX_ACCELEROMETER_QUEUE_SIZE) this->drawAccelerometerData();
        }
    });
}

void CaptureTab::setDefaultCaptureMode() {
    parent->ui.radioButton->setChecked(true);
    parent->ui.radioButton2->setChecked(false);
    parent->ui.radioButton3->setChecked(false);
    parent->ui.radioButton4->setChecked(false);
}

void CaptureTab::registerRadioButtonOnClicked(QRadioButton* radioButton, QImage* image) {
    QObject::connect(radioButton, &QRadioButton::clicked, [this, image]() {
        int width = this->parent->ui.graphicsViewImage->width(), height = this->parent->ui.graphicsViewImage->height();

        QImage imageScaled = (*image).scaled(width, height, Qt::KeepAspectRatio);

        // Deallocate heap memory used by previous GGraphicsScene object
        if (this->parent->ui.graphicsViewImage->scene()) {
            delete this->parent->ui.graphicsViewImage->scene();
        }

        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
        QGraphicsScene* scene = new QGraphicsScene;
        scene->addItem(item);

        this->parent->ui.graphicsViewImage->setScene(scene);
        this->parent->ui.graphicsViewImage->show();
    });
}

QImage CaptureTab::getQCapturedColorImage() {
    return this->colorImage;
}

QImage CaptureTab::getQCapturedDepthToColorImage() {
    return this->depthToColorImage;
}

void CaptureTab::drawGyroscopeData() {
    // Deallocate heap memory used by previous GGraphicsScene object
    if (this->parent->ui.graphicsViewGyroscope->scene()) {
        delete this->parent->ui.graphicsViewGyroscope->scene();
    }

    int width = this->parent->ui.graphicsViewGyroscope->width(), height = this->parent->ui.graphicsViewGyroscope->height();
    QGraphicsScene* scene = new QGraphicsScene();
    QImage image = QPixmap(0.95 * width, 0.95 * height).toImage();

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));

    for (int i = 0; i < MAX_GYROSCOPE_QUEUE_SIZE - 1; ++i) {
        int segmentLength = width / MAX_GYROSCOPE_QUEUE_SIZE;
        int segmentHeight = height / 3;

        // Draw  gyroscope measurement w.r.t x-axis
        int leftSegmentHeight = 2 * this->parent->gyroSampleQueue[i].xyz.x;
        int rightSegmentHeight = 2 * this->parent->gyroSampleQueue[i + 1].xyz.x;
        painter.drawLine(i * segmentLength, segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, segmentHeight / 2 + rightSegmentHeight);

        // Draw  gyroscope measurement w.r.t y-axis
        leftSegmentHeight = 2 * this->parent->gyroSampleQueue[i].xyz.y;
        rightSegmentHeight = 2 * this->parent->gyroSampleQueue[i + 1].xyz.y;
        painter.drawLine(i * segmentLength, 3 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 3 * segmentHeight / 2 + rightSegmentHeight);

        // Draw  gyroscope measurement w.r.t z-axis
        leftSegmentHeight = 2 * this->parent->gyroSampleQueue[i].xyz.z;
        rightSegmentHeight = 2 * this->parent->gyroSampleQueue[i + 1].xyz.z;
        painter.drawLine(i * segmentLength, 5 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 5 * segmentHeight / 2 + rightSegmentHeight);
    }

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene->addItem(item);

    this->parent->ui.graphicsViewGyroscope->setScene(scene);
}

void CaptureTab::drawAccelerometerData() {
    // Deallocate heap memory used by previous GGraphicsScene object
    if (this->parent->ui.graphicsViewAccelerometer->scene()) {
        delete this->parent->ui.graphicsViewAccelerometer->scene();
    }

    int width = this->parent->ui.graphicsViewAccelerometer->width(), height = this->parent->ui.graphicsViewAccelerometer->height();
    QGraphicsScene* scene = new QGraphicsScene();
    QImage image = QPixmap(0.95 * width, 0.95 * height).toImage();

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));

    for (int i = 0; i < MAX_ACCELEROMETER_QUEUE_SIZE - 1; ++i) {
        int segmentLength = width / MAX_ACCELEROMETER_QUEUE_SIZE;
        int segmentHeight = height / 3;

        // Draw  gyroscope measurement w.r.t x-axis
        int leftSegmentHeight = 2 * this->parent->accSampleQueue[i].xyz.x;
        int rightSegmentHeight = 2 * this->parent->accSampleQueue[i + 1].xyz.x;
        painter.drawLine(i * segmentLength, segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, segmentHeight / 2 + rightSegmentHeight);

        // Draw  gyroscope measurement w.r.t y-axis
        leftSegmentHeight = 2 * this->parent->accSampleQueue[i].xyz.y;
        rightSegmentHeight = 2 * this->parent->accSampleQueue[i + 1].xyz.y;
        painter.drawLine(i * segmentLength, 3 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 3 * segmentHeight / 2 + rightSegmentHeight);

        // Draw  gyroscope measurement w.r.t z-axis
        leftSegmentHeight = 2 * this->parent->accSampleQueue[i].xyz.z;
        rightSegmentHeight = 2 * this->parent->accSampleQueue[i + 1].xyz.z;
        painter.drawLine(i * segmentLength, 5 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 5 * segmentHeight / 2 + rightSegmentHeight);
    }

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene->addItem(item);

    this->parent->ui.graphicsViewAccelerometer->setScene(scene);
}

k4a_image_t* CaptureTab::getK4aPointCloud() {
    return &(this->k4aPointCloud);
}

k4a_image_t* CaptureTab::getK4aDepthToColor() {
    return &(this->k4aDepthToColor);
}

QVector3D CaptureTab::query3DPoint(int x, int y) {
    int width = k4a_image_get_width_pixels(*this->getK4aPointCloud());
    int height = k4a_image_get_height_pixels(*this->getK4aPointCloud());

    bool* visited = (bool*) malloc((width * height) * sizeof(bool));
    std::queue<std::pair<int, int>> coordQueue;

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) 
            visited[(width * i) + j] = false;
    }

    visited[(width * y) + x] = true;
    coordQueue.push(std::make_pair(x, y));
    int index;
    int16_t xOut, yOut, zOut;


    while (!coordQueue.empty()) {
        std::pair<int, int> coord = coordQueue.front();
        coordQueue.pop();

        index = 3 * ((width * coord.second) + coord.first);

        xOut = (int16_t) k4a_image_get_buffer(*this->getK4aPointCloud())[index];
        yOut = (int16_t) k4a_image_get_buffer(*this->getK4aPointCloud())[++index];
        zOut = (int16_t) k4a_image_get_buffer(*this->getK4aPointCloud())[++index];

        if (!(xOut == 0 && yOut == 0 && zOut == 0)) {
            free(visited);
            return QVector3D(xOut, yOut, zOut);
        }

        for (int i = coord.second - 1; i <= coord.second + 1; ++i) {
            for (int j = coord.first - 1; j <= coord.first + 1; ++j) {
                if (i < 0 || i >= height) continue;
                if (j < 0 || j >= width) continue;

                if (!visited[(width * i) + j]) {
                    coordQueue.push(std::make_pair(j, i));
                    visited[(width * i) + j] = true;
                }
            }
        }
    }

    free(visited);
    return QVector3D(0, 0, 0);
}

int CaptureTab::getCaptureCount() { return this->captureCount; }

void CaptureTab::setCaptureCount(int newCaptureCount) { this->captureCount = newCaptureCount; }

Recorder* CaptureTab::getRecorder() { return this->recorder;  }

QString CaptureTab::getCaptureFilepath() { return this->captureFilepath; }
void CaptureTab::setCaptureFilepath(QString captureFilepath) { this->captureFilepath = captureFilepath; }
