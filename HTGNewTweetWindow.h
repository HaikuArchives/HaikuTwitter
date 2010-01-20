/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "Window.h"
#include "TextView.h"
#include "Button.h"
#include "View.h"
#include "Alert.h"

#include "twitcurl/twitcurl.h"

#include "TimeLineParser.h"
#include "HTTweet.h"

#ifndef HTG_NEWTWEETWINDOW_H
#define HTG_NEWTWEETWINDOW_H

const int32 POST = 'POST';
const int32 CANCEL = 'CNL';
static const rgb_color kDarkGrey = { 100, 100, 100, 255 };

class HTGNewTweetWindow : public BWindow {
public:
	HTGNewTweetWindow(twitCurl *);
	~HTGNewTweetWindow();
	void MessageReceived(BMessage *msg);

private:
	void postTweet();
	
	twitCurl *twitObj;
	BTextView *message;
	BButton *postButton;
	BButton *cancelButton;
	BView *theView;
};
#endif
