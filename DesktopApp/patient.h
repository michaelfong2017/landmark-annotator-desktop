#ifndef PATIENT_H
#define PATIENT_H

#include "stdafx.h"

enum class Sex {Male, Female, Undefined};

class Patient {
private:
	std::string name;
	std::string hkid;
	std::string phoneNumber;
	std::string email;
	std::string subjectNumber;
	std::string socialSecurityNumber;
	std::string nationality;
	std::string address;
	std::string remarks;
	Sex sex = Sex::Undefined;
	float height;
	float weight;
	QDate dob;
	bool isValid;

public:
	// Setters
	void setName(std::string);
	void setHKID(std::string);
	void setPhoneNumber(std::string);
	void setEmail(std::string);
	void setSubjectNumber(std::string);
	void setSocialSecurityNumber(std::string);
	void setNationality(std::string);
	void setAddress(std::string);
	void setRemarks(std::string);
	void setSex(Sex);
	void setHeight(float);
	void setWeight(float);
	void setDOB(QDate);
	void setValidity(bool);

	// Getters 
	std::string getName();
	std::string getHKID();
	std::string getPhoneNumber();
	std::string getEmail();
	std::string getSubjectNumber();
	std::string getSocialSecurityNumber();
	std::string getNationality();
	std::string getAddress();
	std::string getRemarks();
	Sex getSex();
	float getHeight();
	float getWeight();
	QDate getDOB();
	bool getValidity();
};
#endif
