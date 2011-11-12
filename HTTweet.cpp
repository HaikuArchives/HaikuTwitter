/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTTweet.h"
#include "HTStorage.h"

HTTweet::HTTweet()
{
	bitmapData = NULL;
	imageBitmap = NULL;
	date.day = 1;
	date.month = 0;
	date.year = 1970;
	date.hour = 0;
	date.minute = 0;
	date.second = 0;
	id = 0;
	retweetedBy = string("");
	sourceName = string("");
	bitmapDownloadInProgress = false;
	view = NULL;
	isFollowing = false;
}

HTTweet::HTTweet(HTTweet *originalTweet)
{
	bitmapData = NULL;
	imageBitmap = NULL;
	this->retweetedBy = originalTweet->getRetweetedBy();
	this->screenName = originalTweet->getScreenName();
	this->fullName = originalTweet->getFullName();
	this->text = originalTweet->getText();
	this->profileImageUrl = originalTweet->getProfileImageUrl();
	if (originalTweet->imageBitmap != NULL)
		if(originalTweet->imageBitmap->IsValid()) //Not interested in corrupted data.
			this->imageBitmap = new BBitmap(*originalTweet->getBitmap());
	this->setDate(originalTweet->getUnixTime());
	this->id = originalTweet->getId();
	this->sourceName = originalTweet->getSourceName();
	this->view = originalTweet->getView();
	bitmapDownloadInProgress = false;
	this->isFollowing = originalTweet->following();
}

HTTweet::HTTweet(BMessage* archive)
	: BArchivable(archive)
{
	bitmapData = NULL;
	/*Unarchive ivars*/
	char buff[255];
	archive->FindBool("HTTweet::bitmapDownloadInProgress", &bitmapDownloadInProgress);
	archive->FindString("HTTweet::screenName", *buff);
	screenName = std::string(buff);
	archive->FindString("HTTweet::fullName", *buff);
	fullName = std::string(buff);
	archive->FindString("HTTweet::text", *buff);
	text = std::string(buff);
	archive->FindString("HTTweet::profileImageUrl", *buff);
	profileImageUrl = std::string(buff);
	archive->FindString("HTTweet::sourceName", *buff);
	sourceName = std::string(buff);
	
	archive->FindUInt64("HTTweet::id", &id);
	
	archive->FindBool("HTTweet::isFollowing", &isFollowing);
	
	/*Unarchive profileImage*/
	BMessage msg;
	BArchivable* unarchived;
	if(archive->FindMessage("HTTweet::imageBitmap", &msg) == B_OK) {
		unarchived = instantiate_object(&msg);
		if(unarchived)
			imageBitmap = dynamic_cast<BBitmap *>(unarchived);
		else {
			std::cout << "Unable to instantiate archived <HTGTimeLineView::imageBitmap>." << std::endl;
			imageBitmap = NULL;
			downloadBitmap();
		}
	}
	
	view = NULL;
}

BArchivable*
HTTweet::Instantiate(BMessage* archive)
{
	if(validate_instantiation(archive, "HTTweet"))
		return new HTTweet(archive);
	return NULL;
}

status_t
HTTweet::Archive(BMessage* archive, bool deep)
{
	BArchivable::Archive(archive, deep);
	
	archive->AddString("class", "HTTweet");
	
	/*Kill the download thread*/
	if(bitmapDownloadInProgress)
		kill_thread(downloadThread);
	if(bitmapDownloadInProgress)
		imageBitmap = NULL;
	
	/*Archive ivars*/
	archive->AddBool("HTTweet::bitmapDownloadInProgress", bitmapDownloadInProgress);
	archive->AddString("HTTweet::screenName", screenName.c_str());
	archive->AddString("HTTweet::fullName", fullName.c_str());
	archive->AddString("HTTweet::text", text.c_str());
	archive->AddString("HTTweet::profileImageUrl", profileImageUrl.c_str());
	archive->AddString("HTTweet::sourceName", sourceName.c_str());
	archive->AddUInt64("HTTweet::id", id);
	archive->AddBool("HTTweet::isFollowing", isFollowing);
	
	/*Archive profileImage*/
	if(imageBitmap != NULL) {
		BMessage msg;
		imageBitmap->Archive(&msg, deep);
		archive->AddMessage("HTTweet::imageBitmap", &msg);
	}
	
	/*Resume download*/
	if(bitmapDownloadInProgress)
		downloadBitmap();
	
	return B_OK;
}

BView*
HTTweet::getView()
{
	return view;
}

const string
HTTweet::getRetweetedBy() const
{
	return retweetedBy;
}

