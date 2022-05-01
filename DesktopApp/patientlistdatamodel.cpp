#include "patientlistdatamodel.h"

PatientListDataModel::PatientListDataModel(QObject* parent)
	: QAbstractTableModel(parent)
{
}

int PatientListDataModel::rowCount(const QModelIndex& /*parent*/) const
{
	return 30;
}

int PatientListDataModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 5;
}

QVariant PatientListDataModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();
	// generate a log message when this method gets called
	//qDebug() << QString("row %1, col %2, role %3")
	//    .arg(row).arg(col).arg(role);

	switch (role) {
	case Qt::DisplayRole:
		return QString("Row%1, Column%2")
			.arg(row + 1)
			.arg(col + 1);
	case Qt::FontRole:
		QFont font;
		font.setPointSize(11);
		return font;
		break;
		//case Qt::BackgroundRole:
		//    if (row == 1 && col == 2)  //change background only for cell(1,2)
		//        return QBrush(Qt::red);
		//    break;
		//case Qt::TextAlignmentRole:
		//    if (row == 1 && col == 1) //change text alignment only for cell(1,1)
		//        return int(Qt::AlignRight | Qt::AlignVCenter);
		//    break;
		//case Qt::CheckStateRole:
		//    if (row == 1 && col == 0) //add a checkbox to cell(1,0)
		//        return Qt::Checked;
		//    break;
	}
	return QVariant();
}

QVariant PatientListDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role) {
	case Qt::DisplayRole:
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case 0:
				return QString("Name");
			case 1:
				return QString("Sex");
			case 2:
				return QString("Age");
			case 3:
				return QString("PhoneNumber");
			case 4:
				return QString("SubjectNumber");
			}
		}
		else if (orientation == Qt::Vertical) {
			return QString::number(section + 1);
		}
		break;
	case Qt::FontRole:
		QFont font;
		font.setPointSize(11);
		return font;
		break;
	}
	return QVariant();
}
