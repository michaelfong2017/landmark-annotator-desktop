#ifndef PATIENTLISTTAB_H
#define PATIENTLISTTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"
#include <QtNetwork>
#include <vector>
#include "patienttab.h"

class PatientListTab : public QWidget
{
    Q_OBJECT

public:
    PatientListTab(DesktopApp* parent);
    DesktopApp* getParent();
    void onEnterTab();
    std::map<int, QString> patientIdToSaveFolderPath;

private:
    DesktopApp* parent;
    QTableView* tableView;
    QStandardItemModel* patientListDataModel;
    std::vector<int> patientIdVector;

private slots:
    void onFetchPatientList(QNetworkReply* reply);
    void onSlotRowDoubleClicked(const QModelIndex &index);
};

#endif
