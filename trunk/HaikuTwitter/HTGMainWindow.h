/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#ifndef HTG_MAINWINDOW_H
#define HTG_MAINWINDOW_H

#include "Application.h"
#include "Window.h"
#include "ListView.h"
#include "ScrollView.h"
#include "MenuBar.h"
#include "Menu.h"
#include "MenuItem.h"
#include "MessageRunner.h"

#include "SmartTabView.h"
#include "TimeLineParser.h"
#include "twitcurl/twitcurl.h"
#include "HTTweet.h"
#include "HTGNewTweetWindow.h"
#include "HTGGoToUserWindow.h"
#include "HTGSearchForWindow.h"
#include "HTGAccountSettingsWindow.h"
#include "HTGInfoPopperSettingsWindow.h"
#include "HTGTweetItem.h"
#include "HTGTimeLineView.h"
#include "HTGTimeLineWindow.h"

const int32 NEW_TWEET = 'NWTW';
const int32 REFRESH = 'REFR';
const int32 ACCOUNT_SETTINGS = 'ACC';
const int32 INFOPOPPER_SETTINGS = 'IPS';
const int32 GO_USER = 'GUSR';
const int32 FIND_USER = 'FUSR';
const int32 SEARCH_FOR = 'SFOR';
const int32 GO_SEARCH = 'SRCH';
const int32 ABOUT = 'BOUT';
const int32 CLOSE_TAB = 'CTAB';
const int32 TOGGLE_TABS = 'TTAB';

class HTGMainWindow : public BWindow {
public:
	HTGMainWindow(string username, string password, int refreshTime, BPoint position, int height);
	virtual void MessageReceived(BMessage *msg);
	bool QuitRequested();
	~HTGMainWindow();
	
private:
	string username;
	string password;
	int refreshTime;
	twitter_settings theSettings;

	void _SetupMenu();
	status_t _getSettingsPath(BPath &path);
	void _retrieveSettings();
	status_t _saveSettings();
	void showAbout();

	SmartTabView *tabView;
	
	twitCurl *newTweetObj;
	twitCurl *newTabObj;
	
	HTGTimeLineView *homeTimeLine;
	HTGTimeLineView *friendsTimeLine;
	HTGTimeLineView *mentionsTimeLine;
	HTGTimeLineView *publicTimeLine;
	
	HTGGoToUserWindow *goToUserWindow;
	HTGSearchForWindow *searchForWindow;
	HTGNewTweetWindow *newTweetWindow;
	HTGTimeLineWindow *timeLineWindow;
	HTGAccountSettingsWindow *accountSettingsWindow;
	HTGInfoPopperSettingsWindow *infopopperSettingsWindow;
	
	bool openInNewWindow;
			
	BMenuBar *fMenuBar;
	BMenu *fTwitterMenu;
	BMenu *fEditMenu;
	BMenu *fSettingsMenu;
	BMenuItem *fOpenInTabsMenuItem;
};
#endif
