#include "patienttab.h"
#include "capturetab.h"
#include "realsenseengine.h"

PatientTab::PatientTab(DesktopApp* parent)
{
    this->parent = parent;

    /** Progress bar UI */
    this->parent->ui.progressBar_2->setValue(0);
    this->parent->ui.progressBar_2->setTextVisible(false);
    /** Progress bar UI END */

    tableView = this->parent->ui.patientTab->findChild<QTableView*>("tableViewPatient");
    patientDataModel = new QStandardItemModel(0, 4, this);
    tableView->setModel(this->patientDataModel);

    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    /** Open image url */
    connect(tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onTableClicked(const QModelIndex &)));
    /** Open image url END */

    QObject::connect(this->parent->ui.patientTab->findChild<QPushButton*>("captureNewButton"), &QPushButton::clicked, [this]() {
        qDebug() << "captureNewButton clicked";


        if (!RealsenseEngine::getInstance().isDeviceConnected()) {
            TwoLinesDialog dialog;
            dialog.setLine1("Kinect device cannot be opened!");
            dialog.setLine2("Please check it and try again.");
            dialog.exec();
            return;
        }

        if (!RealsenseEngine::getInstance().isDeviceOpened()) {
            RealsenseEngine::getInstance().configDevice();
            bool isSuccess = RealsenseEngine::getInstance().openDevice();

            if (!isSuccess) {
                TwoLinesDialog dialog;
                dialog.setLine1("Kinect device cannot be opened!");
                dialog.setLine2("Please check it and try again.");
                dialog.exec();
                return;
            }
        }


        //if (!KinectEngine::getInstance().isDeviceConnected()) {
        //    TwoLinesDialog dialog;
        //    dialog.setLine1("Kinect device cannot be opened!");
        //    dialog.setLine2("Please check it and try again.");
        //    dialog.exec();
        //    return;
        //}

        //if (!KinectEngine::getInstance().isDeviceOpened()) {
        //    KinectEngine::getInstance().configDevice();
        //    bool isSuccess = KinectEngine::getInstance().openDevice();

        //    if (!isSuccess) {
        //        TwoLinesDialog dialog;
        //        dialog.setLine1("Kinect device cannot be opened!");
        //        dialog.setLine2("Please check it and try again.");
        //        dialog.exec();
        //        return;
        //    }
        //}

        // need to clear list
        this->parent->captureTab->clearCaptureHistories();

        this->parent->ui.tabWidget->setTabEnabled(TabIndex::CAPTURETAB, true);
        this->parent->ui.tabWidget->setTabEnabled(TabIndex::ANNOTATETAB, true);
        this->parent->ui.tabWidget->setCurrentIndex(TabIndex::CAPTURETAB);
        });
}

DesktopApp* PatientTab::getParent()
{
    return this->parent;
}

void PatientTab::onEnterTab() {
    tableView->hide();
    QNetworkClient::getInstance().fetchExistingImagesOfPatient(currentPatientId, this, SLOT(onFetchExistingImagesOfPatient(QNetworkReply*)));

    this->parent->ui.patientTab->findChild<QLabel*>("patientName")->setText(tr("Name: ") + name);
    this->parent->ui.patientTab->findChild<QLabel*>("patientAge")->setText(tr("Age: ") + age);
    this->parent->ui.patientTab->findChild<QLabel*>("patientSubjectNumber")->setText(tr("Subject Number: ") + subjectNumber);
}


void PatientTab::setCurrentPatientId(int currentPatientId) {
    this->currentPatientId = currentPatientId;
}

int PatientTab::getCurrentPatientId() {
    return this->currentPatientId;
}

void PatientTab::setCurrentPatientName(QString currentPatientName) {
    this->currentPatientName = currentPatientName;
}

QString PatientTab::getCurrentPatientName() {
    return this->currentPatientName;
}

void PatientTab::setName(QString name)
{
    this->name = name;
}

void PatientTab::setSex(QString sex)
{
    this->sex = sex;
}

QString PatientTab::getSex()
{
    return this->sex;
}

void PatientTab::setAge(QString age)
{
    this->age = age;
}

QString PatientTab::getAge()
{
    return this->age;
}

void PatientTab::setDOB(QString dob)
{
    this->dob = dob;
}

QString PatientTab::getDOB()
{
    return this->dob;
}

void PatientTab::setPhoneNumber(QString phoneNumber)
{
    this->phoneNumber = phoneNumber;
}

QString PatientTab::getPhoneNumber()
{
    return this->phoneNumber;
}

