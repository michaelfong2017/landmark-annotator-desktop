#include "patientlisttab.h"
#include "createnewpatientdialog.h"
#include "qnetworkclient.h"

const int COLUMN_COUNT = 16;

PatientListTab::PatientListTab(DesktopApp* parent)
{
	this->parent = parent;

    tableView = this->parent->ui.patientListTab->findChild<QTableView*>("tableView");

    //patientListDataModel = new QStandardItemModel(0, COLUMN_COUNT, this);
    //tableView->setModel(this->patientListDataModel);

    patientListDataModel = new QStandardItemModel(0, COLUMN_COUNT, this);
    PatientListSortFilterProxyModel* proxyModel = new PatientListSortFilterProxyModel(this);
    proxyModel->setSourceModel(patientListDataModel);
    tableView->setModel(proxyModel);
    tableView->setSortingEnabled(true);


    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    tableView->verticalHeader()->hide();

    /** Handle double click row */
    bool value = connect(tableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onSlotRowDoubleClicked(const QModelIndex&)));
    /** Handle double click row END */

    QObject::connect(this->parent->ui.patientListTab->findChild<QPushButton*>("createNewPatientButton"), &QPushButton::clicked, [this]() {
        CreateNewPatientDialog dialog(this);
        dialog.exec();
        
        /** After dialog is finished */
        // Re-fetch data
        this->onEnterTab();
		});

    /** Patient list table pagination */
    //QObject::connect(this->parent->ui.patientListTab->findChild<QPushButton*>("patientListPreviousPageButton"), &QPushButton::clicked, [this]() {
    //    //qDebug() << "Previous page button clicked";
    //    //if (currentPageIndex != 0) {
    //    //    currentPageIndex--;
    //    //}

    //    // Re-fetch data
    //    this->onEnterTab();
    //    });

    //QObject::connect(this->parent->ui.patientListTab->findChild<QPushButton*>("patientListNextPageButton"), &QPushButton::clicked, [this]() {
    //    //qDebug() << "Next page button clicked";
    //    //currentPageIndex++;

    //    // Re-fetch data
    //    this->onEnterTab();
    //    });
    /** Patient list table pagination END */

    this->parent->ui.patientListPreviousPageButton->setVisible(false);
    this->parent->ui.patientListNextPageButton->setVisible(false);
    this->parent->ui.patientListPageLabel->setVisible(false);
}

DesktopApp* PatientListTab::getParent()
{
	return this->parent;
}

void PatientListTab::onEnterTab() {
    tableView->hide();
    QNetworkClient::getInstance().fetchPatientList(this, SLOT(onFetchPatientList(QNetworkReply*)));
}

void PatientListTab::onFetchPatientList(QNetworkReply* reply) {
    qDebug() << "onFetchPatientList";

    // Clear data and re-fetch all data every time
    patientListDataModel->clear();

    /** Headers */
    QStringList headerLabels = { "", "Patient ID", "Name", "Gender", "Age", "Phone Number", "Subject Number", "Creation Time", "idCard", "sin", "email", "address", "remark", "dob", "weight", "height"};

    for (int i = 0; i < COLUMN_COUNT; i++)
    {
        QString text = headerLabels.at(i);
        QStandardItem* item = new QStandardItem(text);
        QFont fn = item->font();
        fn.setPixelSize(14);
        item->setFont(fn);

        patientListDataModel->setHorizontalHeaderItem(i, item);
    }

    tableView->setColumnWidth(0, 18);

    /** This must be put here (below) */
    for (int col = 1; col < COLUMN_COUNT; col++)
    {
        tableView->setColumnWidth(col, 166);
    }
    /** This must be put here (below) END */
    /** Headers END */

    tableView->show();

    QByteArray response_data = reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["items"].toArray();

    qDebug() << jsonArray;
    qDebug() << "Number of returned items:" << jsonArray.size();

    //while (jsonArray.size() != 0 && ROWS_PER_PAGE * this->currentPageIndex >= jsonArray.size()) {
    //    if (currentPageIndex != 0) {
    //        this->currentPageIndex--;
    //    }
    //}

    //this->parent->ui.patientListTab->findChild<QLabel*>("patientListPageLabel")->setText(QString("Page %1").arg(currentPageIndex+1));

    //for (int i = ROWS_PER_PAGE * this->currentPageIndex; i < ROWS_PER_PAGE * (this->currentPageIndex + 1); i++) {
    //        break;
    //    }
    for (int i=0; i<jsonArray.size(); i++) {
        
        QJsonValue value = jsonArray.at(i);
        QJsonObject obj = value.toObject();
        //qDebug() << obj["name"].toString();
        //qDebug() << obj["sex"].toString();
        //qDebug() << obj["birthday"].toString();
        //qDebug() << obj["phoneNumber"].toString();
        //qDebug() << obj["subjectNumber"].toString();

        /** Construct a map with patientId as the key and save folder path as the value */
        patientIdToSaveFolderPath.insert_or_assign(obj["patientId"].toInt(), obj["name"].toString() + "_" + obj["birthday"].toString() + "_" + obj["idCard"].toString());
        /** Construct a map with patientId as the key and save folder path as the value END */


        QList<QStandardItem*> itemList;
        QStandardItem* item;
        for (int j = 0; j < COLUMN_COUNT; j++)
        {
            QString text;
            switch (j) {
                case 0:
                    text = QString::number(jsonArray.size() - i);
                    break;
                case 1:
                    text = QString::number(obj["patientId"].toInt());
                    break;
                case 2:
                    text = obj["name"].toString();
                    break;
                case 3:
                    text = obj["sexTxt"].toString();
                    break;
                case 4:
                {
                    if (obj["birthday"].toString().isEmpty()) {
                        text = QString();
                        break;
                    }

                    QDate date = QDate::fromString(obj["birthday"].toString(), "yyyy-MM-dd");
                    QDate today = QDate::currentDate();
                    int age = today.year() - date.year();

                    if (today.month() < date.month()) {
                        age--;
                    }
                    else if (today.month() == date.month() && today.day() < date.day()) {
                        age--;
                    }

                    text = QString::number(age);
                    break;
                }
                case 5:
                    text = obj["phoneNumber"].toString();
                    break;
                case 6:
                    text = obj["subjectNumber"].toString();
                    break;
                case 7:
                    text = obj["relationTime"].toString();
                    text = Helper::convertFetchedDateTime(text);
                    break;
                case 8:
                    text = obj["idCard"].toString();
                    break;
                case 9:
                    text = obj["sin"].toString();
                    break;
                case 10:
                    text = obj["email"].toString();
                    break;
                case 11:
                    text = obj["address"].toString();
                    break;
                case 12:
                    text = obj["remark"].toString();
                    break;
                case 13:
                    text = obj["birthday"].toString();
                    break;
                case 14:
                    text = obj["weight"].toString();
                    break;
                case 15:
                    text = obj["height"].toString();
                    break;
            }
            item = new QStandardItem(text);
            QFont fn = item->font();
            fn.setPixelSize(14);
            item->setFont(fn);
            itemList << item;
        }

        patientListDataModel->insertRow(patientListDataModel->rowCount(), itemList);

        QModelIndex curIndex = patientListDataModel->index(patientListDataModel->rowCount() - 1, 0);
    }
    // Sort by creation date in descending order, and hide patientId column
    patientListDataModel->sort(7, Qt::DescendingOrder);

    tableView->hideColumn(1);

    tableView->hideColumn(8);
    tableView->hideColumn(9);
    tableView->hideColumn(10);
    tableView->hideColumn(11);
    tableView->hideColumn(12);
    tableView->hideColumn(13);
    tableView->hideColumn(14);
    tableView->hideColumn(15);
    // Sort and hide END
    reply->deleteLater();

}

