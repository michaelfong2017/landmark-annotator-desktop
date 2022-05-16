#include "createnewpatientdialog.h"

CreateNewPatientDialog::CreateNewPatientDialog(PatientListTab* parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	this->parent = parent;

	ui.buttonBox->addButton("Add", QDialogButtonBox::AcceptRole);

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
		Patient patient;
        bool isPatientDataValid = true;

        std::string name, hkid, phone, email, subjectNumber, socialSecurityNumber, nationality, address;

        // Validate mandatory fields
        if ((name = ui.nameInput->toPlainText().toStdString()) != "") {
            patient.setName(name);
        }
        else isPatientDataValid = false;

        /** HKID card number seems to be the unique key of a patient in the database.
          * This also seems to correspond to the id (patientId) returned in response. */
        if ((hkid = ui.idInput->toPlainText().toStdString()) != "") {
            patient.setHKID(hkid);
        }
        else isPatientDataValid = false;

        // Parse date of birth (DOB)
        qDebug() << ui.dobInput->selectedDate();
        //else isPatientDataValid = false;

        // Optional fields
        socialSecurityNumber = ui.socialSecurityInput->toPlainText().toStdString();
        patient.setSocialSecurityNumber(socialSecurityNumber);

        subjectNumber = ui.subjectNumberInput->toPlainText().toStdString();
        patient.setSubjectNumber(subjectNumber);

        phone = ui.phoneInput->toPlainText().toStdString();
        patient.setPhoneNumber(phone);

        email = ui.emailInput->toPlainText().toStdString();
        patient.setEmail(email);

        nationality = ui.nationalityInput->toPlainText().toStdString();
        patient.setNationality(nationality);

        address = ui.addressInput->toPlainText().toStdString();
        patient.setAddress(address);

        if (ui.male->isChecked()) patient.setSex(Sex::Male);
        else if (ui.female->isChecked()) patient.setSex(Sex::Female);
        else patient.setSex(Sex::Undefined);

        std::string height = ui.heightInput->toPlainText().toStdString();
        std::string weight = ui.weightInput->toPlainText().toStdString();
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
            std::string errorMessage = "Please fill in all mandatory fields.\n Height and weight should be a number";
            ui.patientDataValidation->setText(QString::fromStdString(errorMessage));
        }
        else {
            //if (this->savePatientData()) {
            //    ui.patientDataValidation->setText("All good to go!");
            //}
            //else {
            //    ui.patientDataValidation->setText("Something went wrong while saving patient data.");
            //}

            QNetworkClient::getInstance().uploadNewPatient(patient, this, SLOT(onUploadNewPatient(QNetworkReply*)));
        }

        patient.setValidity(isPatientDataValid);
	});
}

void CreateNewPatientDialog::onUploadNewPatient(QNetworkReply* reply) {
    qDebug() << reply->readAll();
    reply->deleteLater();

    QDialog::accept();
}
