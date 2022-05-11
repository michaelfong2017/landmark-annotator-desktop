#include "patientlisttab.h"
#include "patientlistdatamodel.h"
#include "createnewpatientdialog.h"
#include "qnetworkclient.h"

PatientListTab::PatientListTab(DesktopApp* parent)
{
	this->parent = parent;

    QTableView* tableView = this->parent->ui.patientListTab->findChild<QTableView*>("tableView");
    PatientListDataModel* patientListDataModel = new PatientListDataModel();
    tableView->setModel(patientListDataModel);

    for (int col = 0; col < 5; col++)
    {
        tableView->setColumnWidth(col, 200);
    }

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->show();

    QObject::connect(this->parent->ui.patientListTab->findChild<QPushButton*>("createNewPatientButton"), &QPushButton::clicked, [this]() {
        qDebug() << "createNewPatientButton clicked";
        CreateNewPatientDialog dialog(this);
        dialog.exec();
		});
}

DesktopApp* PatientListTab::getParent()
{
	return this->parent;
}

void PatientListTab::onEnterTab() {
    QNetworkClient::getInstance().fetchPatientList(this, SLOT(onFetchPatientList(QNetworkReply*)));
}

void PatientListTab::onFetchPatientList(QNetworkReply* reply) {
    QByteArray response_data = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(response_data);

    qDebug() << json;

    reply->deleteLater();

}

