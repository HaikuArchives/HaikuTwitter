/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "Window.h"
#include "TextControl.h"
#include "Button.h"
#include "StringView.h"
#include "TranslationUtils.h"
#include "Bitmap.h"
#include "View.h"
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <Roster.h>
#include <iostream>
#include <FindDirectory.h>

#include "twitcurl/twitcurl.h"
#include "HTGMainWindow.h"

#ifndef HTG_AUTHORIZEWINDOW_H
#define HTG_AUTHORIZEWINDOW_H

const int32 GO_TO_AUTH_URL = 'URL';
const int32 GO_AUTH = 'GO';

struct oauth_settings {
	char key[255];
	char secret[255];
};

class HTGAuthorizeWindow : public BWindow {
public:
	HTGAuthorizeWindow(int refreshTime, BPoint position, int height);
	~HTGAuthorizeWindow();
	void MessageReceived(BMessage *msg);

private:
	status_t _getSettingsPath(BPath &path);
	void openUrl(std::string url);
	void storeTokens(std::string key, std::string secret);
	int refreshTime;
	BPoint position;
	int height;
	
	oauth_settings oauth;
	twitCurl *twitObj;
	BTextControl *query;
	BButton *goButton;
	BView *theView;
	BWindow *delegate;
};
#endif
