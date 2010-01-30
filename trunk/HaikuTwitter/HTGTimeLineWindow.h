/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "Application.h"
#include "Window.h"
#include "ListView.h"
#include "ScrollView.h"
#include "MessageRunner.h"

#include <string>

#include "TimeLineParser.h"
#include "twitcurl/twitcurl.h"
#include "HTTweet.h"
#include "HTGTweetItem.h"
#include "HTGTimeLineView.h"

#ifndef HTG_TIMELINEWINDOW_H
#define HTG_TIMELINEWINDOW_H

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);

class HTGTimeLineWindow : public BWindow {
public:
	HTGTimeLineWindow(BWindow *parent, string username, string password, int refreshTime, const int32 TYPE, const char* requestInfo);
	void MessageReceived(BMessage *msg);
	~HTGTimeLineWindow();
	
private:	
	HTGTimeLineView *theTimeLine;
	BMessageRunner *refreshTimer;
	BWindow *parent;
};
#endif
