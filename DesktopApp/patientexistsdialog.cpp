#include "patientexistsdialog.h"

PatientExistsDialog::PatientExistsDialog()
	: QDialog()
{
	ui.setupUi(this);

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
		QDialog::accept();
		});
}