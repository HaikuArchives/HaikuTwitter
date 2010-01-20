/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "Application.h"
#include "Window.h"
#include "ListView.h"
#include "ScrollView.h"
#include "MenuBar.h"
#include "Menu.h"
#include "MenuItem.h"

#include "TimeLineParser.h"
#include "twitcurl/twitcurl.h"
#include "HTTweet.h"
#include "HTGNewTweetWindow.h"
#include "HTGAccountSettingsWindow.h"
#include "HTGAboutWindow.h"
#include "HTGTweetItem.h"

#ifndef HTG_TIMELINEWINDOW_H
#define HTG_TIMELINEWINDOW_H

const int32 NEW_TWEET = 'NWTW';
const int32 REFRESH = 'REFR';
const int32 ACCOUNT_SETTINGS = 'ACC';
const int32 ABOUT = 'BOUT';

class HTGTimeLineWindow : public BWindow {
public:
	HTGTimeLineWindow(twitCurl *);
	void MessageReceived(BMessage *msg);
	bool QuitRequested();
	~HTGTimeLineWindow();
	
private:
	void _SetupMenu();
	void updateTimeLine();

	BListView *listView;
	BScrollView *scrollView;
	BView *containerView;
	TimeLineParser *timeLineParser;
	twitCurl *twitObj;
	HTTweet** tweetPtr;
	HTGNewTweetWindow *newTweetWindow;
	HTGAccountSettingsWindow *accountSettingsWindow;
	HTGAboutWindow *aboutWindow;
	
	BMenuBar *fMenuBar;
	BMenu *fTwitterMenu;
	BMenu *fEditMenu;
	BMenu *fSettingsMenu;
};
#endif
