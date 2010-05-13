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
const int32 SHORTEN = 'SURL';

class HTGNewTweetWindow : public BWindow {
public:
	HTGNewTweetWindow(twitCurl *);
	~HTGNewTweetWindow();
	void inputChanged(int length);
	void MessageReceived(BMessage *msg);
	void SetText(const char *text);
	void setTweetId(const char* tweetId);

private:
	void postTweet();
	void updateCounter();
	void shortenAllUrls();
	char *shortenUrl(const char *longUrl);
	std::string urlEncode(const char* input);
	std::string tweetId;
	
	twitCurl *twitObj;
	HTGTextView *message;
	BButton *postButton;
	BButton *cancelButton;
	BButton *shortenButton;
	BView *theView;
	BStringView *counterView;
};
#endif
