/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "Window.h"
#include "HTGTextView.h"
#include "Button.h"
#include "View.h"
#include "StringView.h"
#include "Alert.h"

#include "twitcurl/twitcurl.h"

#include "TimeLineParser.h"
#include "HTTweet.h"

#ifndef HTG_NEWTWEETWINDOW_H
#define HTG_NEWTWEETWINDOW_H

#define NUMBER_OF_ALLOWED_CHARS 140

const int32 POST = 'POST';
const int32 CANCEL = 'CNL';
const int32 UPDATED = 'UPDT';

class HTGNewTweetWindow : public BWindow {
public:
	HTGNewTweetWindow(twitCurl *);
	~HTGNewTweetWindow();
	void inputChanged(int length);
	void MessageReceived(BMessage *msg);

private:
	void postTweet();
	void updateCounter();
	
	twitCurl *twitObj;
	HTGTextView *message;
	BButton *postButton;
	BButton *cancelButton;
	BView *theView;
	BStringView *counterView;
};
#endif
