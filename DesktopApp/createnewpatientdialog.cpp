#include "createnewpatientdialog.h"

CreateNewPatientDialog::CreateNewPatientDialog(PatientListTab* parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	this->parent = parent;

    QPushButton* b = ui.buttonBox->addButton("Add", QDialogButtonBox::AcceptRole);
    b->setDefault(false);
    b->setAutoDefault(false);
    b->setFocusPolicy(Qt::NoFocus);

    QPushButton* b2 = ui.buttonBox->addButton("", QDialogButtonBox::HelpRole);
    b2->setDefault(true);
    b2->setAutoDefault(true);
    b2->setVisible(false);

    disconnect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
        bool isPatientDataValid = true;

        std::string name, hkid, phone, email, subjectNumber, socialSecurityNumber, nationality, address;
        QDate dob;

        // Validate mandatory fields
        if ((name = ui.nameInput->text().toStdString()) != "") {
            patient.setName(name);
        }
        else isPatientDataValid = false;

        /** HKID card number seems to be the unique key of a patient in the database.
          * This also seems to correspond to the id (patientId) returned in response. */
        if ((hkid = ui.idInput->text().toStdString()) != "") {
            patient.setHKID(hkid);
        }
        else isPatientDataValid = false;

        // Parse date of birth (DOB)
        qDebug() << "ui.dobInput->selectedDate()" << ui.dobInput->selectedDate().toString("yyyy.MM.dd");
        //else isPatientDataValid = false;

        // Optional fields
        dob = ui.dobInput->selectedDate();
        patient.setDOB(dob);

        socialSecurityNumber = ui.socialSecurityInput->text().toStdString();
        patient.setSocialSecurityNumber(socialSecurityNumber);

        subjectNumber = ui.subjectNumberInput->text().toStdString();
        patient.setSubjectNumber(subjectNumber);

        phone = ui.phoneInput->text().toStdString();
        patient.setPhoneNumber(phone);

        email = ui.emailInput->text().toStdString();
        patient.setEmail(email);

        nationality = ui.nationalityInput->text().toStdString();
        patient.setNationality(nationality);

        address = ui.addressInput->toPlainText().toStdString();
        patient.setAddress(address);

        if (ui.male->isChecked()) patient.setSex(Sex::Male);
        else if (ui.female->isChecked()) patient.setSex(Sex::Female);
        else patient.setSex(Sex::Undefined);

        std::string height = ui.heightInput->text().toStdString();
        std::string weight = ui.weightInput->text().toStdString();
        if (height != "" && weight != "") {
            try {
                // Height and weight should be a number
                float parsedHeight = (float)std::stod(height, nullptr);
                float parsedWeight = (float)std::stod(weight, nullptr);
                patient.setHeight(parsedHeight);
                patient.setWeight(parsedWeight);
            }
            catch (std::exception& ia) {
                isPatientDataValid = false;
            }
        }
        else {
            patient.setHeight(0);
            patient.setWeight(0);
        }

        if (!isPatientDataValid) {
            std::string errorMessage = "Please fill in all mandatory fields.";
            ui.patientDataValidation->setText(QString::fromStdString(errorMessage));
        }
        else {
            //if (this->savePatientData()) {
            //    ui.patientDataValidation->setText("All good to go!");
            //}
            //else {
            //    ui.patientDataValidation->setText("Something went wrong while saving patient data.");
            //}

            QNetworkClient::getInstance().checkNewPatient(patient, this, SLOT(onCheckNewPatient(QNetworkReply*)));
        }

        patient.setValidity(isPatientDataValid);
	});
}

void CreateNewPatientDialog::onCheckNewPatient(QNetworkReply* reply) {
    QByteArray response_data = reply->readAll();
    reply->deleteLater();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

    QJsonArray jsonArray = jsonResponse.array();

    //qDebug() << response_data;
    //qDebug() << jsonResponse;
    //qDebug() << jsonArray;

    if (jsonArray.isEmpty()) {
        qDebug() << "Patient does not exist. Uploading patient...";
        QNetworkClient::getInstance().uploadNewPatient(patient, this, SLOT(onUploadNewPatient(QNetworkReply*)));
    }
    else {
        qDebug() << "Patient exists. No data are uploaded.";

        TwoLinesDialog dialog;
        dialog.setLine1("Patient already exists.");
        dialog.exec();

        QDialog::accept();
    }
}

void CreateNewPatientDialog::onUploadNewPatient(QNetworkReply* reply) 
{
    qDebug() << "onUploadNewPatient";
    qDebug() << reply->readAll();
    reply->deleteLater();

    QDialog::accept();
}
