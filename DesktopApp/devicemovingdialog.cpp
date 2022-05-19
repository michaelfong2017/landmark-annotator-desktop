#include "devicemovingdialog.h"

DeviceMovingDialog::DeviceMovingDialog(CaptureTab* parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	this->parent = parent;
}
