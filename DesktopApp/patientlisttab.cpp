#include "patientlisttab.h"
#include "createnewpatientdialog.h"
#include "qnetworkclient.h"

PatientListTab::PatientListTab(DesktopApp* parent)
{
	this->parent = parent;

    tableView = this->parent->ui.patientListTab->findChild<QTableView*>("tableView");
    patientListDataModel = new QStandardItemModel(0, 5, this);
    tableView->setModel(this->patientListDataModel);

    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

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
    QObject::connect(this->parent->ui.patientListTab->findChild<QPushButton*>("patientListPreviousPageButton"), &QPushButton::clicked, [this]() {
        //qDebug() << "Previous page button clicked";
        if (currentPageIndex != 0) {
            currentPageIndex--;
        }

        // Re-fetch data
        this->onEnterTab();
        });

    QObject::connect(this->parent->ui.patientListTab->findChild<QPushButton*>("patientListNextPageButton"), &QPushButton::clicked, [this]() {
        //qDebug() << "Next page button clicked";
        currentPageIndex++;

        // Re-fetch data
        this->onEnterTab();
        });
    /** Patient list table pagination END */
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
    QStringList headerLabels = { "Name", "Sex", "Age", "PhoneNumber", "SubjectNumber" };

    for (int i = 0; i < 5; i++)
    {
        QString text = headerLabels.at(i);
        QStandardItem* item = new QStandardItem(text);
        QFont fn = item->font();
        fn.setPointSize(11);
        item->setFont(fn);

        patientListDataModel->setHorizontalHeaderItem(i, item);
    }

    /** This must be put here (below) */
    for (int col = 0; col < 5; col++)
    {
        tableView->setColumnWidth(col, 200);
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

    while (ROWS_PER_PAGE * this->currentPageIndex >= jsonArray.size()) {
        if (currentPageIndex != 0) {
            this->currentPageIndex--;
        }
    }

    this->parent->ui.patientListTab->findChild<QLabel*>("patientListPageLabel")->setText(QString("Page %1").arg(currentPageIndex+1));

    for (int i = ROWS_PER_PAGE * this->currentPageIndex; i < ROWS_PER_PAGE * (this->currentPageIndex + 1); i++) {
        if (i > jsonArray.size() - 1) {
            break;
        }
        
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

        /** Store patientId of each patient in memory for later processing */
        int patientId;
        patientId = obj["patientId"].toInt();
        //qDebug() << "patientId is " << patientId;
        patientIdVector.push_back(patientId);
        /** Store patientId of each patient in memory for later processing END */

        QList<QStandardItem*> itemList;
        QStandardItem* item;
        for (int i = 0; i < 5; i++)
        {
            QString text;
            switch (i) {
                case 0:
                    text = obj["name"].toString();
                 break;
                case 1:
                    text = obj["sexTxt"].toString();
                    break;
                case 2:
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
                case 3:
                    text = obj["phoneNumber"].toString();
                    break;
                case 4:
                    text = obj["subjectNumber"].toString();
                    break;
            }
            item = new QStandardItem(text);
            QFont fn = item->font();
            fn.setPointSize(11);
            item->setFont(fn);
            itemList << item;
        }

        patientListDataModel->insertRow(patientListDataModel->rowCount(), itemList);

        QModelIndex curIndex = patientListDataModel->index(patientListDataModel->rowCount() - 1, 0);
    }
    reply->deleteLater();

}

void PatientListTab::onSlotRowDoubleClicked(const QModelIndex &index) {
    int row = tableView->currentIndex().row();
    qDebug() << "Selected patientId is" << patientIdVector[row];

    this->parent->patientTab->setCurrentPatientId(patientIdVector[row]);

    /** Use the map with patientId as the key and save folder path as the value */
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    QDir path = QDir((dir.absolutePath()) + "/" + patientIdToSaveFolderPath[patientIdVector[row]]);

    this->parent->savePath = path;
    qDebug() << "DIR:::" << path;
    
    /** Use map with patientId as the key and save folder path as the value END */

    for (int i = 0; i < 5; i++)
    {
        QString data;
        QModelIndex index;
        switch (i) {
        case 0:
            index = patientListDataModel->index(row, i);
            data = patientListDataModel->data(index).toString();
            this->parent->patientTab->setName(data);
            break;
        case 2:
            index = patientListDataModel->index(row, i);
            data = patientListDataModel->data(index).toString();
            this->parent->patientTab->setAge(data);
            break;
        case 4:
            index = patientListDataModel->index(row, i);
            data = patientListDataModel->data(index).toString();
            this->parent->patientTab->setSubjectNumber(data);
            break;
        }
       
    }

    this->parent->ui.tabWidget->setCurrentIndex(2);
}