void PatientListTab::onSlotRowDoubleClicked(const QModelIndex &index) {
    int row = tableView->currentIndex().row();
    QModelIndex curIndex = patientListDataModel->index(row, 1);
    
    int currentPatientId = patientListDataModel->data(curIndex).toInt();
    qDebug() << "Selected patientId is" << currentPatientId << "";

    QModelIndex curIndex2 = patientListDataModel->index(row, 2);
    this->parent->patientTab->setCurrentPatientId(currentPatientId);

    QString name = patientListDataModel->data(curIndex2).toString();
    this->parent->patientTab->setCurrentPatientName(name);

    /** Use the map with patientId as the key and save folder path as the value */
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    QDir path = QDir((dir.absolutePath()) + "/" + patientIdToSaveFolderPath[currentPatientId]);

    this->parent->savePath = path;
    /** Handle Chinese name when saving video */
    this->parent->tempVideoSavePath = QDir("/tempVideoSaveFolder");
    /** Handle Chinese name when saving video END */
    qDebug() << "Selected patient DIR:::" << path;
    
    /** Use map with patientId as the key and save folder path as the value END */

    for (int i = 0; i < COLUMN_COUNT; i++)
    {
        QString data;
        QModelIndex index;
        index = patientListDataModel->index(row, i);
        data = patientListDataModel->data(index).toString();
        switch (i) {
            case 2:
                this->parent->patientTab->setName(data);
                //qDebug() << "Selected Name is" << data;
                break;
            case 3:
                this->parent->patientTab->setSex(data);
                //qDebug() << "Selected Sex is" << data;
                break;
            case 4:
                this->parent->patientTab->setAge(data);
                //qDebug() << "Selected Age is" << data;
                break;
            case 5:
                this->parent->patientTab->setPhoneNumber(data);
                //qDebug() << "Selected Phone Number is" << data;
                break;
            case 6:
                this->parent->patientTab->setSubjectNumber(data);
                //qDebug() << "Selected Subject number is" << data;
                break;
            case 8:
                this->parent->patientTab->setIdCard(data);
                //qDebug() << "Selected ID Card is" << data;
                break;
            case 9:
                this->parent->patientTab->setSin(data);
                //qDebug() << "Selected Social Security Number is" << data;
                break;
            case 10:
                this->parent->patientTab->setEmail(data);
                //qDebug() << "Selected Email is" << data;
                break;
            case 11:
                this->parent->patientTab->setAddress(data);
                //qDebug() << "Selected Address is" << data;
                break;
            case 12:
                this->parent->patientTab->setRemark(data);
                //qDebug() << "Selected Remark is" << data;
                break;
            case 13:
                this->parent->patientTab->setDOB(data);
                qDebug() << "Selected DOB is" << data;
                break;
            case 14:
                this->parent->patientTab->setWeight(data);
                qDebug() << "Selected Weight is" << data;
                break;
            case 15:
                this->parent->patientTab->setHeight(data);
                qDebug() << "Selected Height is" << data;
                break;
            }
    }

    this->parent->ui.tabWidget->setTabEnabled(2, true);

    this->parent->ui.tabWidget->setCurrentIndex(2);
}

