#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_reportdialog.h"
#include "stdafx.h"
#include <annotatetab.h>
#include "reportdialog_eng.h"
#include "ui_reportdialog_eng.h"

class ReportDialog : public QDialog
{
	Q_OBJECT

public:
	ReportDialog(AnnotateTab* parent = nullptr);
	Ui::ReportDialogUI ui;
    void printWithPreview();
	QImage ReportImage;

private:
	AnnotateTab* parent;

	/**** chi text */
	QString chi_TitleLabel;
	QString chi_label_15;
	QString chi_nameLabel_10;
	QString chi_leftCheckBox1;
	QString chi_rightCheckBox1;
	QString chi_nameLabel_8;
	QString chi_leftCheckBox2;
	QString chi_rightCheckBox2;
	QString chi_nameLabel_9;
	QString chi_leftCheckBox3;
	QString chi_rightCheckBox3;
	QString chi_nameLabel_11;
	QString chi_leftCheckBox4;
	QString chi_rightCheckBox4;
	QString chi_label_7;
	QString chi_label_17;

	QString chi_label_6;
	QString chi_label;
	QString chi_radioButton;
	QString chi_label_18;
	QString chi_radioButton_3;
	QString chi_label_19;
	QString chi_radioButton_2;
	QString chi_label_20;
	QString chi_label_2;
	QString chi_label_8;
	QString chi_label_9;
	QString chi_label_10;
	QString chi_label_11;
	QString chi_label_12;
	QString chi_label_13;
	QString chi_label_22;
	QString chi_label_14;
	QString chi_label_16;
	QString chi_label_3;

	QString chi_label_4;

	/**** chi style */
	QString chi_style_TitleLabel;
	QString chi_style_label_15;
	QString chi_style_nameLabel_10;
	QString chi_style_leftCheckBox1;
	QString chi_style_rightCheckBox1;
	QString chi_style_nameLabel_8;
	QString chi_style_leftCheckBox2;
	QString chi_style_rightCheckBox2;
	QString chi_style_nameLabel_9;
	QString chi_style_leftCheckBox3;
	QString chi_style_rightCheckBox3;
	QString chi_style_nameLabel_11;
	QString chi_style_leftCheckBox4;
	QString chi_style_rightCheckBox4;
	QString chi_style_label_7;
	QString chi_style_label_17;

	QString chi_style_label_6;
	QString chi_style_label;
	QString chi_style_radioButton;
	QString chi_style_label_18;
	QString chi_style_radioButton_3;
	QString chi_style_label_19;
	QString chi_style_radioButton_2;
	QString chi_style_label_20;
	QString chi_style_label_2;
	QString chi_style_label_8;
	QString chi_style_label_9;
	QString chi_style_label_10;
	QString chi_style_label_11;
	QString chi_style_label_12;
	QString chi_style_label_13;
	QString chi_style_label_22;
	QString chi_style_label_14;
	QString chi_style_label_16;
	QString chi_style_label_3;

	QString chi_style_label_4;

	/**** eng text */
	QString eng_TitleLabel;
	QString eng_label_15;
	QString eng_nameLabel_10;
	QString eng_leftCheckBox1;
	QString eng_rightCheckBox1;
	QString eng_nameLabel_8;
	QString eng_leftCheckBox2;
	QString eng_rightCheckBox2;
	QString eng_nameLabel_9;
	QString eng_leftCheckBox3;
	QString eng_rightCheckBox3;
	QString eng_nameLabel_11;
	QString eng_leftCheckBox4;
	QString eng_rightCheckBox4;
	QString eng_label_7;
	QString eng_label_17;

	QString eng_label_6;
	QString eng_label;
	QString eng_radioButton;
	QString eng_label_18;
	QString eng_radioButton_3;
	QString eng_label_19;
	QString eng_radioButton_2;
	QString eng_label_20;
	QString eng_label_2;
	QString eng_label_8;
	QString eng_label_9;
	QString eng_label_10;
	QString eng_label_11;
	QString eng_label_12;
	QString eng_label_13;
	QString eng_label_22;
	QString eng_label_14;
	QString eng_label_16;
	QString eng_label_3;

	QString eng_label_4;

	/**** eng style */
	QString eng_style_TitleLabel;
	QString eng_style_label_15;
	QString eng_style_nameLabel_10;
	QString eng_style_leftCheckBox1;
	QString eng_style_rightCheckBox1;
	QString eng_style_nameLabel_8;
	QString eng_style_leftCheckBox2;
	QString eng_style_rightCheckBox2;
	QString eng_style_nameLabel_9;
	QString eng_style_leftCheckBox3;
	QString eng_style_rightCheckBox3;
	QString eng_style_nameLabel_11;
	QString eng_style_leftCheckBox4;
	QString eng_style_rightCheckBox4;
	QString eng_style_label_7;
	QString eng_style_label_17;

	QString eng_style_label_6;
	QString eng_style_label;
	QString eng_style_radioButton;
	QString eng_style_label_18;
	QString eng_style_radioButton_3;
	QString eng_style_label_19;
	QString eng_style_radioButton_2;
	QString eng_style_label_20;
	QString eng_style_label_2;
	QString eng_style_label_8;
	QString eng_style_label_9;
	QString eng_style_label_10;
	QString eng_style_label_11;
	QString eng_style_label_12;
	QString eng_style_label_13;
	QString eng_style_label_22;
	QString eng_style_label_14;
	QString eng_style_label_16;
	QString eng_style_label_3;

	QString eng_style_label_4;
	/*****/

	void setAllChiLabelsFromUI();
	void setAllEngLabelsFromAnotherUI();
	

protected:
	//virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

signals:

private slots:
	void printDocument(QPrinter* printer);

};

#endif
