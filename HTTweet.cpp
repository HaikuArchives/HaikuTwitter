/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTTweet.h"

HTTweet::HTTweet()
{
	imageBitmap = NULL;
	date.day = 1;
	date.month = 0;
	date.year = 1970;
	date.hour = 0;
	date.minute = 0;
	date.second = 0;
	id = string("");
	sourceName = string("");
	bitmapDownloadInProgress = false;
	view = NULL;
	isFollowing = false;
	this->setDate(*new string(" "));
}

HTTweet::HTTweet(string &screenName, string &text, string &profileImageUrl, string &dateString)
{
	HTTweet::HTTweet();
	this->screenName = screenName;
	this->text = text;
	this->profileImageUrl = profileImageUrl;
	this->setDate(dateString);
	this->rawDate = dateString;
	imageBitmap = NULL;
	id = string("");
	sourceName = string("");
	bitmapDownloadInProgress = false;
	view = NULL;
	isFollowing = false;
}

HTTweet::HTTweet(HTTweet *originalTweet)
{
	HTTweet::HTTweet();
	this->screenName = originalTweet->getScreenName();
	this->fullName = originalTweet->getFullName();
	this->text = originalTweet->getText();
	this->profileImageUrl = originalTweet->getProfileImageUrl();
	if (originalTweet->imageBitmap != NULL)
		if(originalTweet->imageBitmap->IsValid()) //Not interested in corrupted data.
			this->imageBitmap = new BBitmap(*originalTweet->getBitmap());
	this->date = originalTweet->getDate();
	this->id = string(originalTweet->getId());
	this->sourceName = originalTweet->getSourceName();
	this->view = originalTweet->getView();
	bitmapDownloadInProgress = false;
	this->isFollowing = originalTweet->following();
}

BView*
HTTweet::getView()
{
	return view;
}

const string
HTTweet::getRawDate()
{
	return rawDate;
}

const string
HTTweet::getScreenName()
{
	return screenName;	
}

//Note: Real name is unreliable, force screen name (Issue 11)
const string
HTTweet::getFullName()
{
	//if(fullName.length() > 0) //Return screenname if fullname is missing
		return fullName;
	//else
	//	return screenName;
}

const string
HTTweet::getSourceName()
{
	return sourceName;
}

const string
HTTweet::getText()
{
	return text;	
}

const string
HTTweet::getProfileImageUrl()
{
	return profileImageUrl;	
}

void
HTTweet::setScreenName(string &screenName)
{
	this->screenName = screenName;
}

void
HTTweet::setFullName(string &fullName)
{
	this->fullName = fullName;
}

void
HTTweet::setSourceName(string &sourceName)
{
	this->sourceName = sourceName;
}

void
HTTweet::setText(string &text)
{
	this->text = text;
}

void
HTTweet::setView(BView* view)
{
	this->view = view;
}

void
HTTweet::setDate(string &dateString)
{
	if(dateString.length() < 29) { //29
		#ifdef DEBUG_ENABLED
		std::cout << "HTTweet::setDate(string &): Got invalid date string" << std::endl;
		#endif
		dateString = std::string("Wed Jan 01 00:00:00 +0000 1970");
	}
	
	this->rawDate = dateString;
	const char *cString = dateString.c_str();
	date.month = stringToMonth(cString);
	
	// The number 1 is represented by ASCII code 49.
	// We substract every char by 48 to make it int.
	date.day = cString[9]-48 + (cString[8]-48)*10;
	date.hour = cString[12]-48 + (cString[11]-48)*10;;
	date.minute = cString[15]-48 + (cString[14]-48)*10;
	date.second = cString[18]-48 + (cString[17]-48)*10;
	date.year = cString[29]-48 + (cString[28]-48)*10 + (cString[27]-48)*100 + (cString[26]-48)*1000;
}

void
HTTweet::setDate(time_t unixTime)
{	
	struct tm* timeinfo;

	timeinfo = localtime(&unixTime);
	date.year = timeinfo->tm_year +1900;
	date.month = timeinfo->tm_mon;
	date.day = timeinfo->tm_mday;
	date.hour = timeinfo->tm_hour;
	date.minute = timeinfo->tm_min;
}

