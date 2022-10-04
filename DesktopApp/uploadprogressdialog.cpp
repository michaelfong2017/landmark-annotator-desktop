#include "uploadprogressdialog.h"

UploadProgressDialog::UploadProgressDialog()
	: QDialog()
{
	ui.setupUi(this);

	tableView = this->ui.tableView;
	dataModel = new QStandardItemModel(0, COLUMN_COUNT, this);
	tableView->setModel(this->dataModel);

	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	tableView->verticalHeader()->hide();


    /** Headers */
    QStringList headerLabels = { "Upload number", "Patient Name", "Capture number", "Progress" };

    for (int i = 0; i < COLUMN_COUNT; i++)
    {
        QString text = headerLabels.at(i);
        QStandardItem* item = new QStandardItem(text);
        QFont fn = item->font();
        fn.setPixelSize(14);
        item->setFont(fn);

        dataModel->setHorizontalHeaderItem(i, item);
    }

    tableView->setColumnWidth(0, 150);

    /** This must be put here (below) */
    for (int col = 1; col < COLUMN_COUNT; col++)
    {
        tableView->setColumnWidth(col, 150);
    }

    tableView->show();

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
		QDialog::accept();
		});
}

void UploadProgressDialog::onUploading(QString patientName, int captureNumber)
{
    qDebug() << "UploadProgressDialog onUploading";

    qDebug() << requests.size();

    QList<QStandardItem*> itemList;
    QStandardItem* item;
    for (int j = 0; j < COLUMN_COUNT; j++)
    {
        QString text;
        switch (j) {
        case 0:
            text = QString::number(dataModel->rowCount() + 1);
            break;
        case 1:
            text = patientName;
            break;
        case 2:
            text = QString::number(captureNumber);
            break;
        case 3:
            text = QString("Uploading...");
            break;
        }
        item = new QStandardItem(text);
        QFont fn = item->font();
        fn.setPixelSize(14);
        item->setFont(fn);
        if (j == 3) {
            item->setForeground(QColor(Qt::red));
        }
        itemList << item;
    }

    dataModel->insertRow(0, itemList);
}

void UploadProgressDialog::onCompleted(int uploadNumber)
{
    qDebug() << "UploadProgressDialog onCompleted";

    QModelIndex index;
    index = dataModel->index(dataModel->rowCount() - uploadNumber, 3);
    dataModel->setData(index, QString("Completed"), Qt::DisplayRole);
    dataModel->setData(index, QColor(Qt::blue), Qt::ForegroundRole);

    // delete uploadrequest object << not work now
    //delete (requests.find(uploadNumber)->second);
}

void UploadProgressDialog::onFailed(int uploadNumber)
{
    qDebug() << "UploadProgressDialog onFailed";

    QModelIndex index;
    index = dataModel->index(dataModel->rowCount() - uploadNumber, 3);
    dataModel->setData(index, QString("Failed"), Qt::DisplayRole);
    dataModel->setData(index, QColor(Qt::red), Qt::ForegroundRole);
}