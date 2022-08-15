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
        QDesktopServices::openUrl(QUrl(url));
    }
}
