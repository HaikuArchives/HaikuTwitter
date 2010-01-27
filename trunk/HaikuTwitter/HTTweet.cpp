/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTTweet.h"

HTTweet::HTTweet() {
	imageBitmap = NULL;
	date.day = -1;
	date.day = -1;
	date.day = -1;
	date.day = -1;
	id = -1;
}

HTTweet::HTTweet(string &screenName, string &text, string &profileImageUrl, string &dateString) {
	this->screenName = screenName;
	this->text = text;
	this->profileImageUrl = profileImageUrl;
	this->setDate(dateString);
	imageBitmap = NULL;
	id = -1;
}

HTTweet::HTTweet(HTTweet *originalTweet) {
	this->screenName = originalTweet->getScreenName();
	this->text = originalTweet->getText();
	this->profileImageUrl = originalTweet->getProfileImageUrl();
	if (originalTweet->imageBitmap != NULL)
		if(originalTweet->imageBitmap->IsValid()) //Not interested in corrupted data.
			this->imageBitmap = new BBitmap(*originalTweet->getBitmap());
	this->date = originalTweet->getDate();
	this->id = originalTweet->getId();
}

const string HTTweet::getScreenName() {
	return screenName;	
}

const string HTTweet::getText() {
	return text;	
}

const string HTTweet::getProfileImageUrl() {
	return profileImageUrl;	
}

void HTTweet::setScreenName(string &screenName) {
	this->screenName = screenName;
}

void HTTweet::setText(string &text) {
	this->text = text;
}

void HTTweet::setDate(string &dateString) {
	const char *cString = dateString.c_str();
	
	date.month = stringToMonth(cString);
	
	// The number 1 is represented by ASCII code 49.
	// We substract every char by 48 to make it int.
	date.day = cString[9]-48 + (cString[8]-48)*10;
	date.hour = cString[12]-48 + (cString[11]-48)*10;;
	date.minute = cString[15]-48 + (cString[14]-48)*10;
	date.second = cString[18]-48 + (cString[17]-48)*10;
}

struct DateStruct HTTweet::getDate() const {
	return date;
}

bool HTTweet::operator<(const HTTweet &b) const {
	struct DateStruct bDate = b.getDate();
	struct DateStruct aDate = this->getDate();
	if(aDate.month < bDate.month)
		return true;
	if(aDate.month > bDate.month)
		return false;
	if(aDate.day < bDate.day)
		return true;
	if(aDate.day > bDate.day)
		return false;
	if(aDate.hour < bDate.hour)
		return true;
	if(aDate.hour > bDate.hour)
		return false;
	if(aDate.minute < bDate.minute)
		return true;
	if(aDate.minute > bDate.minute)
		return false;
	if(aDate.second < bDate.second)
		return true;
	return false;	
}

int HTTweet::getId() {
	return id;
}

void HTTweet::setId(int id) {
	this->id = id;
}

const int HTTweet::stringToMonth(const char *date) {
	//jan feb mar apr may jun jul aug sep oct nov dec
	if(strncmp(date+4, "Jan", 3) == 0)
		return 1;
	if(strncmp(date+4, "Feb", 3) == 0)
		return 2;
	if(strncmp(date+4, "Mar", 3) == 0)
		return 3;
	if(strncmp(date+4, "Apr", 3) == 0)
		return 4;
	if(strncmp(date+4, "May", 3) == 0)
		return 5;
	if(strncmp(date+4, "Jun", 3) == 0)
		return 6;
	if(strncmp(date+4, "Jul", 3) == 0)
		return 7;
	if(strncmp(date+4, "Aug", 3) == 0)
		return 8;
	if(strncmp(date+4, "Sep", 3) == 0)
		return 9;
	if(strncmp(date+4, "Oct", 3) == 0)
		return 10;
	if(strncmp(date+4, "Nov", 3) == 0)
		return 11;
	if(strncmp(date+4, "Dec", 3) == 0)
		return 12;
	return -1;
}

void HTTweet::setProfileImageUrl(string &profileImageUrl) {
	this->profileImageUrl = profileImageUrl;
}

BBitmap* HTTweet::getBitmap() {
	return imageBitmap;
}

BBitmap HTTweet::getBitmapCopy() {
	return *imageBitmap;
}

void HTTweet::downloadBitmap() {
	this->downloadBitmap(profileImageUrl.c_str());
}

void HTTweet::downloadBitmap(const char *url) {
	CURL *curl_handle;
	BMallocIO *mallocIO = new BMallocIO();
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	
	/*send all data to this function*/
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	
	/*we pass out 'mallocIO' object to the callback function*/
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)mallocIO);
	
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "haikutwitter-agent/1.0");
	
	/*get the data*/
	if(curl_easy_perform(curl_handle) < 0)
		std::cout << "libcURL: Download of bitmap failed." << std::endl;
	
	/*cleanup curl stuff*/
	curl_easy_cleanup(curl_handle);
	
	/*Translate downloaded data to bitmap*/
	imageBitmap = BTranslationUtils::GetBitmap(mallocIO);
	
	/*Delete the buffer*/
	delete mallocIO;
}

HTTweet::~HTTweet() {
	if(imageBitmap != NULL) {
		delete imageBitmap;
	}
}

/*Callback function for cURL (userIcon download)*/
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
	size_t realsize = size *nmemb;
	BMallocIO *mallocIO = (BMallocIO *)data;
	
	return mallocIO->Write(ptr, realsize);
}
