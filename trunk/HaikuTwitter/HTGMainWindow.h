/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
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
#include "twitcurl/twitcurl.h"
#include "HTTweet.h"
#include "HTGNewTweetWindow.h"
#include "HTGGoToUserWindow.h"
#include "HTGSearchForWindow.h"
#include "HTGAccountSettingsWindow.h"
#include "HTGInfoPopperSettingsWindow.h"
//#include "HTGTweetItem.h"
#include "HTGTimeLineView.h"
#include "HTGTimeLineWindow.h"
#include "HTGErrorHandling.h"
#include "HTGAvatarView.h"
#include "HTGStatusBar.h"
#include "HTAccountCredentials.h"

const uint32 NEW_TWEET = 'NWTW';
const uint32 NEW_RETWEET = 'NRTW';
const uint32 REFRESH = 'REFR';
const uint32 ACCOUNT_SETTINGS = 'ACC';
const uint32 INFOPOPPER_SETTINGS = 'IPS';
const uint32 GO_USER = 'GUSR';
const uint32 FIND_USER = 'FUSR';
const uint32 SEARCH_FOR = 'SFOR';
const uint32 GO_SEARCH = 'SRCH';
const uint32 ABOUT = 'BOUT';
const uint32 CLOSE_TAB = 'CTAB';
const uint32 TOGGLE_AVATARVIEW = 'TATR';
const uint32 TOGGLE_TABS = 'TTAB';
const uint32 TOGGLE_PUBLIC = 'TPUB';
const uint32 TOGGLE_AUTOSTART = 'ASTT';
const uint32 TOGGLE_SAVETWEETS = 'STTG';
const uint32 TEXT_SIZE_INCREASE = 'TINC';
const uint32 TEXT_SIZE_DECREASE = 'TDEC';
const uint32 TEXT_SIZE_REVERT = 'TREV';

status_t addSavedSearchesThreadFunction(void *data);
status_t addTrendingThreadFunction(void *data);

class HTGMainWindow : public BWindow {
public:
	HTGMainWindow(string key, string secret, int refreshTime, BPoint position, int height);
	virtual void MessageReceived(BMessage *msg);
	bool QuitRequested();
	void setQuitOnClose(bool);
	void AvatarViewResized();
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
	void _SetupAvatarView();
	void _ResizeAvatarView();
	void _addPublicTimeLine();
	void _removePublicTimeLine();
	void _addSavedSearches();
	void _setTimelineTextSize(int);
	status_t _getSettingsPath(BPath &path);
	void _retrieveSettings();
	status_t _saveSettings();
	void showAbout();

	bool quitOnClose;
	
	bool noAuth;

	SmartTabView *tabView;
	
	twitCurl *newTweetObj;
	twitCurl *newTabObj;
	
	HTGTimeLineView *homeTimeLine;
	HTGTimeLineView *friendsTimeLine;
	HTGTimeLineView *mentionsTimeLine;
	HTGTimeLineView *publicTimeLine;
	HTGTimeLineView *directTimeLine;
	
	HTAccountCredentials* accountCredentials;
	
	HTGGoToUserWindow *goToUserWindow;
	HTGSearchForWindow *searchForWindow;
	HTGNewTweetWindow *newTweetWindow;
	HTGTimeLineWindow *timeLineWindow;
	HTGAccountSettingsWindow *accountSettingsWindow;
	HTGInfoPopperSettingsWindow *infopopperSettingsWindow;
	
	HTGAvatarView *fAvatarView;
	
	HTGStatusBar *statusBar;
	
	BMenuBar *fMenuBar;
	BMenu *fTwitterMenu;
	BMenu *fViewMenu;
	BMenu *fSettingsMenu;
	BMenuItem *fHideAvatarViewMenuItem;
	BMenuItem *fEnablePublicMenuItem;
	BMenuItem *fAutoStartMenuItem;
	BMenuItem *fSaveTweetsMenuItem;
};
#endif
