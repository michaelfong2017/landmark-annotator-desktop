#include "alignmenttab.h"

AlignmentTab::AlignmentTab(DesktopApp* parent) {
    this->parent = parent;

    QObject::connect(this->parent->ui.loadColorButton, &QPushButton::clicked, [this]() {
        QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Load Color Video stream"), visitFolderPath, tr("MP4 (*.mp4)"));

        this->colorInputFilename = fileName;
        this->colorVideoCapture = new cv::VideoCapture(fileName.toStdString());

        this->parent->ui.colorFilename->setText(fileName);
    });

    QObject::connect(this->parent->ui.loadDepthButton, &QPushButton::clicked, [this]() {
        QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Load Depth Video stream"), visitFolderPath, tr("MP4 (*.mp4)"));

        this->depthInputFilename = fileName;
        this->depthVideoCapture = new cv::VideoCapture(fileName.toStdString());
        this->parent->ui.depthFilename->setText(fileName);
    });

    QObject::connect(this->parent->ui.alignButton, &QPushButton::clicked, [this]() {
        // TODO: Fix validation later
        if (!this->colorVideoCapture->isOpened() || !this->depthVideoCapture->isOpened()) {
            this->parent->ui.messageText->setText("Invalid input video streams.");
            return;
        }

        cv::Size colorSize(
            1280,
            720 
        );

        this->depthToColorVideoWriter = new cv::VideoWriter(
            "C:/Users/Edward/Desktop/test.mp4",
            cv::VideoWriter::fourcc('H', '2', '6', '4'),
            14,
            colorSize
        );

        int i = 0;
        while (1) {
            i++;
            cv::Mat colorFrame, depthFrame, colorTemp, depthTemp;

            *(this->colorVideoCapture) >> colorTemp;
            *(this->depthVideoCapture) >> depthTemp;

            if (colorTemp.empty() || depthTemp.empty())
                break;

            cvtColor(colorTemp, colorFrame, cv::COLOR_RGB2RGBA);
            cvtColor(depthTemp, depthFrame, cv::COLOR_RGB2GRAY);

            depthFrame.convertTo(depthFrame, CV_16U, 5000.0 / 255.0, 0.0);

            k4a::image color = k4a::image::create_from_buffer(
                K4A_IMAGE_FORMAT_COLOR_BGRA32,
                colorFrame.cols,
                colorFrame.rows,
                (int)colorFrame.step,
                colorFrame.data,
                colorFrame.step * colorFrame.rows,
                nullptr,
                nullptr
            );
            k4a::image depth = k4a::image::create_from_buffer(
                K4A_IMAGE_FORMAT_DEPTH16,
                depthFrame.cols,
                depthFrame.rows,
                depthFrame.step, 
                depthFrame.data,
                depthFrame.step * depthFrame.rows,
                nullptr,
                nullptr
            );

            k4a_calibration_t calibration;
            if (k4a_device_get_calibration(this->parent->device, this->parent->deviceConfig.depth_mode, this->parent->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
                break;
            }

            k4a_transformation_t transformationHandle = k4a_transformation_create(&calibration);
            k4a_image_t alignmentImage;

            if (k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,
                color.get_width_pixels(),
                color.get_height_pixels(),
                color.get_width_pixels() * (int)sizeof(uint16_t),
                &alignmentImage) != K4A_RESULT_SUCCEEDED) {
                k4a_transformation_destroy(transformationHandle);
                k4a_image_release(alignmentImage);
                this->parent->ui.messageText->setText("creation failed");
                break;
            }

            if (k4a_transformation_depth_image_to_color_camera(transformationHandle, depth.handle(), alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
                k4a_transformation_destroy(transformationHandle);
                this->parent->ui.messageText->setText("transformation failed");
                break;
            }

            double min, max;
            cv::Mat matAlignmentImageRaw = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16U, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP);

            cv::minMaxIdx(matAlignmentImageRaw, &min, &max);
            cv::Mat matAlignmentImage;
            cv::convertScaleAbs(matAlignmentImageRaw, matAlignmentImage, 255 / max);

            cv::Mat temp;
            cv::applyColorMap(matAlignmentImage, temp, cv::COLORMAP_RAINBOW);

            *(this->depthToColorVideoWriter) << temp;

            // Release memory
            colorTemp.release();
            depthTemp.release();

            color.reset();
            depth.reset();

            colorFrame.release();
            depthFrame.release();

            k4a_image_release(alignmentImage);
            matAlignmentImageRaw.release();
            matAlignmentImage.release();
            temp.release();

            k4a_transformation_destroy(transformationHandle);
        }

        // Reset everything
        this->colorInputFilename = QString();
        this->depthInputFilename = QString();
        this->colorVideoCapture->release();
        this->depthVideoCapture->release();
        this->depthToColorVideoWriter->release();
        this->parent->ui.colorFilename->setText("");
        this->parent->ui.depthFilename->setText("");

    });
}