void PatientTab::setSubjectNumber(QString subjectNumber)
{
    this->subjectNumber = subjectNumber;
}

void PatientTab::setIdCard(QString idCard)
{
    this->idCard = idCard;
}

void PatientTab::setSin(QString sin)
{
    this->sin = sin;
}

void PatientTab::setEmail(QString email)
{
    this->email = email;
}

void PatientTab::setAddress(QString address)
{
    this->address = address;
}

void PatientTab::setRemark(QString remark)
{
    this->remark = remark;
}

void PatientTab::setWeight(QString weight)
{
    this->weight = weight;
}

QString PatientTab::getWeight()
{
    return this->weight;
}

void PatientTab::setHeight(QString height)
{
    this->height = height;
}

QString PatientTab::getHeight()
{
    return this->height;
}

void PatientTab::onLanguageChanged()
{
    setHeaders();

    this->parent->ui.patientTab->findChild<QLabel*>("patientName")->setText(tr("Name: ") + name);
    this->parent->ui.patientTab->findChild<QLabel*>("patientAge")->setText(tr("Age: ") + age);
    this->parent->ui.patientTab->findChild<QLabel*>("patientSubjectNumber")->setText(tr("Subject Number: ") + subjectNumber);
}

void PatientTab::onFetchExistingImagesOfPatient(QNetworkReply* reply) {
    // Clear data and re-fetch all data every time
    patientDataModel->clear();

    /** Headers */
    setHeaders();

    /** This must be put here (below) */
    for (int col = 0; col < 4; col++)
    {
        tableView->setColumnWidth(col, 400);
    }
    /** This must be put here (below) END */
    /** Headers END */

    tableView->show();

    QByteArray response_data = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

    //qDebug() << jsonResponse;

    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["items"].toArray();

    foreach(const QJsonValue & value, jsonArray) {
        QJsonObject obj = value.toObject();

        QList<QStandardItem*> itemList;
        QStandardItem* item;
        for (int i = 0; i < 4; i++)
        {
            if (i == 0) {
                QString text;

                text = obj["takeDate"].toString();
                text = Helper::convertFetchedDateTime(text);

                item = new QStandardItem(text);
                QFont fn = item->font();
                fn.setPixelSize(14);

                item->setFont(fn);
                item->setTextAlignment(Qt::AlignCenter);
            }
            else if (i == 1) {
                QString text;

                text = tr("View");

                item = new QStandardItem(text);
                QFont fn = item->font();
                fn.setPixelSize(14);

                fn.setUnderline(true);

                item->setFont(fn);

                QBrush brush;
                QColor blue(Qt::blue);
                brush.setColor(blue);
                item->setForeground(brush);
                item->setTextAlignment(Qt::AlignCenter);
            }
            else if (i == 2) {
                QString text;

                text = obj["url"].toString();

                item = new QStandardItem(text);
                QFont fn = item->font();
                fn.setPixelSize(14);

                fn.setUnderline(true);

                item->setFont(fn);

                QBrush brush;
                QColor blue(Qt::blue);
                brush.setColor(blue);
                item->setForeground(brush);
            }
            else if (i == 3) {
                QString text;

                text = obj["aiOriginResult"].toString();

                item = new QStandardItem(text);
                QFont fn = item->font();
                fn.setPixelSize(14);

                item->setFont(fn);
            }

            itemList << item;
        }

        patientDataModel->insertRow(patientDataModel->rowCount(), itemList);

        QModelIndex curIndex = patientDataModel->index(patientDataModel->rowCount() - 1, 0);
    }
    
    tableView->hideColumn(2);
    tableView->hideColumn(3);

    reply->deleteLater();
}

void PatientTab::setHeaders()
{
    QStringList headerLabels = { tr("Captured Date"), tr("Analysis"), tr("Url"), tr("Landmarks") };

    for (int i = 0; i < 4; i++)
    {
        QString text = headerLabels.at(i);
        QStandardItem* item = new QStandardItem(text);
        QFont fn = item->font();
        fn.setPixelSize(14);
        item->setFont(fn);
        item->setTextAlignment(Qt::AlignCenter);

        patientDataModel->setHorizontalHeaderItem(i, item);
    }
}

