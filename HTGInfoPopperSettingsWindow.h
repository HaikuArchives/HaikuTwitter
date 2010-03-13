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
#include <Button.h>
#include <CheckBox.h>
#include <Message.h>
#include <StringView.h>
#include <FindDirectory.h>

#ifndef HTG_INFOPOPPER_SETTINGS_WINDOW
#define HTG_INFOPOPPER_SETTINGS_WINDOW

using namespace std;

const int32 kRevert = 'RVRT';

struct infopopper_settings {
	bool friendsNotify;
	bool mentionsNotify;
	bool publicNotify;
};



class HTGInfoPopperSettingsWindow : public BWindow {
public:
	HTGInfoPopperSettingsWindow();
	~HTGInfoPopperSettingsWindow();
	
	static infopopper_settings _getDefaults();
	static status_t _getSettingsPath(BPath &path);
	
private:
	void _retrieveSettings();
	status_t _saveSettings();
	void _setupWindow();
	void MessageReceived(BMessage *msg);
	infopopper_settings theSettings;
	
	BButton *revertButton;
	BCheckBox *friendsNotifyBox;
	BCheckBox *mentionsNotifyBox;
	BCheckBox *publicNotifyBox;
	BView *backgroundView;
	BStringView *informationView;
	
};
#endif
