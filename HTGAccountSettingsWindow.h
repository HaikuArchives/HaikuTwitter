/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <Window.h>
#include <Alert.h>
#include <File.h>
#include <Path.h>
#include <TextControl.h>
#include <CheckBox.h>
#include <Button.h>
#include <Message.h>
#include <StringView.h>
#include <FindDirectory.h>

#ifndef HTG_ACCOUNT_SETTINGS_WINDOW
#define HTG_ACCOUNT_SETTINGS_WINDOW

using namespace std;

const int32 REVERT = 'RVRT';

struct twitter_settings {
	char username[30];
	char password[30];
	int refreshTime;
	BPoint position;
	int height;
	bool useTabs;
	bool enablePublic;
	bool saveSearches;
};



class HTGAccountSettingsWindow : public BWindow {
public:
	HTGAccountSettingsWindow();
	~HTGAccountSettingsWindow();
	
private:
	status_t _getSettingsPath(BPath &path);
	void _retrieveSettings();
	status_t _saveSettings();
	void _setupWindow();
	void MessageReceived(BMessage *msg);
	twitter_settings theSettings;
	
	BButton *revertButton;
	BTextControl *passwordView;
	BTextControl *usernameView;
	BTextControl *refreshView;
	BCheckBox *savedSearchesBox;
	BView *backgroundView;
	BStringView *informationView;
	
};
#endif
