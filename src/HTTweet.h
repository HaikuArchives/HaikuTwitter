/*
 * Copyright 2010-2012 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#ifndef HT_TWEET_H
#define HT_TWEET_H

#include <string>

#include <Archivable.h>

class BView;
class BBitmap;

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
	HTTweet(HTTweet *originalTweet);
	HTTweet(BMessage* archive);
	~HTTweet();
	status_t Archive(BMessage* archive, bool deep);
	BArchivable* Instantiate(BMessage* archive);
	bool operator<(const HTTweet &b) const;
	BView* getView();
	const std::string getScreenName() const;
	const std::string getRetweetedBy() const;
	const std::string getFullName();
	const std::string getText();
	const std::string getProfileImageUrl();
	const std::string getRelativeDate() const;
	const std::string getSourceName();
	time_t getUnixTime() const;
	BBitmap* getBitmap();
	BBitmap getBitmapCopy();
	struct DateStruct getDate() const;
	static int sortByDateFunc(const void*, const void*);
	static BBitmap* defaultBitmap();
	void setView(BView *);
	void downloadBitmap(bool async = false);
	void setRetweetedBy(std::string);
	void setScreenName(std::string);
	void setFullName(std::string);
	void setText(std::string);
	void setProfileImageUrl(std::string);
	void setDate(time_t);
	void setId(uint64);
	void setSourceName(std::string);
	void setBitmap(BBitmap *);
	void waitUntilDownloadComplete();
	bool isDownloadingBitmap();
	uint64 getId();
	const char* getIdString();
	bool following();
	void setFollowing(bool);
	
	/*This must be public (threads)*/
	bool bitmapDownloadInProgress; // Archived
	BMallocIO* bitmapData;
	
private:
	const char* monthToString(int month) const;
	BView *view;				// Not archived
	thread_id downloadThread;	// Not archived
	BBitmap *imageBitmap;		// Archived
	std::string screenName;			// Archived
	std::string fullName;			// Archived
	std::string text;				// Archived
	std::string profileImageUrl;		// Archived
	std::string sourceName;			// Archived
	std::string retweetedBy;
	struct DateStruct date;		// Not archived
	uint64 id;					// Archived
	bool isFollowing;			// Archived
};


#endif // HT_TWEET_H