void
HTTweet::setPublishedDate(string &dateString)
{
	if(dateString.length() < 17) {
		#ifdef DEBUG_ENABLED
		std::cout << "HTTweet::setPublishedDate(string &): Got invalid date string" << std::endl;
		#endif
		dateString = std::string("1970-01-01T00:00:00Z");
	}
	
	this->rawDate = dateString;
	const char *cString = dateString.c_str();
	
	// The number 1 is represented by ASCII code 49.
	// We substract every char by 48 to make it int.
	date.day = cString[9]-48 + (cString[8]-48)*10;
	date.month = (cString[6]-48 + (cString[5]-48)*10) -1; //tm_mon (0-11)
	date.hour = cString[12]-48 + (cString[11]-48)*10;;
	date.minute = cString[15]-48 + (cString[14]-48)*10;
	date.second = cString[18]-48 + (cString[17]-48)*10;
	date.year = cString[3]-48 + (cString[2]-48)*10 + (cString[1]-48)*100 + (cString[0]-48)*1000;
}

struct DateStruct
HTTweet::getDate() const
{
	return date;
}

time_t
HTTweet::getUnixTime() const
{
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	timeinfo->tm_year = date.year -1900;
	timeinfo->tm_mon = date.month;
	timeinfo->tm_mday = date.day;
	timeinfo->tm_hour = date.hour;
	timeinfo->tm_min = date.minute;
		
	return mktime(timeinfo);
}

const string
HTTweet::getRelativeDate()
{
	time_t currentTime = time(NULL);
	tm *timeinfo;
	timeinfo = gmtime(&currentTime);
	char tempString[100];
	
	int diffYear = timeinfo->tm_year-this->getDate().year +1900;
	int diffHour = timeinfo->tm_hour-this->getDate().hour;
	int diffDay = timeinfo->tm_mday-this->getDate().day;
	int diffMin = timeinfo->tm_min-this->getDate().minute;
	int diffMonth = timeinfo->tm_mon-this->getDate().month;
		
	//Special cases
	if(diffMin > 45)
		diffHour++;
	
	if(diffYear > 1)
		sprintf(tempString, "%i years ago", diffYear);
	else if(diffYear == 1)
		sprintf(tempString, "Last year");
	else if(diffMonth > 0) {
		sprintf(tempString, "%i %s", this->getDate().day, monthToString(this->getDate().month));
	}
	else if(diffDay > 1)
		sprintf(tempString, "%i days ago", diffDay);
	else if(diffDay == 1)
		sprintf(tempString, "Yesterday");
	else if(diffHour == 1)
		sprintf(tempString, "Last hour");
	else if(diffHour > 1)
		sprintf(tempString, "%i hours ago", diffHour);
	else if(diffMin > 1)
		sprintf(tempString, "%i minutes ago", diffMin);
	else
		sprintf(tempString, "Moments ago");
		
	return std::string(tempString);
}

