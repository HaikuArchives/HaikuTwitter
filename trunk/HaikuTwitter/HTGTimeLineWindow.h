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
#include "TabView.h"
#include "MessageRunner.h"

#include "TimeLineParser.h"
#include "twitcurl/twitcurl.h"
#include "HTTweet.h"
#include "HTGNewTweetWindow.h"
#include "HTGAccountSettingsWindow.h"
#include "HTGAboutWindow.h"
#include "HTGTweetItem.h"
#include "HTGTimeLineView.h"

#ifndef HTG_TIMELINEWINDOW_H
#define HTG_TIMELINEWINDOW_H

const int32 NEW_TWEET = 'NWTW';
const int32 REFRESH = 'REFR';
const int32 ACCOUNT_SETTINGS = 'ACC';
const int32 GO_USER = 'GUSR';
const int32 ABOUT = 'BOUT';

class HTGTimeLineWindow : public BWindow {
public:
	HTGTimeLineWindow(string username, string password, int refreshTime);
	void MessageReceived(BMessage *msg);
	bool QuitRequested();
	~HTGTimeLineWindow();
	
private:
	void _SetupMenu();

	BTabView *tabView;
	
	twitCurl *newTweetObj;
	
	HTGTimeLineView *friendsTimeLine;
	HTGTimeLineView *mentionsTimeLine;
	HTGTimeLineView *publicTimeLine;
	
	HTGNewTweetWindow *newTweetWindow;
	HTGAccountSettingsWindow *accountSettingsWindow;
	HTGAboutWindow *aboutWindow;
		
	BMenuBar *fMenuBar;
	BMenu *fTwitterMenu;
	BMenu *fEditMenu;
	BMenu *fSettingsMenu;
};
#endif
