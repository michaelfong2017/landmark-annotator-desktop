#include "twolinesdialog.h"

TwoLinesDialog::TwoLinesDialog()
	: QDialog()
{
	ui.setupUi(this);

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
		QDialog::accept();
		});
}

void TwoLinesDialog::setLine1(QString text)
{
	ui.label->setText(text);
}

void TwoLinesDialog::setLine2(QString text)
{
	ui.label_2->setText(text);
}
