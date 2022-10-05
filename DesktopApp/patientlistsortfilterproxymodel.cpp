#include "patientlistsortfilterproxymodel.h"

PatientListSortFilterProxyModel::PatientListSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool PatientListSortFilterProxyModel::lessThan(const QModelIndex& left,
    const QModelIndex& right) const {
    if (left.isValid() && right.isValid())
        if (left.column() == 0 || left.column() == 4) // patient number column and age column
            return left.data().toInt() < right.data().toInt();
    return QSortFilterProxyModel::lessThan(left, right);

}