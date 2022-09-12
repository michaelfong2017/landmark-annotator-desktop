#include "patienttab.h"
#include "capturetab.h"

PatientTab::PatientTab(DesktopApp* parent)
{
    this->parent = parent;

    tableView = this->parent->ui.patientTab->findChild<QTableView*>("tableViewPatient");
    patientDataModel = new QStandardItemModel(0, 2, this);
    tableView->setModel(this->patientDataModel);

    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    /** Open image url */
    connect(tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onTableClicked(const QModelIndex &)));
    /** Open image url END */

    QObject::connect(this->parent->ui.patientTab->findChild<QPushButton*>("captureNewButton"), &QPushButton::clicked, [this]() {
        qDebug() << "captureNewButton clicked";

        if (!KinectEngine::getInstance().isDeviceConnected()) {
            TwoLinesDialog dialog;
            dialog.setLine1("Kinect device cannot be opened!");
            dialog.setLine2("Please check it and try again.");
            dialog.exec();
            return;
        }

        if (!KinectEngine::getInstance().isDeviceOpened()) {
            KinectEngine::getInstance().configDevice();
            bool isSuccess = KinectEngine::getInstance().openDevice();

            if (!isSuccess) {
                TwoLinesDialog dialog;
                dialog.setLine1("Kinect device cannot be opened!");
                dialog.setLine2("Please check it and try again.");
                dialog.exec();
                return;
            }
        }

        // need to clear list
        this->parent->captureTab->clearCaptureHistories();

        this->parent->ui.tabWidget->setTabEnabled(3, true);
        this->parent->ui.tabWidget->setTabEnabled(4, true);
        this->parent->ui.tabWidget->setCurrentIndex(3);
        });
}

DesktopApp* PatientTab::getParent()
{
    return this->parent;
}

void PatientTab::onEnterTab() {
    tableView->hide();
    QNetworkClient::getInstance().fetchExistingImagesOfPatient(currentPatientId, this, SLOT(onFetchExistingImagesOfPatient(QNetworkReply*)));

    this->parent->ui.patientTab->findChild<QLabel*>("patientName")->setText(QString("Name: %1").arg(name));
    this->parent->ui.patientTab->findChild<QLabel*>("patientAge")->setText(QString("Age: %1").arg(age));
    this->parent->ui.patientTab->findChild<QLabel*>("patientSubjectNumber")->setText(QString("Subject Number: %1").arg(subjectNumber));
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

void PatientTab::onFetchExistingImagesOfPatient(QNetworkReply* reply) {
    // Clear data and re-fetch all data every time
    patientDataModel->clear();

    /** Headers */
    QStringList headerLabels = { "Captured Date", "Image" };

    for (int i = 0; i < 2; i++)
    {
        QString text = headerLabels.at(i);
        QStandardItem* item = new QStandardItem(text);
        QFont fn = item->font();
        fn.setPixelSize(14);
        item->setFont(fn);

        patientDataModel->setHorizontalHeaderItem(i, item);
    }

    /** This must be put here (below) */
    for (int col = 0; col < 2; col++)
    {
        tableView->setColumnWidth(col, 500);
    }
    /** This must be put here (below) END */
    /** Headers END */

    tableView->show();

    QByteArray response_data = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

    qDebug() << jsonResponse;

    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["items"].toArray();


    foreach(const QJsonValue & value, jsonArray) {
        QJsonObject obj = value.toObject();

        QList<QStandardItem*> itemList;
        QStandardItem* item;
        for (int i = 0; i < 2; i++)
        {
            if (i == 0) {
                QString text;

                text = obj["takeDate"].toString();
                text = Helper::convertFetchedDateTime(text);

                item = new QStandardItem(text);
                QFont fn = item->font();
                fn.setPixelSize(14);

                item->setFont(fn);
            }
            else if (i == 1) {
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

            itemList << item;
        }

        patientDataModel->insertRow(patientDataModel->rowCount(), itemList);

        QModelIndex curIndex = patientDataModel->index(patientDataModel->rowCount() - 1, 0);
    }
    reply->deleteLater();
}

void PatientTab::onTableClicked(const QModelIndex& index)
{
    if (index.isValid() && index.column() == 1) {
        QString url = index.data().toString();
        //QDesktopServices::openUrl(QUrl(url));

        QNetworkClient::getInstance().downloadImage(url, this, SLOT(onDownloadImage(QNetworkReply*)));
    }
}

void PatientTab::onDownloadImage(QNetworkReply* reply) {
    QByteArray imageData = reply->readAll();

    QImage image = QImage::fromData(imageData);

    qDebug() << "image format: " << image.format();

    QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
    QString savePath = QDir(visitFolderPath).filePath(QString::fromStdString("d2.png"));

    image.save(savePath);

    cv::Mat FourChannelPNG = cv::imread(savePath.toStdString(), -1);

    int height = FourChannelPNG.rows;
    int width = FourChannelPNG.cols;

    qDebug() << "height: " << height << ", " << width;

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
        colorIMGChannels[1].at<uint8_t>(i) = channels[2].at<uint16_t>(i) % 256;
        colorIMGChannels[2].at<uint8_t>(i) = channels[2].at<uint16_t>(i) >> 8;
    }

    cv::merge(colorIMGChannels, ColorIMG);

    //cv::imshow("RGB", ColorIMG);
    /** ColorIMG END */

    /** Depth 1 */
    cv::Mat AlignedDepthIMG1 = cv::Mat::zeros(height, width, CV_16UC1);
    std::vector<cv::Mat>alignedDepthIMG1Channels(1);
    alignedDepthIMG1Channels[0] = cv::Mat::zeros(height, width, CV_16UC1);

    for (int i = 0; i < width * height; i++) {
        alignedDepthIMG1Channels[0].at<uint16_t>(i) = channels[0].at<uint16_t>(i);
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
    QImage qDepthImage1 = convertDepthToColorCVToColorizedQImageDetailed(AlignedDepthIMG1); // 16 bit
    QImage qDepthImage2 = convertDepthToColorCVToColorizedQImageDetailed(AlignedDepthIMG2); // 16 bit
    /** Convert CV image to QImage END */

    ShowImagesDialog dialog;
    dialog.setQColorImage(qColorImage);
    dialog.setQDepthImage1(qDepthImage1);
    dialog.setQDepthImage2(qDepthImage2);
    dialog.exec();
}