bool
HTTweet::operator<(const HTTweet &b) const
{
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

const char*
HTTweet::getId()
{
	return id.c_str();
}

void
HTTweet::setId(const char* id)
{
	string idClean(id);
	int pos = idClean.find(" ");
	if(pos != std::string::npos)
		idClean.resize(pos-1);
	this->id = idClean;
}

bool
HTTweet::following()
{
	return isFollowing;
}

void
HTTweet::setFollowing(bool isFollowing)
{
	this->isFollowing = isFollowing;
}

const int
HTTweet::stringToMonth(const char *date)
{
	//jan feb mar apr may jun jul aug sep oct nov dec
	if(strncmp(date+4, "Jan", 3) == 0)
		return 0;
	if(strncmp(date+4, "Feb", 3) == 0)
		return 1;
	if(strncmp(date+4, "Mar", 3) == 0)
		return 2;
	if(strncmp(date+4, "Apr", 3) == 0)
		return 3;
	if(strncmp(date+4, "May", 3) == 0)
		return 4;
	if(strncmp(date+4, "Jun", 3) == 0)
		return 5;
	if(strncmp(date+4, "Jul", 3) == 0)
		return 6;
	if(strncmp(date+4, "Aug", 3) == 0)
		return 7;
	if(strncmp(date+4, "Sep", 3) == 0)
		return 8;
	if(strncmp(date+4, "Oct", 3) == 0)
		return 9;
	if(strncmp(date+4, "Nov", 3) == 0)
		return 10;
	if(strncmp(date+4, "Dec", 3) == 0)
		return 11;
	#ifdef DEBUG_ENABLED
	std::cout << "HTTweet::stringToMonth: Failed" << std::endl;
	#endif
	return 0;
}

const char*
HTTweet::monthToString(int month)
{
	//jan feb mar apr may jun jul aug sep oct nov dec
	switch(month) {
		case 0:
			return "Jan";
		case 1:
			return "Feb";
		case 2:
			return "Mar";
		case 3:
			return "Apr";
		case 4:
			return "May";
		case 5:
			return "Jun";
		case 6:
			return "Jul";
		case 7:
			return "Aug";
		case 8:
			return "Sep";
		case 9:
			return "Oct";
		case 10:
			return "Nov";
		case 11:
			return "Dec";
		default:
			#ifdef DEBUG_ENABLED
			std::cout << "HTTweet::stringToMonth: Failed" << std::endl;
			#endif
			return "Jan";
	}
	
	
	//return "Jan";
}

void
HTTweet::setProfileImageUrl(string &profileImageUrl)
{
	this->profileImageUrl = profileImageUrl;
}

BBitmap*
HTTweet::getBitmap()
{
	return imageBitmap;
}

void
HTTweet::setBitmap(BBitmap *bitmap)
{
	imageBitmap = bitmap;
}

BBitmap
HTTweet::getBitmapCopy()
{
	return *imageBitmap;
}

void
HTTweet::downloadBitmap()
{
	if(profileImageUrl.find("error") != std::string::npos)
		imageBitmap = defaultBitmap();
	else if(!bitmapDownloadInProgress) {
		downloadThread = spawn_thread(_threadDownloadBitmap, profileImageUrl.c_str(), 10, this);
		resume_thread(downloadThread);
	}
}

status_t
_threadDownloadBitmap(void *data)
{
	HTTweet *super = (HTTweet*)data;
	super->bitmapDownloadInProgress = true;
	CURL *curl_handle;
	BMallocIO *mallocIO = new BMallocIO();
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, super->getProfileImageUrl().c_str());
	
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
	if(super->getView() != NULL && super->getView()->LockLooper()) {
			super->setBitmap(BTranslationUtils::GetBitmap(mallocIO));
			super->bitmapDownloadInProgress = false;
			super->getView()->Invalidate();
			super->getView()->UnlockLooper();
	}else {
		super->setBitmap(BTranslationUtils::GetBitmap(mallocIO));
		super->bitmapDownloadInProgress = false;
	}
	
	/*Delete the buffer*/
	delete mallocIO;
	
	return B_OK;
}

bool
HTTweet::isDownloadingBitmap()
{
	return bitmapDownloadInProgress;
}

void
HTTweet::waitUntilDownloadComplete()
{
	status_t junkId;
	wait_for_thread(downloadThread, &junkId);
}

HTTweet::~HTTweet()
{
	/*Kill the download thread*/
	if(bitmapDownloadInProgress)
		kill_thread(downloadThread);
	
	if(imageBitmap != NULL)
		delete imageBitmap;
	imageBitmap == NULL;
}

/*Callback function for cURL (userIcon download)*/
static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size *nmemb;
	BMallocIO *mallocIO = (BMallocIO *)data;
	
	return mallocIO->Write(ptr, realsize);
}

BBitmap*
defaultBitmap()
{
	return BTranslationUtils::GetBitmap(B_VECTOR_ICON_TYPE, "twitter_icon");
}
