/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#ifndef HT_TWEET_H
#define HT_TWEET_H

#include <iostream>
#include <string.h>
#include <string>
#include <View.h>
#include <TranslationUtils.h>
#include <TranslatorFormats.h>
#include <DataIO.h>
#include <Bitmap.h>
#include <Archivable.h>
#include <File.h>
#include <BitmapStream.h>
#include <TranslatorRoster.h>
#include <Application.h>
#include <Resources.h>
#include <Roster.h>

#include <ctime>
#include <curl/curl.h>

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);
status_t _threadDownloadBitmap(void *);
BBitmap* defaultBitmap();

using namespace std;

struct DateStruct {
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int year;
};

class HTTweet : public BArchivable {
public:
	HTTweet();
	HTTweet(string &screenName, string &text, string &profileImageUrl, string &dateString);
	HTTweet(HTTweet *originalTweet);
	HTTweet(BMessage* archive);
	~HTTweet();
	status_t Archive(BMessage* archive, bool deep);
	BArchivable* Instantiate(BMessage* archive);
	bool operator<(const HTTweet &b) const;
	BView* getView();
	const string getScreenName();
	const string getFullName();
	const string getText();
	const string getProfileImageUrl();
	const string getRelativeDate();
	const string getRawDate();
	const string getSourceName();
	time_t getUnixTime() const;
	BBitmap* getBitmap();
	BBitmap getBitmapCopy();
	struct DateStruct getDate() const;
	void setView(BView *);
	void downloadBitmap();
	void setScreenName(string&);
	void setFullName(string&);
	void setText(string&);
	void setProfileImageUrl(string&);
	void setDate(string&);
	void setDate(time_t);
	void setPublishedDate(string&);
	void setId(const char*);
	void setSourceName(string&);
	void setBitmap(BBitmap *);
	void waitUntilDownloadComplete();
	bool isDownloadingBitmap();
	const char* getId();
	bool following();
	void setFollowing(bool);
	
	/*This must be public (threads)*/
	bool bitmapDownloadInProgress;//Archived
	
private:
	const int stringToMonth(const char *date);	
	const char* monthToString(int month);
	BView *view;				//Not archived
	thread_id downloadThread;	//Not archived
	BBitmap *imageBitmap;		//Archived
	string screenName;			//Archived
	string fullName;			//Archived
	string text;				//Archived
	string profileImageUrl;		//Archived
	string rawDate;				//Archived
	string sourceName;			//Archived
	struct DateStruct date;		//Not archived
	string id;					//Archived
	bool isFollowing;			//Archived
};
#endif
