#include "landmarksconfirmed.h"

LandmarksConfirmedDialog::LandmarksConfirmedDialog()
	: QDialog()
{
	ui.setupUi(this);

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
		QDialog::accept();
		});
}