#include "patienttab.h"
#include "patientdatamodel.h"

PatientTab::PatientTab(DesktopApp* parent)
{
    this->parent = parent;

    QTableView* tableView = this->parent->ui.patientTab->findChild<QTableView*>("tableViewPatient");
    PatientDataModel* patientDataModel = new PatientDataModel();
    tableView->setModel(patientDataModel);

    for (int col = 0; col < 2; col++)
    {
        tableView->setColumnWidth(col, 500);
    }

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->show();

    QObject::connect(this->parent->ui.patientTab->findChild<QPushButton*>("captureNewButton"), &QPushButton::clicked, [this]() {
        qDebug() << "captureNewButton clicked";
        this->parent->ui.tabWidget->setCurrentIndex(3);
        });
}

DesktopApp* PatientTab::getParent()
{
    return this->parent;
}
