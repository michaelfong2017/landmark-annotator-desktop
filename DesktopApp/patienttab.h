#ifndef PATIENTTAB_H
#define PATIENTTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"
#include "qnetworkclient.h"
#include <kinectengine.h>
#include <twolinesdialog.h>
#include "helper.h"

class PatientTab : public QWidget
{
    Q_OBJECT

public:
    PatientTab(DesktopApp* parent);
    DesktopApp* getParent();
    void onEnterTab();
    void setCurrentPatientId(int currentPatientId);
    int getCurrentPatientId();
    void setCurrentPatientName(QString currentPatientName);
    QString getCurrentPatientName();
    void setName(QString name);
    void setSex(QString sex);
    void setAge(QString age);
    void setPhoneNumber(QString phoneNumber);
    void setSubjectNumber(QString subjectNumber);
    void setIdCard(QString idCard);
    void setSin(QString sin);
    void setEmail(QString email);
    void setAddress(QString address);
    void setRemark(QString remark);

private:
    DesktopApp* parent;
    QTableView* tableView;
    QStandardItemModel* patientDataModel;
    int currentPatientId;
    QString currentPatientName;
    QString name;
    QString sex;
    QString age;
    QString phoneNumber;
    QString subjectNumber;
    QString idCard;
    QString sin;
    QString email;
    QString address;
    QString remark;

private slots:
    void onFetchExistingImagesOfPatient(QNetworkReply* reply);
    void onTableClicked(const QModelIndex& index);
};

#endif
