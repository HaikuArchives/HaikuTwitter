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
#include "File.h"
#include "Path.h"
#include "FindDirectory.h"
#include "Directory.h"

#include "AnimationHelper.h"
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
#include "HTGErrorHandling.h"

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
const int32 TOGGLE_PUBLIC = 'TPUB';
const int32 TOGGLE_AUTOSTART = 'ASTT';
const int32 TOGGLE_SAVETWEETS = 'STTG';
const int32 TEXT_SIZE_INCREASE = 'TINC';
const int32 TEXT_SIZE_DECREASE = 'TDEC';
const int32 TEXT_SIZE_REVERT = 'TREV';

status_t addSavedSearchesThreadFunction(void *data);
status_t addTrendingThreadFunction(void *data);

class HTGMainWindow : public BWindow {
public:
	HTGMainWindow(string key, string secret, int refreshTime, BPoint position, int height);
	virtual void MessageReceived(BMessage *msg);
	bool QuitRequested();
	void setQuitOnClose(bool);
	~HTGMainWindow();
	
private:
	string key;
	string secret;
	int refreshTime;
	twitter_settings theSettings;
	
	int trendingRefreshCounter;
	
	bool _isAutoStarted();
	void _setAutoStarted(bool autostarted);
	void _SetupMenu();
	void _addPublicTimeLine();
	void _removePublicTimeLine();
	void _addSavedSearches();
	void _addTrending();
	void _setTimelineTextSize(int);
	status_t _getSettingsPath(BPath &path);
	void _retrieveSettings();
	status_t _saveSettings();
	void showAbout();

	bool quitOnClose;

	SmartTabView *tabView;
	
	twitCurl *newTweetObj;
	twitCurl *newTabObj;
	
	HTGTimeLineView *homeTimeLine;
	HTGTimeLineView *friendsTimeLine;
	HTGTimeLineView *mentionsTimeLine;
	HTGTimeLineView *publicTimeLine;
	HTGTimeLineView *directTimeLine;
	
	HTGGoToUserWindow *goToUserWindow;
	HTGSearchForWindow *searchForWindow;
	HTGNewTweetWindow *newTweetWindow;
	HTGTimeLineWindow *timeLineWindow;
	HTGAccountSettingsWindow *accountSettingsWindow;
	HTGInfoPopperSettingsWindow *infopopperSettingsWindow;
				
	BMenuBar *fMenuBar;
	BMenu *fTwitterMenu;
	BMenu *fEditMenu;
	BMenu *fSettingsMenu;
	BMenu *fTrendingSubMenu;
	BMenuItem *fOpenInTabsMenuItem;
	BMenuItem *fEnablePublicMenuItem;
	BMenuItem *fAutoStartMenuItem;
	BMenuItem *fSaveTweetsMenuItem;
};
#endif
