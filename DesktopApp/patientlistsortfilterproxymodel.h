#ifndef PATIENTLISTSORTFILTERPROXYMODEL_H
#define PATIENTLISTSORTFILTERPROXYMODEL_H

#include <stdafx.h>

class PatientListSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    PatientListSortFilterProxyModel(QObject* parent = nullptr);

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

};
#endif