const string
HTTweet::getScreenName() const
{
	return screenName;	
}

const string
HTTweet::getFullName()
{
	if(fullName.length() > 0) //Return screenname if fullname is missing
		return fullName;
	else
		return screenName;
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
HTTweet::setScreenName(string screenName)
{
	this->screenName = screenName;
}

void
HTTweet::setRetweetedBy(string retweetedBy)
{
	this->retweetedBy = retweetedBy;
}

void
HTTweet::setFullName(string fullName)
{
	this->fullName = fullName;
}

void
HTTweet::setSourceName(string sourceName)
{
	this->sourceName = sourceName;
}

void
HTTweet::setText(string text)
{
	this->text = text;
}

void
HTTweet::setView(BView* view)
{
	this->view = view;
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
HTTweet::getRelativeDate() const
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
	return (this->getUnixTime() < b.getUnixTime());
}

uint64
HTTweet::getId()
{
	return id;
}

const char*
HTTweet::getIdString()
{
	std::stringstream out;
	out << id;
	return out.str().c_str();
}

void
HTTweet::setId(uint64 id)
{
	this->id = id;
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

const char*
HTTweet::monthToString(int month) const
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
}

void
HTTweet::setProfileImageUrl(string profileImageUrl)
{
	this->profileImageUrl = profileImageUrl;
}

BBitmap*
HTTweet::getBitmap()
{
	if(bitmapDownloadInProgress)
		return NULL;
	
	if(bitmapData != NULL) {
		if(imageBitmap != NULL)
			delete imageBitmap;
		imageBitmap = BTranslationUtils::GetBitmap(bitmapData);
		delete bitmapData;
		bitmapData = NULL;
	}
		
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
HTTweet::downloadBitmap(bool async)
{
	if(bitmapDownloadInProgress)
		return;
		
	if(async) {
		downloadThread = spawn_thread(_threadDownloadBitmap, profileImageUrl.c_str(), 10, this);
		resume_thread(downloadThread);
	}
	else {
	_threadDownloadBitmap(this);
	}
}

status_t
_threadDownloadBitmap(void *data)
{
	HTTweet *super = (HTTweet*)data;
	super->bitmapDownloadInProgress = true;
	std::string url(super->getProfileImageUrl());
	BMallocIO *mallocIO = NULL;
	
	status_t cacheStatus = HTStorage::findBitmap(url, &mallocIO);
	while(cacheStatus == B_BUSY) {
		usleep(1000);
		cacheStatus = HTStorage::findBitmap(url, &mallocIO);
	}
	
	int32 retryCount = 0;
	while(mallocIO == NULL && cacheStatus == B_ENTRY_NOT_FOUND) {
		//Touch the cache file so we don't start multiple downloads
		if(HTStorage::cacheBitmap(NULL, url) < B_OK) {
			cacheStatus = HTStorage::findBitmap(url, &mallocIO);
			continue;
		}
		CURL *curl_handle;
		
		mallocIO = new BMallocIO();
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
		
		/*Handle unsuccessful downloads*/
		if(mallocIO->BufferLength() < 256) {
			if(retryCount > 10)
				break;
			else {
				retryCount ++;
				delete mallocIO;
				mallocIO = NULL;
				usleep(500*retryCount);

				#ifdef DEBUG_ENABLED
				std::cout << "Data length to small - Retrying image download..." << std::endl;
				std::cout << super->getProfileImageUrl().c_str() << std::endl;
				#endif
			}
		}
		else
			break;
	}
	
	/*Success!*/
	if(cacheStatus == B_ENTRY_NOT_FOUND)
		HTStorage::cacheBitmap(mallocIO, url);
	super->bitmapDownloadInProgress = false;
	super->bitmapData = mallocIO;
	if(super->getView() != NULL && super->getView()->LockLooper()) {
			super->bitmapData = mallocIO;
			super->getView()->Invalidate();
			super->getView()->UnlockLooper();
	}
	
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
	
	if(bitmapData != NULL)
		delete bitmapData;
	
	if(imageBitmap != NULL)
		delete imageBitmap;
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
HTTweet::defaultBitmap()
{
	return BTranslationUtils::GetBitmap('PNG ', "logo_48");
}

int
HTTweet::sortByDateFunc(const void* voida, const void* voidb)
{
	const HTTweet** a = (const HTTweet**) voida;
	const HTTweet** b = (const HTTweet**) voidb;
	
	if((*a)->getUnixTime() < (*b)->getUnixTime())
		return 1;
	if((*a)->getUnixTime() > (*b)->getUnixTime())
		return -1;
	else
		return 0;
}
