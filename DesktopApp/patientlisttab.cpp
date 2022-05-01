#include "patientlisttab.h"
#include "patientlistdatamodel.h"

PatientListTab::PatientListTab(DesktopApp* parent)
{
	this->parent = parent;

    QTableView* tableView = this->parent->ui.tableView;
    PatientListDataModel* patientListDataModel = new PatientListDataModel();
    tableView->setModel(patientListDataModel);

    for (int col = 0; col < 5; col++)
    {
        tableView->setColumnWidth(col, 200);
    }

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->show();
}

DesktopApp* PatientListTab::getParent()
{
	return this->parent;
}
