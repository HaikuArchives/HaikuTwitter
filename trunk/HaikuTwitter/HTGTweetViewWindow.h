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

#ifndef HTG_TWEETVIEWWINDOW_H
#define HTG_TWEETVIEWWINDOW_H

const int32 TWEETVIEWWINDOW_CLOSED = 'TVWC';

class HTGTweetViewWindow : public BWindow {
public:
	HTGTweetViewWindow(BWindow *parent, BList* tweets = new BList());
	void MessageReceived(BMessage *msg);
	bool QuitRequested();
	void AddList(BList *tweets);
	~HTGTweetViewWindow();
	
private:	
	HTGTimeLineView *theTimeLine;
	BMessageRunner *refreshTimer;
	BWindow *parent;
};
#endif
