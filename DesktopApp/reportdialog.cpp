#include "reportdialog.h"
#include <QPrintDialog>
#include <QPrintPreviewDialog>

ReportDialog::ReportDialog(AnnotateTab* parent) : QDialog(parent)
{
	ui.setupUi(this);

	this->parent = parent;

	/** For changing language */
	this->setAllChiLabelsFromUI();
	this->setAllEngLabelsFromAnotherUI();
	/** For changing language END */

	this->setFixedSize(this->width(), this->height());

	if (!this->parent->invalidDistance) {

		ui.variable1Labe2->setText(QString::fromStdString(std::to_string(this->parent->distance1)));

		if (this->parent->distance1 > 0) {
			ui.rightCheckBox1->setChecked(true);
		}
		else if (this->parent->distance1 < 0) {
			ui.leftCheckBox1->setChecked(true);
		}
		else {

		}
	}
	else {
		ui.variable1Labe2->setText(QString::fromStdString("Invalid"));
	}

	ui.variable1Label4->setText(QString::fromStdString(std::to_string(this->parent->angle2)));
	if (this->parent->angle2 > 0) {
		ui.rightCheckBox2->setChecked(true);
	}
	else if (this->parent->angle2 < 0) {
		ui.leftCheckBox2->setChecked(true);
	}
	else {

	}

	ui.variable1Label6->setText(QString::fromStdString(std::to_string(this->parent->angle1)));
	if (this->parent->angle1 > 0) {
		ui.rightCheckBox3->setChecked(true);
	}
	else if (this->parent->angle2 < 0) {
		ui.leftCheckBox3->setChecked(true);
	}
	else {

	}

	ui.variable1Label8->setText(QString::fromStdString(std::to_string(this->parent->trunkRotation)));
	if (this->parent->trunkRotation > 0) {
		ui.rightCheckBox4->setChecked(true);
	}
	else if (this->parent->angle2 < 0) {
		ui.leftCheckBox4->setChecked(true);
	}
	else {

	}
	ui.leftCheckBox1->setEnabled(false);
	ui.leftCheckBox2->setEnabled(false);
	ui.leftCheckBox3->setEnabled(false);
	ui.leftCheckBox4->setEnabled(false);
	ui.rightCheckBox1->setEnabled(false);
	ui.rightCheckBox2->setEnabled(false);
	ui.rightCheckBox3->setEnabled(false);
	ui.rightCheckBox4->setEnabled(false);

	ui.NameLabel->setText(this->parent->getParent()->patientTab->getCurrentPatientName());
	ui.PhoneLabel->setText(this->parent->getParent()->patientTab->getPhoneNumber());
	ui.GenerLabel->setText(this->parent->getParent()->patientTab->getSex());
	ui.BirthLabel->setText(this->parent->getParent()->patientTab->getDOB());
	ui.AgeLabel->setText(this->parent->getParent()->patientTab->getAge());
	ui.WeightLabel->setText(this->parent->getParent()->patientTab->getWeight());
	ui.HeightLabel->setText(this->parent->getParent()->patientTab->getHeight());

	int reportWidth = this->ui.graphicsView->width();
	int reportHeight = this->ui.graphicsView->height();

	int annotateWidth = this->parent->getParent()->ui.graphicsViewAnnotation2->width();
	int annotateHeight = this->parent->getParent()->ui.graphicsViewAnnotation2->height();

	float scale = std::min((float)reportWidth / annotateWidth, (float)reportHeight / annotateHeight);

	qDebug() << (float)reportWidth / annotateWidth;
	qDebug() << (float)reportHeight / annotateHeight;

	scale -= 0.05;

	ui.graphicsView->setScene(this->parent->getDepthToColorScene());
	ui.graphicsView->scale(scale, scale);
	ui.graphicsView->show();

	/*QObject::connect(ui.pdfButton, &QPushButton::clicked, [this]() {

		QWidget* w = ui.scrollAreaWidgetContents_3;

		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName("report.pdf");
		printer.setPageMargins(5, 10, 8, 16, QPrinter::Millimeter);
		printer.setFullPage(false);

		QPainter painter(&printer);

		double xscale = printer.pageRect().width() / double(w->width());
		double yscale = printer.pageRect().height() / double(w->height());
		double scale = qMin(xscale, yscale);
		painter.translate(double(printer.paperRect().x() + printer.pageRect().width() / 2), double(printer.paperRect().y() + printer.pageRect().height() / 2));
		painter.scale(scale, scale);
		painter.translate(-w->width() / 2, -w->height() / 2);
		w->render(&painter);

		QDialog::accept();
	});*/

	QObject::connect(ui.printButton, &QPushButton::clicked, [this]() {
		printWithPreview();
		});

	QObject::connect(ui.clearButton, &QPushButton::clicked, [this]() {
		ui.signView->clearImage();
		});

	/** Switch between english and chinese */
	QObject::connect(ui.chineseButton, &QPushButton::clicked, [this]() {
		ui.TitleLabel->setText(chi_TitleLabel);
		ui.label_15->setText(chi_label_15);
		ui.nameLabel_10->setText(chi_nameLabel_10);
		ui.leftCheckBox1->setText(chi_leftCheckBox1);
		ui.rightCheckBox1->setText(chi_rightCheckBox1);
		ui.nameLabel_8->setText(chi_nameLabel_8);
		ui.leftCheckBox2->setText(chi_leftCheckBox2);
		ui.rightCheckBox2->setText(chi_rightCheckBox2);
		ui.nameLabel_9->setText(chi_nameLabel_9);
		ui.leftCheckBox3->setText(chi_leftCheckBox3);
		ui.rightCheckBox3->setText(chi_rightCheckBox3);
		ui.nameLabel_11->setText(chi_nameLabel_11);
		ui.leftCheckBox4->setText(chi_leftCheckBox4);
		ui.rightCheckBox4->setText(chi_rightCheckBox4);
		ui.label_7->setText(chi_label_7);
		ui.label_17->setText(chi_label_17);

		ui.label_6->setText(chi_label_6);
		ui.label->setText(chi_label);
		ui.radioButton->setText(chi_radioButton);
		ui.label_18->setText(chi_label_18);
		ui.radioButton_3->setText(chi_radioButton_3);
		ui.label_19->setText(chi_label_19);
		ui.radioButton_2->setText(chi_radioButton_2);
		ui.label_20->setText(chi_label_20);
		ui.label_2->setText(chi_label_2);
		ui.label_8->setText(chi_label_8);
		ui.label_9->setText(chi_label_9);
		ui.label_10->setText(chi_label_10);
		ui.label_11->setText(chi_label_11);
		ui.label_12->setText(chi_label_12);
		ui.label_13->setText(chi_label_13);
		ui.label_22->setText(chi_label_22);
		ui.label_14->setText(chi_label_14);
		ui.label_16->setText(chi_label_16);
		ui.label_3->setText(chi_label_3);

		ui.label_4->setText(chi_label_4);


		ui.TitleLabel->setStyleSheet(chi_style_TitleLabel);
		ui.label_15->setStyleSheet(chi_style_label_15);
		ui.nameLabel_10->setStyleSheet(chi_style_nameLabel_10);
		ui.leftCheckBox1->setStyleSheet(chi_style_leftCheckBox1);
		ui.rightCheckBox1->setStyleSheet(chi_style_rightCheckBox1);
		ui.nameLabel_8->setStyleSheet(chi_style_nameLabel_8);
		ui.leftCheckBox2->setStyleSheet(chi_style_leftCheckBox2);
		ui.rightCheckBox2->setStyleSheet(chi_style_rightCheckBox2);
		ui.nameLabel_9->setStyleSheet(chi_style_nameLabel_9);
		ui.leftCheckBox3->setStyleSheet(chi_style_leftCheckBox3);
		ui.rightCheckBox3->setStyleSheet(chi_style_rightCheckBox3);
		ui.nameLabel_11->setStyleSheet(chi_style_nameLabel_11);
		ui.leftCheckBox4->setStyleSheet(chi_style_leftCheckBox4);
		ui.rightCheckBox4->setStyleSheet(chi_style_rightCheckBox4);
		ui.label_7->setStyleSheet(chi_style_label_7);
		ui.label_17->setStyleSheet(chi_style_label_17);

		ui.label_6->setStyleSheet(chi_style_label_6);
		ui.label->setStyleSheet(chi_style_label);
		ui.radioButton->setStyleSheet(chi_style_radioButton);
		ui.label_18->setStyleSheet(chi_style_label_18);
		ui.radioButton_3->setStyleSheet(chi_style_radioButton_3);
		ui.label_19->setStyleSheet(chi_style_label_19);
		ui.radioButton_2->setStyleSheet(chi_style_radioButton_2);
		ui.label_20->setStyleSheet(chi_style_label_20);
		ui.label_2->setStyleSheet(chi_style_label_2);
		ui.label_8->setStyleSheet(chi_style_label_8);
		ui.label_9->setStyleSheet(chi_style_label_9);
		ui.label_10->setStyleSheet(chi_style_label_10);
		ui.label_11->setStyleSheet(chi_style_label_11);
		ui.label_12->setStyleSheet(chi_style_label_12);
		ui.label_13->setStyleSheet(chi_style_label_13);
		ui.label_22->setStyleSheet(chi_style_label_22);
		ui.label_14->setStyleSheet(chi_style_label_14);
		ui.label_16->setStyleSheet(chi_style_label_16);
		ui.label_3->setStyleSheet(chi_style_label_3);

		ui.label_4->setStyleSheet(chi_style_label_4);
		});

	QObject::connect(ui.englishButton, &QPushButton::clicked, [this]() {
		ui.TitleLabel->setText(eng_TitleLabel);
		ui.label_15->setText(eng_label_15);
		ui.nameLabel_10->setText(eng_nameLabel_10);
		ui.leftCheckBox1->setText(eng_leftCheckBox1);
		ui.rightCheckBox1->setText(eng_rightCheckBox1);
		ui.nameLabel_8->setText(eng_nameLabel_8);
		ui.leftCheckBox2->setText(eng_leftCheckBox2);
		ui.rightCheckBox2->setText(eng_rightCheckBox2);
		ui.nameLabel_9->setText(eng_nameLabel_9);
		ui.leftCheckBox3->setText(eng_leftCheckBox3);
		ui.rightCheckBox3->setText(eng_rightCheckBox3);
		ui.nameLabel_11->setText(eng_nameLabel_11);
		ui.leftCheckBox4->setText(eng_leftCheckBox4);
		ui.rightCheckBox4->setText(eng_rightCheckBox4);
		ui.label_7->setText(eng_label_7);
		ui.label_17->setText(eng_label_17);

		ui.label_6->setText(eng_label_6);
		ui.label->setText(eng_label);
		ui.radioButton->setText(eng_radioButton);
		ui.label_18->setText(eng_label_18);
		ui.radioButton_3->setText(eng_radioButton_3);
		ui.label_19->setText(eng_label_19);
		ui.radioButton_2->setText(eng_radioButton_2);
		ui.label_20->setText(eng_label_20);
		ui.label_2->setText(eng_label_2);
		ui.label_8->setText(eng_label_8);
		ui.label_9->setText(eng_label_9);
		ui.label_10->setText(eng_label_10);
		ui.label_11->setText(eng_label_11);
		ui.label_12->setText(eng_label_12);
		ui.label_13->setText(eng_label_13);
		ui.label_22->setText(eng_label_22);
		ui.label_14->setText(eng_label_14);
		ui.label_16->setText(eng_label_16);
		ui.label_3->setText(eng_label_3);

		ui.label_4->setText(eng_label_4);


		ui.TitleLabel->setStyleSheet(eng_style_TitleLabel);
		ui.label_15->setStyleSheet(eng_style_label_15);
		ui.nameLabel_10->setStyleSheet(eng_style_nameLabel_10);
		ui.leftCheckBox1->setStyleSheet(eng_style_leftCheckBox1);
		ui.rightCheckBox1->setStyleSheet(eng_style_rightCheckBox1);
		ui.nameLabel_8->setStyleSheet(eng_style_nameLabel_8);
		ui.leftCheckBox2->setStyleSheet(eng_style_leftCheckBox2);
		ui.rightCheckBox2->setStyleSheet(eng_style_rightCheckBox2);
		ui.nameLabel_9->setStyleSheet(eng_style_nameLabel_9);
		ui.leftCheckBox3->setStyleSheet(eng_style_leftCheckBox3);
		ui.rightCheckBox3->setStyleSheet(eng_style_rightCheckBox3);
		ui.nameLabel_11->setStyleSheet(eng_style_nameLabel_11);
		ui.leftCheckBox4->setStyleSheet(eng_style_leftCheckBox4);
		ui.rightCheckBox4->setStyleSheet(eng_style_rightCheckBox4);
		ui.label_7->setStyleSheet(eng_style_label_7);
		ui.label_17->setStyleSheet(eng_style_label_17);

		ui.label_6->setStyleSheet(eng_style_label_6);
		ui.label->setStyleSheet(eng_style_label);
		ui.radioButton->setStyleSheet(eng_style_radioButton);
		ui.label_18->setStyleSheet(eng_style_label_18);
		ui.radioButton_3->setStyleSheet(eng_style_radioButton_3);
		ui.label_19->setStyleSheet(eng_style_label_19);
		ui.radioButton_2->setStyleSheet(eng_style_radioButton_2);
		ui.label_20->setStyleSheet(eng_style_label_20);
		ui.label_2->setStyleSheet(eng_style_label_2);
		ui.label_8->setStyleSheet(eng_style_label_8);
		ui.label_9->setStyleSheet(eng_style_label_9);
		ui.label_10->setStyleSheet(eng_style_label_10);
		ui.label_11->setStyleSheet(eng_style_label_11);
		ui.label_12->setStyleSheet(eng_style_label_12);
		ui.label_13->setStyleSheet(eng_style_label_13);
		ui.label_22->setStyleSheet(eng_style_label_22);
		ui.label_14->setStyleSheet(eng_style_label_14);
		ui.label_16->setStyleSheet(eng_style_label_16);
		ui.label_3->setStyleSheet(eng_style_label_3);

		ui.label_4->setStyleSheet(eng_style_label_4);
		});
	/** Switch between english and chinese END */
}

