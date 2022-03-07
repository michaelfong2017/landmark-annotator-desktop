#include "patientdatatab.h"

PatientDataTab::PatientDataTab(DesktopApp* parent) {
    this->parent = parent;

    QObject::connect(parent->ui.savePatientButton, &QPushButton::clicked, [this]() {

        bool isPatientDataValid = true;
        
        std::string name, hkid, phone, email, medicalNumber, nationality, address;

        // Validate mandatory fields
        if ((name = this->parent->ui.nameInput->toPlainText().toStdString()) != "") {
            this->parent->patient.setName(name);
        }
        else isPatientDataValid = false;

        if ((medicalNumber = this->parent->ui.medicalInput->toPlainText().toStdString()) != "") {
            this->parent->patient.setMedicalNumber(medicalNumber);
        }
        else isPatientDataValid = false;

        if ((phone = this->parent->ui.phoneInput->toPlainText().toStdString()) != "") {
            this->parent->patient.setPhoneNumber(phone);
        }
        else isPatientDataValid = false;

        // Optional fields
        hkid = this->parent->ui.idInput->toPlainText().toStdString();
        this->parent->patient.setHKID(hkid);

        email = this->parent->ui.emailInput->toPlainText().toStdString();
        this->parent->patient.setEmail(email);

        nationality = this->parent->ui.nationalityInput->toPlainText().toStdString();
        this->parent->patient.setNationality(nationality);

        address = this->parent->ui.addressInput->toPlainText().toStdString();
        this->parent->patient.setAddress(address);

        if (this->parent->ui.male->isChecked()) this->parent->patient.setSex(Sex::Male);
        else if (this->parent->ui.female->isChecked()) this->parent->patient.setSex(Sex::Female);
        else this->parent->patient.setSex(Sex::Undefined);

        std::string height = this->parent->ui.heightInput->toPlainText().toStdString();
        std::string weight = this->parent->ui.weightInput->toPlainText().toStdString();
        if (height != "" && weight != "") {
            try {
                // Height and weight should be a number
                float parsedHeight = (float) std::stod(height, nullptr);
                float parsedWeight = (float) std::stod(weight, nullptr);
                this->parent->patient.setHeight(parsedHeight);
                this->parent->patient.setWeight(parsedWeight);

                // Parse date of birth (DOB)
                this->parent->patient.setDOB(this->parent->ui.dobInput->selectedDate());
            }
            catch (std::exception& ia) {
                isPatientDataValid = false;
            }
        }
        else {
            this->parent->patient.setHeight(0);
            this->parent->patient.setWeight(0);
        }

        if (!isPatientDataValid) {
            std::string errorMessage = "Please fill in all mandatory fields.\n Height and weight should be a number";
            this->parent->ui.patientDataValidation->setText(QString::fromStdString(errorMessage));
        }
        else {
            if (this->savePatientData()) {
                this->parent->ui.patientDataValidation->setText("All good to go!");
            }
            else {
                this->parent->ui.patientDataValidation->setText("Something went wrong while saving patient data.");
            }
        }

        this->parent->patient.setValidity(isPatientDataValid);
    });


    QObject::connect(parent->ui.loadPatientButton, &QPushButton::clicked, [this]() {
        // Reset all fields on the form
        this->parent->ui.nameInput->setText("");
        this->parent->ui.idInput->setText("");
        this->parent->ui.phoneInput->setText("");
        this->parent->ui.emailInput->setText("");
        this->parent->ui.medicalInput->setText("");
        this->parent->ui.nationalityInput->setText("");
        this->parent->ui.addressInput->setText("");
        this->parent->ui.heightInput->setText("");
        this->parent->ui.weightInput->setText("");
        this->parent->ui.male->setChecked(false);
        this->parent->ui.female->setChecked(false);

        QString fileName = QFileDialog::getOpenFileName(this, tr("Load Patient Info"), QString(), tr("Text (*.txt)"));

        QFile file(fileName);
        file.open(QIODevice::ReadOnly);

        QTextStream in(&file);

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList list = line.split(":");

            if (list.length() > 1) {
                QString key = list[0], value = list[1].trimmed();

                if (key == "Full name") this->parent->ui.nameInput->setText(value);
                else if (key == "HKID") this->parent->ui.idInput->setText(value);
                else if (key == "Phone number") this->parent->ui.phoneInput->setText(value);
                else if (key == "Email") this->parent->ui.emailInput->setText(value);
                else if (key == "Medical number") this->parent->ui.medicalInput->setText(value);
                else if (key == "Nationality") this->parent->ui.nationalityInput->setText(value);
                else if (key == "Address") this->parent->ui.addressInput->setText(value);
                else if (key == "Height") this->parent->ui.heightInput->setText(value);
                else if (key == "Weight") this->parent->ui.weightInput->setText(value);
                else if (key == "Sex") {
                    if (value == "Male") this->parent->ui.male->setChecked(true);
                    else if (value == "Female") this->parent->ui.female->setChecked(true);
                } 
                else if (key == "Date of birth") {
                    qDebug() << value;
                    this->parent->ui.dobInput->setSelectedDate(QDate::fromString(value, DATE_FORMAT));
                }
            }
        }

        file.close();

        this->parent->savePath = QFileInfo(fileName).dir();
    });
}

