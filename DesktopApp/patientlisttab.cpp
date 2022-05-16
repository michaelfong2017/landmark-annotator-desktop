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


    foreach(const QJsonValue &value, jsonArray) {
        QJsonObject obj = value.toObject();
        //qDebug() << obj["name"].toString();
        //qDebug() << obj["sex"].toString();
        //qDebug() << obj["birthday"].toString();
        //qDebug() << obj["phoneNumber"].toString();
        //qDebug() << obj["subjectNumber"].toString();

        /** Store patientId of each patient in memory for later processing */
        int patientId;
        patientId = obj["patientId"].toInt();
        qDebug() << "patientId is " << patientId;
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
                    text = obj["sex"].toString();
                    break;
                case 2:
                    text = obj["birthday"].toString();
                    break;
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

    
}

