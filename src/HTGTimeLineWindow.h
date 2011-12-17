/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "Application.h"
#include "Window.h"
#include "ListView.h"
#include "ScrollView.h"
#include "MessageRunner.h"

#include <string>

#include "twitcurl/twitcurl.h"
#include "HTTweet.h"
//#include "HTGTweetItem.h"
#include "HTGTimeLineView.h"

#ifndef HTG_TIMELINEWINDOW_H
#define HTG_TIMELINEWINDOW_H

class HTGTimeLineWindow : public BWindow {
public:
	HTGTimeLineWindow(BWindow *parent, string key, string secret, int refreshTime, const int32 TYPE, const char* requestInfo);
	void MessageReceived(BMessage *msg);
	~HTGTimeLineWindow();
	
private:	
	HTGTimeLineView *theTimeLine;
	BView* backgroundView;
	BMessageRunner *refreshTimer;
	BWindow *parent;
};
#endif