void PatientTab::onTableClicked(const QModelIndex& index)
{
    if (index.isValid() && index.column() == 1 && !isDownloading) {

        /** Progress bar UI */
        this->parent->ui.progressBar_2->setTextVisible(true);
        this->parent->ui.progressBar_2->setValue(50);
        /** Progress bar UI END */

        int row = tableView->currentIndex().row();

        QModelIndex curIndex = patientDataModel->index(row, 2);
        QString url = patientDataModel->data(curIndex).toString();

        curIndex = patientDataModel->index(row, 3);
        landmarkS = patientDataModel->data(curIndex).toString();

        qDebug() << "url " << url;

        //QDesktopServices::openUrl(QUrl(url));
        isDownloading = true;

        QNetworkClient::getInstance().downloadImage(url, this, SLOT(onDownloadImage(QNetworkReply*)));
    }
}

void PatientTab::onDownloadImage(QNetworkReply* reply) {

    qDebug() << "landmarkString" << landmarkS;

    QByteArray imageData = reply->readAll();

    QImage image = QImage::fromData(imageData);

    qDebug() << "onDownloadImage() image format: " << image.format();

    /*QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
    QString savePath = QDir(visitFolderPath).filePath(QString::fromStdString("d2.png"));*/

    /*image.save(savePath);
    cv::Mat FourChannelPNG = cv::imread(savePath.toStdString(), -1);
    qDebug() << "savePath" << savePath;*/

    if (image.format() != QImage::Format_RGBA64) {
        qDebug() << "Error: image format incorrect " << image.format();
        TwoLinesDialog dialog;
        dialog.setLine1("Error: Image format incorrect");
        dialog.exec();
        isDownloading = false;
        /** Progress bar UI */
        this->parent->ui.progressBar_2->setTextVisible(false);
        this->parent->ui.progressBar_2->setValue(0);
        /** Progress bar UI END */
        return;
    }

    // 800x1080 for Kinect, 534x720 for Realsense
    cv::Mat FourChannelPNG = cv::Mat(image.height(), image.width(), CV_16UC4, image.bits(), image.bytesPerLine());

    int height = FourChannelPNG.rows;
    int width = FourChannelPNG.cols;
    qDebug() << "dimension(h, w, c): " << height << ", " << width << ", " << FourChannelPNG.channels();

    std::vector<cv::Mat>channels(4);
    cv::split(FourChannelPNG, channels);

    /** ColorIMG */
    cv::Mat ColorIMG = cv::Mat::zeros(height, width, CV_8UC3);
    std::vector<cv::Mat>colorIMGChannels(3);
    colorIMGChannels[0] = cv::Mat::zeros(height, width, CV_8UC1);
    colorIMGChannels[1] = cv::Mat::zeros(height, width, CV_8UC1);
    colorIMGChannels[2] = cv::Mat::zeros(height, width, CV_8UC1);

    for (int i = 0; i < width * height; i++) {
        colorIMGChannels[0].at<uint8_t>(i) = channels[1].at<uint16_t>(i) >> 8;
        colorIMGChannels[1].at<uint8_t>(i) = channels[0].at<uint16_t>(i) % 256;
        colorIMGChannels[2].at<uint8_t>(i) = channels[0].at<uint16_t>(i) >> 8;
    }

    cv::merge(colorIMGChannels, ColorIMG);

    //cv::imshow("RGB", ColorIMG);
    /** ColorIMG END */

    /** Depth 1 */
    cv::Mat AlignedDepthIMG1 = cv::Mat::zeros(height, width, CV_16UC1);
    std::vector<cv::Mat>alignedDepthIMG1Channels(1);
    alignedDepthIMG1Channels[0] = cv::Mat::zeros(height, width, CV_16UC1);

    for (int i = 0; i < width * height; i++) {
        alignedDepthIMG1Channels[0].at<uint16_t>(i) = channels[2].at<uint16_t>(i);
    }

    cv::merge(alignedDepthIMG1Channels, AlignedDepthIMG1);

    // Convert from 16 bit to 8 bit for display
    cv::Mat AlignedDepthIMG1_8bit;
    AlignedDepthIMG1.convertTo(AlignedDepthIMG1_8bit, CV_8U, 255.0 / 5000.0, 0.0);
    // Convert from 16 bit to 8 bit for display END

    //cv::imshow("Depth1_8bit", AlignedDepthIMG1_8bit);
    /** Depth 1 END */


     /** Depth 2 */
    cv::Mat AlignedDepthIMG2 = cv::Mat::zeros(height, width, CV_16UC1);
    std::vector<cv::Mat>alignedDepthIMG2Channels(1);
    alignedDepthIMG2Channels[0] = cv::Mat::zeros(height, width, CV_16UC1);

    for (int i = 0; i < width * height; i++) {
        alignedDepthIMG2Channels[0].at<uint16_t>(i) = channels[3].at<uint16_t>(i);
    }

    cv::merge(alignedDepthIMG2Channels, AlignedDepthIMG2);

    // Convert from 16 bit to 8 bit for display
    cv::Mat AlignedDepthIMG2_8bit;
    AlignedDepthIMG2.convertTo(AlignedDepthIMG2_8bit, CV_8U, 255.0 / 5000.0, 0.0);
    // Convert from 16 bit to 8 bit for display END

    //cv::imshow("Depth2_8bit", AlignedDepthIMG2_8bit);
    /** Depth 2 END */

    /** Convert CV image to QImage */
    QImage qColorImage = convertColorCVToQImage(ColorIMG);
    //QImage qDepthImage1 = convertDepthToColorCVToColorizedQImageDetailed(AlignedDepthIMG1); // 16 bit
    //QImage qDepthImage2 = convertDepthToColorCVToColorizedQImageDetailed(AlignedDepthIMG2); // 16 bit
    /** Convert CV image to QImage END */


    QString aiOriginResult = landmarkS;
    if (aiOriginResult == "") {
        int w = AlignedDepthIMG1.cols;
        int h = AlignedDepthIMG1.rows;
        std::string PtC = "[" + std::to_string(w / 2) + ", " + std::to_string(h / 5) + "], ";
        std::string PtA2 = "[" + std::to_string(2 * w / 3) + ", " + std::to_string(2 * h / 5) + "], ";
        std::string PtA1 = "[" + std::to_string(w / 3) + ", " + std::to_string(2 * h / 5) + "], ";
        std::string PtB2 = "[" + std::to_string(2 * w / 3) + ", " + std::to_string(3 * h / 5) + "], ";
        std::string PtB1 = "[" + std::to_string(w / 3) + ", " + std::to_string(3 * h / 5) + "], ";
        std::string PtD = "[" + std::to_string(w / 2) + ", " + std::to_string(4 * h / 5) + "], ";
        std::string complete = "[" + PtC + PtA2 + PtA1 + PtB2 + PtB1 + PtD + "]";
        aiOriginResult = QString::fromStdString(complete);
    }

    AnnotateTab* annotateTab = this->parent->annotateTab;
    //annotateTab->imageId = imageId;

    QStringList list = aiOriginResult.split(",");
    for (int i = 0; i < list.size(); i++) {
        QString chopped = list[i].remove("[").remove("]");
        float f = chopped.toFloat();

        switch (i) {
            case 0: annotateTab->predictedCX = f; break;
            case 1: annotateTab->predictedCY = f; break;
            case 2: annotateTab->predictedA2X = f; break;
            case 3: annotateTab->predictedA2Y = f; break;
            case 4: annotateTab->predictedA1X = f; break;
            case 5: annotateTab->predictedA1Y = f; break;
            case 6: annotateTab->predictedB2X = f; break;
            case 7: annotateTab->predictedB2Y = f; break;
            case 8: annotateTab->predictedB1X = f; break;
            case 9: annotateTab->predictedB1Y = f; break;
            case 10: annotateTab->predictedDX = f; break;
            case 11: annotateTab->predictedDY = f; break;
        }
    }

    //if (!KinectEngine::getInstance().isDeviceOpened()) {
    //    KinectEngine::getInstance().configDevice();
    //    bool isSuccess = KinectEngine::getInstance().openDevice();

    //    if (!isSuccess) {
    //        TwoLinesDialog dialog;
    //        dialog.setLine1("Kinect device cannot be opened!");
    //        dialog.setLine2("Please check it and try again.");
    //        dialog.exec();
    //        return;
    //    }
    //}

    isDownloading = false;

    /** Progress bar UI */
    this->parent->ui.progressBar_2->setValue(100);
    this->parent->ui.progressBar_2->setTextVisible(false);
    /** Progress bar UI END */

    this->parent->annotateTab->reloadCurrentImage(qColorImage, AlignedDepthIMG1);
    this->parent->ui.tabWidget->setTabEnabled(TabIndex::ANNOTATETAB, true);
    this->parent->ui.tabWidget->setCurrentIndex(TabIndex::ANNOTATETAB);

    this->parent->ui.progressBar_2->setValue(0);

    /*ShowImagesDialog dialog;
    dialog.setQColorImage(qColorImage);
    dialog.setQDepthImage1(qDepthImage1);
    dialog.setQDepthImage2(qDepthImage2);
    dialog.exec();*/
}


