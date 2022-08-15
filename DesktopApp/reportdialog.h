#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_reportdialog.h"
#include "stdafx.h"
#include <annotatetab.h>

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

protected:
	//virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

signals:

private slots:
	void printDocument(QPrinter* printer);

};

#endif
