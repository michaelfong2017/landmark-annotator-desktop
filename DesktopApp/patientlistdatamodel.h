#ifndef PATIENTLISTDATAMODEL_H
#define PATIENTLISTDATAMODEL_H

#include <QtWidgets/QWidget>
#include "stdafx.h"

class PatientListDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PatientListDataModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

#endif
