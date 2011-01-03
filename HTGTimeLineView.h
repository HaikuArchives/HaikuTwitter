/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "ListView.h"
#include "ScrollView.h"
#include "TabView.h"
#include "Roster.h"
#include "Dragger.h"
#include "Archivable.h"

#include <string>
#include <sstream>

#include <Notification.h>

#include "HTStorage.h"
#include "TimeLineParser.h"
#include "DirectMessageParser.h"
#include "SearchParser.h"
#include "twitcurl/twitcurl.h"
#include "HTTweet.h"
#include "HTGTweetItem.h"
#include "HTGInfoPopperSettingsWindow.h"

#ifndef HTG_TIMELINEVIEW_H
#define HTG_TIMELINEVIEW_H

const int32 TIMELINE_HOME = 'HOME';
const int32 TIMELINE_FRIENDS = 'TML';
const int32 TIMELINE_MENTIONS = 'MNTN';
const int32 TIMELINE_PUBLIC = 'PBLC';
const int32 TIMELINE_USER = 'TUSR';
const int32 TIMELINE_SEARCH = 'TSCH';
const int32 TIMELINE_DIRECT = 'DMSG';
const int32 TIMELINE_HDD = 'THDD';

status_t updateTimeLineThread(void *data);

class HTGTimeLineView : public BView {
public:
	HTGTimeLineView(twitCurl *, const int32, BRect, const char * requestInfo = " ", int textSize = BFont().Size(), bool saveTweets = false);
	HTGTimeLineView(const int32, BRect, BList*, int textSize = BFont().Size());
	HTGTimeLineView(BMessage* archive);
	void updateTimeLine();
	void AttachedToWindow();
	void SetFont(const BFont*, uint32 = B_FONT_ALL);
	void AddList(BList *tweets);
	void setSaveTweets(bool);
	void clearList();
	virtual status_t Archive(BMessage* archive, bool deep = true) const;
	BArchivable* Instantiate(BMessage* archive);
	~HTGTimeLineView();
	
//private:
	void sendNotificationFor(HTTweet *theTweet);
	int32 getSearchID();
	void setSearchID(int32 id);
	void savedSearchDestoySelf();
	void savedSearchCreateSelf();
	bool _retrieveInfoPopperBoolFromSettings();
	std::string& htmlFormatedString(const char *orig);
	
	bool waitingForUpdate;		//Archived
	bool wantsNotifications;	//Archived
	bool saveTweets;			//Archived
	BListView *listView;		//Archived (deep)
	BList *unhandledList;		//Archived (deep)
	BScrollView *theScrollView;	//Not archived
	
	thread_id previousThread;	//Not archived
	
	int32 searchID;				//Archived
	twitCurl *twitObj;			//Not archived (oauth keys archived instead)
	int32 TYPE;					//Archived
};
#endif