//void ReportDialog::resizeEvent(QResizeEvent* event)
//{
//	event->accept();
//
//	if (event->size().width() > event->size().height()) {
//		QWidget::resize(event->size().height(), event->size().height());
//	}
//	else {
//		QWidget::resize(event->size().width(), event->size().width());
//	}
//}



void ReportDialog::printWithPreview()
{

	QPrinter printer(QPrinter::HighResolution);
	printer.setPageSize(QPrinter::A4);
	printer.setOrientation(QPrinter::Portrait); //打印方向 Portrait 纵向，Landscape：横向
//    printer.setOutputFormat(QPrinter::NativeFormat);

	QPrintPreviewDialog preview(&printer);
	connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printDocument(QPrinter*)));
	preview.setWindowTitle("Preview Dialog");
	preview.setWindowState(Qt::WindowMaximized);
	preview.exec();
}

void ReportDialog::setAllChiLabelsFromUI()
{
	chi_TitleLabel = ui.TitleLabel->text();
	chi_label_15 = ui.label_15->text();
	chi_nameLabel_10 = ui.nameLabel_10->text();
	chi_leftCheckBox1 = ui.leftCheckBox1->text();
	chi_rightCheckBox1 = ui.rightCheckBox1->text();
	chi_nameLabel_8 = ui.nameLabel_8->text();
	chi_leftCheckBox2 = ui.leftCheckBox2->text();
	chi_rightCheckBox2 = ui.rightCheckBox2->text();
	chi_nameLabel_9 = ui.nameLabel_9->text();
	chi_leftCheckBox3 = ui.leftCheckBox3->text();
	chi_rightCheckBox3 = ui.rightCheckBox3->text();
	chi_nameLabel_11 = ui.nameLabel_11->text();
	chi_leftCheckBox4 = ui.leftCheckBox4->text();
	chi_rightCheckBox4 = ui.rightCheckBox4->text();
	chi_label_7 = ui.label_7->text();
	chi_label_17 = ui.label_17->text();

	chi_label_6 = ui.label_6->text();
	chi_label = ui.label->text();
	chi_radioButton = ui.radioButton->text();
	chi_label_18 = ui.label_18->text();
	chi_radioButton_3 = ui.radioButton_3->text();
	chi_label_19 = ui.label_19->text();
	chi_radioButton_2 = ui.radioButton_2->text();
	chi_label_20 = ui.label_20->text();
	chi_label_2 = ui.label_2->text();
	chi_label_8 = ui.label_8->text();
	chi_label_9 = ui.label_9->text();
	chi_label_10 = ui.label_10->text();
	chi_label_11 = ui.label_11->text();
	chi_label_12 = ui.label_12->text();
	chi_label_13 = ui.label_13->text();
	chi_label_22 = ui.label_22->text();
	chi_label_14 = ui.label_14->text();
	chi_label_16 = ui.label_16->text();
	chi_label_3 = ui.label_3->text();

	chi_label_4 = ui.label_4->text();


	chi_style_TitleLabel = ui.TitleLabel->styleSheet();
	chi_style_label_15 = ui.label_15->styleSheet();
	chi_style_nameLabel_10 = ui.nameLabel_10->styleSheet();
	chi_style_leftCheckBox1 = ui.leftCheckBox1->styleSheet();
	chi_style_rightCheckBox1 = ui.rightCheckBox1->styleSheet();
	chi_style_nameLabel_8 = ui.nameLabel_8->styleSheet();
	chi_style_leftCheckBox2 = ui.leftCheckBox2->styleSheet();
	chi_style_rightCheckBox2 = ui.rightCheckBox2->styleSheet();
	chi_style_nameLabel_9 = ui.nameLabel_9->styleSheet();
	chi_style_leftCheckBox3 = ui.leftCheckBox3->styleSheet();
	chi_style_rightCheckBox3 = ui.rightCheckBox3->styleSheet();
	chi_style_nameLabel_11 = ui.nameLabel_11->styleSheet();
	chi_style_leftCheckBox4 = ui.leftCheckBox4->styleSheet();
	chi_style_rightCheckBox4 = ui.rightCheckBox4->styleSheet();
	chi_style_label_7 = ui.label_7->styleSheet();
	chi_style_label_17 = ui.label_17->styleSheet();

	chi_style_label_6 = ui.label_6->styleSheet();
	chi_style_label = ui.label->styleSheet();
	chi_style_radioButton = ui.radioButton->styleSheet();
	chi_style_label_18 = ui.label_18->styleSheet();
	chi_style_radioButton_3 = ui.radioButton_3->styleSheet();
	chi_style_label_19 = ui.label_19->styleSheet();
	chi_style_radioButton_2 = ui.radioButton_2->styleSheet();
	chi_style_label_20 = ui.label_20->styleSheet();
	chi_style_label_2 = ui.label_2->styleSheet();
	chi_style_label_8 = ui.label_8->styleSheet();
	chi_style_label_9 = ui.label_9->styleSheet();
	chi_style_label_10 = ui.label_10->styleSheet();
	chi_style_label_11 = ui.label_11->styleSheet();
	chi_style_label_12 = ui.label_12->styleSheet();
	chi_style_label_13 = ui.label_13->styleSheet();
	chi_style_label_22 = ui.label_22->styleSheet();
	chi_style_label_14 = ui.label_14->styleSheet();
	chi_style_label_16 = ui.label_16->styleSheet();
	chi_style_label_3 = ui.label_3->styleSheet();

	chi_style_label_4 = ui.label_4->styleSheet();
}

