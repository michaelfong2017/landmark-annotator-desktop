#ifndef PATIENTTAB_H
#define PATIENTTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"
#include "qnetworkclient.h"

class PatientTab : public QWidget
{
    Q_OBJECT

public:
    PatientTab(DesktopApp* parent);
    DesktopApp* getParent();
    void onEnterTab();
    void setCurrentPatientId(int currentPatientId);
    int getCurrentPatientId();
    void setName(QString name);
    void setAge(QString age);
    void setSubjectNumber(QString subjectNumber);

private:
    DesktopApp* parent;
    QTableView* tableView;
    QStandardItemModel* patientDataModel;
    int currentPatientId;
    QString name;
    QString age;
    QString subjectNumber;

private slots:
    void onFetchExistingImagesOfPatient(QNetworkReply* reply);
    void onTableClicked(const QModelIndex& index);
};

#endif
