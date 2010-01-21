/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include <iostream>
#include <string.h>
#include <TranslationUtils.h>
#include <TranslatorFormats.h>
#include <DataIO.h>
#include <Bitmap.h>

#include "curl/curl.h"

#ifndef HT_TWEET_H
#define HT_TWEET_H

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);

using namespace std;

struct DateStruct {
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

class HTTweet {
public:
	HTTweet();
	HTTweet(string &screenName, string &text, string &profileImageUrl, string &dateString);
	HTTweet(HTTweet *originalTweet);
	~HTTweet();
	bool operator<(const HTTweet &b) const;
	const string getScreenName();
	const string getText();
	const string getProfileImageUrl();
	BBitmap* getBitmap();
	struct DateStruct getDate() const;
	void setScreenName(string&);
	void setText(string&);
	void setProfileImageUrl(string&);
	void setDate(string&);
	
private:
	const int stringToMonth(const char *date);
	string screenName;
	string text;
	string profileImageUrl;
	struct DateStruct date;
	BBitmap *imageBitmap;
	void downloadBitmap(const char *url);
};
#endif