bool PatientDataTab::savePatientData() {
    QString defaultSavePath = this->parent->savePath.absolutePath() + "/" + QString::fromStdString(this->parent->patient.getMedicalNumber());
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save patient data"), defaultSavePath, tr("Text (*.txt)"));

    if (!fileName.isEmpty()) {
        QDir savePath = QFileInfo(fileName).dir();

        if (savePath.dirName().toStdString() == this->parent->patient.getMedicalNumber()) {
            // If selected directory name is a medical number folder created prior
            this->parent->savePath = savePath;
        }
        else {
            // If medical number folder doesn't exist, create a new one first
            if (!savePath.exists(QString::fromStdString(this->parent->patient.getMedicalNumber()))) {
                savePath.mkdir(QString::fromStdString(this->parent->patient.getMedicalNumber()));
            }

            // Get into the medical number folder
            savePath.cd(QString::fromStdString(this->parent->patient.getMedicalNumber()));

            this->parent->savePath = savePath;
        }

        // The new filename is savePath + filename
        fileName = this->parent->savePath.absolutePath() + "/" + QFileInfo(fileName).fileName();

        QFile file(fileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);

        QTextStream out(&file);
        out << "Full name: " << QString::fromStdString(this->parent->patient.getName()) << "\n";
        out << "Medical number: " << QString::fromStdString(this->parent->patient.getMedicalNumber()) << "\n";
        out << "Phone number: " << QString::fromStdString(this->parent->patient.getPhoneNumber()) << "\n";
        out << "HKID: " << QString::fromStdString(this->parent->patient.getHKID()) << "\n";
        out << "Email: " << QString::fromStdString(this->parent->patient.getEmail()) << "\n";
        out << "Date of birth: " << this->parent->patient.getDOB().toString(DATE_FORMAT) << "\n";
        out << "Nationality: " << QString::fromStdString(this->parent->patient.getNationality()) << "\n";
        out << "Address: " << QString::fromStdString(this->parent->patient.getAddress()) << "\n";

        std::string sex;
        switch (this->parent->patient.getSex()) {
        case Sex::Male:
            sex = "Male";
            break;
        case Sex::Female:
            sex = "Female";
            break;
        default:
            sex = "";
            break;
        }

        out << "Sex: " << QString::fromStdString(sex) << "\n";

        float height = this->parent->patient.getHeight();
        out << "Height: " << QString::fromStdString(std::to_string(height)) << "\n";

        float weight = this->parent->patient.getWeight();
        out << "Weight: " << QString::fromStdString(std::to_string(weight)) << "\n";

        file.close();

        return true;
    }
    else return false;
}
