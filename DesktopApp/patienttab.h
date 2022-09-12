#ifndef PATIENTTAB_H
#define PATIENTTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"
#include "qnetworkclient.h"
#include <kinectengine.h>
#include <twolinesdialog.h>
#include <showimagesdialog.h>
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
    QString getSex();
    void setAge(QString age);
    QString getAge();
    void setDOB(QString dob);
    QString getDOB();
    void setPhoneNumber(QString phoneNumber);
    QString getPhoneNumber();
    void setSubjectNumber(QString subjectNumber);
    void setIdCard(QString idCard);
    void setSin(QString sin);
    void setEmail(QString email);
    void setAddress(QString address);
    void setRemark(QString remark);
    void setWeight(QString weight);
    QString getWeight();
    void setHeight(QString height);
    QString getHeight();

private:
    DesktopApp* parent;
    QTableView* tableView;
    QStandardItemModel* patientDataModel;
    int currentPatientId;
    QString currentPatientName;
    QString name;
    QString sex;
    QString age;
    QString dob;
    QString phoneNumber;
    QString subjectNumber;
    QString idCard;
    QString sin;
    QString email;
    QString address;
    QString remark;
    QString weight;
    QString height;

private slots:
    void onFetchExistingImagesOfPatient(QNetworkReply* reply);
    void onTableClicked(const QModelIndex& index);
    void onDownloadImage(QNetworkReply* reply);
};

#endif