//void PatientTab::onDownloadImage(QNetworkReply* reply) {
//    QByteArray imageData = reply->readAll();
//
//    QImage image = QImage::fromData(imageData);
//
//    qDebug() << "image format: " << image.format();
//
//    QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
//    QString savePath = QDir(visitFolderPath).filePath(QString::fromStdString("d2.png"));
//
//    image.save(savePath);
//
//    cv::Mat FourChannelPNG = cv::imread(savePath.toStdString(), -1);
//
//    int height = FourChannelPNG.rows;
//    int width = FourChannelPNG.cols;
//
//    qDebug() << "height: " << height << ", " << width;
//
//    std::vector<cv::Mat>channels(4);
//    cv::split(FourChannelPNG, channels);
//
//    /** ColorIMG */
//    cv::Mat ColorIMG = cv::Mat::zeros(height, width, CV_8UC3);
//    std::vector<cv::Mat>colorIMGChannels(3);
//    colorIMGChannels[0] = cv::Mat::zeros(height, width, CV_8UC1);
//    colorIMGChannels[1] = cv::Mat::zeros(height, width, CV_8UC1);
//    colorIMGChannels[2] = cv::Mat::zeros(height, width, CV_8UC1);
//
//    for (int i = 0; i < width * height; i++) {
//        colorIMGChannels[0].at<uint8_t>(i) = channels[1].at<uint16_t>(i) >> 8;
//        colorIMGChannels[1].at<uint8_t>(i) = channels[2].at<uint16_t>(i) % 256;
//        colorIMGChannels[2].at<uint8_t>(i) = channels[2].at<uint16_t>(i) >> 8;
//    }
//
//    cv::merge(colorIMGChannels, ColorIMG);
//
//    //cv::imshow("RGB", ColorIMG);
//    /** ColorIMG END */
//
//    /** Depth 1 */
//    cv::Mat AlignedDepthIMG1 = cv::Mat::zeros(height, width, CV_16UC1);
//    std::vector<cv::Mat>alignedDepthIMG1Channels(1);
//    alignedDepthIMG1Channels[0] = cv::Mat::zeros(height, width, CV_16UC1);
//
//    for (int i = 0; i < width * height; i++) {
//        alignedDepthIMG1Channels[0].at<uint16_t>(i) = channels[0].at<uint16_t>(i);
//    }
//
//    cv::merge(alignedDepthIMG1Channels, AlignedDepthIMG1);
//
//    // Convert from 16 bit to 8 bit for display
//    cv::Mat AlignedDepthIMG1_8bit;
//    AlignedDepthIMG1.convertTo(AlignedDepthIMG1_8bit, CV_8U, 255.0 / 5000.0, 0.0);
//    // Convert from 16 bit to 8 bit for display END
//
//    //cv::imshow("Depth1_8bit", AlignedDepthIMG1_8bit);
//    /** Depth 1 END */
//
//
//     /** Depth 2 */
//    cv::Mat AlignedDepthIMG2 = cv::Mat::zeros(height, width, CV_16UC1);
//    std::vector<cv::Mat>alignedDepthIMG2Channels(1);
//    alignedDepthIMG2Channels[0] = cv::Mat::zeros(height, width, CV_16UC1);
//
//    for (int i = 0; i < width * height; i++) {
//        alignedDepthIMG2Channels[0].at<uint16_t>(i) = channels[3].at<uint16_t>(i);
//    }
//
//    cv::merge(alignedDepthIMG2Channels, AlignedDepthIMG2);
//
//    // Convert from 16 bit to 8 bit for display
//    cv::Mat AlignedDepthIMG2_8bit;
//    AlignedDepthIMG2.convertTo(AlignedDepthIMG2_8bit, CV_8U, 255.0 / 5000.0, 0.0);
//    // Convert from 16 bit to 8 bit for display END
//
//    //cv::imshow("Depth2_8bit", AlignedDepthIMG2_8bit);
//    /** Depth 2 END */
//
//    /** Convert CV image to QImage */
//    QImage qColorImage = convertColorCVToQImage(ColorIMG);
//    QImage qDepthImage1 = convertDepthToColorCVToColorizedQImageDetailed(AlignedDepthIMG1); // 16 bit
//    QImage qDepthImage2 = convertDepthToColorCVToColorizedQImageDetailed(AlignedDepthIMG2); // 16 bit
//    /** Convert CV image to QImage END */
//
//    ShowImagesDialog dialog;
//    dialog.setQColorImage(qColorImage);
//    dialog.setQDepthImage1(qDepthImage1);
//    dialog.setQDepthImage2(qDepthImage2);
//    dialog.exec();
//}