void ReportDialog::setAllEngLabelsFromAnotherUI()
{
	ReportDialog_Eng dialog;
	Ui_ReportDialog_EngUI ui = dialog.ui;

	eng_TitleLabel = ui.TitleLabel->text();
	eng_label_15 = ui.label_15->text();
	eng_nameLabel_10 = ui.nameLabel_10->text();
	eng_leftCheckBox1 = ui.leftCheckBox1->text();
	eng_rightCheckBox1 = ui.rightCheckBox1->text();
	eng_nameLabel_8 = ui.nameLabel_8->text();
	eng_leftCheckBox2 = ui.leftCheckBox2->text();
	eng_rightCheckBox2 = ui.rightCheckBox2->text();
	eng_nameLabel_9 = ui.nameLabel_9->text();
	eng_leftCheckBox3 = ui.leftCheckBox3->text();
	eng_rightCheckBox3 = ui.rightCheckBox3->text();
	eng_nameLabel_11 = ui.nameLabel_11->text();
	eng_leftCheckBox4 = ui.leftCheckBox4->text();
	eng_rightCheckBox4 = ui.rightCheckBox4->text();
	eng_label_7 = ui.label_7->text();
	eng_label_17 = ui.label_17->text();

	eng_label_6 = ui.label_6->text();
	eng_label = ui.label->text();
	eng_radioButton = ui.radioButton->text();
	eng_label_18 = ui.label_18->text();
	eng_radioButton_3 = ui.radioButton_3->text();
	eng_label_19 = ui.label_19->text();
	eng_radioButton_2 = ui.radioButton_2->text();
	eng_label_20 = ui.label_20->text();
	eng_label_2 = ui.label_2->text();
	eng_label_8 = ui.label_8->text();
	eng_label_9 = ui.label_9->text();
	eng_label_10 = ui.label_10->text();
	eng_label_11 = ui.label_11->text();
	eng_label_12 = ui.label_12->text();
	eng_label_13 = ui.label_13->text();
	eng_label_22 = ui.label_22->text();
	eng_label_14 = ui.label_14->text();
	eng_label_16 = ui.label_16->text();
	eng_label_3 = ui.label_3->text();

	eng_label_4 = ui.label_4->text();


	eng_style_TitleLabel = ui.TitleLabel->styleSheet();
	eng_style_label_15 = ui.label_15->styleSheet();
	eng_style_nameLabel_10 = ui.nameLabel_10->styleSheet();
	eng_style_leftCheckBox1 = ui.leftCheckBox1->styleSheet();
	eng_style_rightCheckBox1 = ui.rightCheckBox1->styleSheet();
	eng_style_nameLabel_8 = ui.nameLabel_8->styleSheet();
	eng_style_leftCheckBox2 = ui.leftCheckBox2->styleSheet();
	eng_style_rightCheckBox2 = ui.rightCheckBox2->styleSheet();
	eng_style_nameLabel_9 = ui.nameLabel_9->styleSheet();
	eng_style_leftCheckBox3 = ui.leftCheckBox3->styleSheet();
	eng_style_rightCheckBox3 = ui.rightCheckBox3->styleSheet();
	eng_style_nameLabel_11 = ui.nameLabel_11->styleSheet();
	eng_style_leftCheckBox4 = ui.leftCheckBox4->styleSheet();
	eng_style_rightCheckBox4 = ui.rightCheckBox4->styleSheet();
	eng_style_label_7 = ui.label_7->styleSheet();
	eng_style_label_17 = ui.label_17->styleSheet();

	eng_style_label_6 = ui.label_6->styleSheet();
	eng_style_label = ui.label->styleSheet();
	eng_style_radioButton = ui.radioButton->styleSheet();
	eng_style_label_18 = ui.label_18->styleSheet();
	eng_style_radioButton_3 = ui.radioButton_3->styleSheet();
	eng_style_label_19 = ui.label_19->styleSheet();
	eng_style_radioButton_2 = ui.radioButton_2->styleSheet();
	eng_style_label_20 = ui.label_20->styleSheet();
	eng_style_label_2 = ui.label_2->styleSheet();
	eng_style_label_8 = ui.label_8->styleSheet();
	eng_style_label_9 = ui.label_9->styleSheet();
	eng_style_label_10 = ui.label_10->styleSheet();
	eng_style_label_11 = ui.label_11->styleSheet();
	eng_style_label_12 = ui.label_12->styleSheet();
	eng_style_label_13 = ui.label_13->styleSheet();
	eng_style_label_22 = ui.label_22->styleSheet();
	eng_style_label_14 = ui.label_14->styleSheet();
	eng_style_label_16 = ui.label_16->styleSheet();
	eng_style_label_3 = ui.label_3->styleSheet();

	eng_style_label_4 = ui.label_4->styleSheet();
}

void ReportDialog::printDocument(QPrinter* printer)
{

	QWidget* w = ui.scrollAreaWidgetContents_3;
	QPainter painter(printer);

	painter.begin(printer);

	double xscale = printer->pageRect().width() / double(w->width());
	double yscale = printer->pageRect().height() / double(w->height());
	double scale = qMin(xscale, yscale);
	painter.translate(double(printer->paperRect().x() + printer->pageRect().width() / 2),
		double(printer->paperRect().y() + printer->pageRect().height() / 2));
	painter.scale(scale, scale);
	painter.translate(-w->width() / 2, -w->height() / 2);
	w->render(&painter);

	painter.end();
}