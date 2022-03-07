#include "patient.h"

void Patient::setName(std::string name) {
	this->name = name;
}

void Patient::setHKID(std::string hkid) {
	this->hkid = hkid;
}

void Patient::setPhoneNumber(std::string phoneNumber) {
	this->phoneNumber = phoneNumber;
}

void Patient::setEmail(std::string email) {
	this->email = email;
}

void Patient::setMedicalNumber(std::string medicalNumber) {
	this->medicalNumber = medicalNumber;
}

void Patient::setNationality(std::string nationality) {
	this->nationality = nationality;
}

void Patient::setAddress(std::string address) {
	this->address = address;
}

void Patient::setSex(Sex sex) {
	this->sex = sex;
}

void Patient::setHeight(float height) {
	this->height = height;
}

void Patient::setWeight(float weight) {
	this->weight = weight;
}

void Patient::setDOB(QDate dob) {
	this->dob = dob;
}

void Patient::setValidity(bool isValid) {
	this->isValid = isValid;
}

std::string Patient::getName() {
	return this->name;
}

std::string Patient::getHKID() {
	return this->hkid;
}

std::string Patient::getPhoneNumber() {
	return this->phoneNumber;
}

std::string Patient::getEmail() {
	return this->email;
}

std::string Patient::getMedicalNumber() {
	return this->medicalNumber;
}

std::string Patient::getNationality() {
	return this->nationality;
}

std::string Patient::getAddress() {
	return this->address;
}

Sex Patient::getSex() {
	return this->sex;
}

float Patient::getHeight() {
	return this->height;
}

float Patient::getWeight() {
	return this->weight;
}

QDate Patient::getDOB() {
	return this->dob;
}

bool Patient::getValidity() {
	return this->isValid;
}
