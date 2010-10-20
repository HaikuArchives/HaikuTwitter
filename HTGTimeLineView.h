/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "ListView.h"
#include "ScrollView.h"
#include "TabView.h"
#include "Roster.h"

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
	void updateTimeLine();
	void AttachedToWindow();
	void SetFont(const BFont*, uint32 = B_FONT_ALL);
	void AddList(BList *tweets);
	void setSaveTweets(bool);
	void clearList();
	~HTGTimeLineView();
	
//private:
	void sendNotificationFor(HTTweet *theTweet);
	int32 getSearchID();
	void setSearchID(int32 id);
	void savedSearchDestoySelf();
	void savedSearchCreateSelf();
	bool _retrieveInfoPopperBoolFromSettings();
	bool waitingForUpdate;
	bool wantsNotifications;
	bool saveTweets;
	BListView *listView;
	BList *unhandledList;
	BScrollView *theScrollView;
	
	thread_id previousThread;
	
	int32 searchID;
	TimeLineParser *timeLineParser;
	twitCurl *twitObj;
	int32 TYPE;
	
	std::string& htmlFormatedString(const char *orig);
};
#endif
