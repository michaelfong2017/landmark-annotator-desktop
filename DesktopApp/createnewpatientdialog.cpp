#include "createnewpatientdialog.h"

CreateNewPatientDialog::CreateNewPatientDialog(PatientListTab* parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	this->parent = parent;

	ui.buttonBox->addButton("Add", QDialogButtonBox::AcceptRole);

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
		QDialog::accept();
	});
}