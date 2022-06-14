#include "loadingdialog.h"

LoadingDialog::LoadingDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.progressBar->setValue(1);
}

void LoadingDialog::SetBarValue(int i) {
	ui.progressBar->setValue(i);
}