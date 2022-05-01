#include "patientlisttab.h"
#include "patientlistdatamodel.h"

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
        //SaveImageDialog dialog(this);
        //dialog.exec();
		});
}

DesktopApp* PatientListTab::getParent()
{
	return this->parent;
}
