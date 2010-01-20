/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGAccountSettingsWindow.h"

HTGAccountSettingsWindow::HTGAccountSettingsWindow() : BWindow(BRect(100, 100, 500, 210), "Account settings", B_TITLED_WINDOW, B_NOT_RESIZABLE) {
	_retrieveSettings();
	_setupWindow();
}

HTGAccountSettingsWindow::~HTGAccountSettingsWindow() {
	_saveSettings();
	
	revertButton->RemoveSelf();
	usernameView->RemoveSelf();
	passwordView->RemoveSelf();
	backgroundView->RemoveSelf();
	delete revertButton;
	delete usernameView;
	delete passwordView;
	delete backgroundView;
}

status_t HTGAccountSettingsWindow::_getSettingsPath(BPath &path) {
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("HaikuTwitter_settings");
	return B_OK;
}

void HTGAccountSettingsWindow::_setupWindow() {
	/*Add the view*/
	backgroundView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	backgroundView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(backgroundView);
	
	/*Add revertButton*/
	revertButton = new BButton(BRect(3, 80, 100, -1), NULL, "Revert", new BMessage(REVERT));
	backgroundView->AddChild(revertButton);
	
	/*Add the username field*/
	usernameView = new BTextControl(BRect(5,5,250,25), "Username", "Username", theSettings.username, new BMessage());
	usernameView->SetDivider(usernameView->Divider() -25);
	backgroundView->AddChild(usernameView);
	
	/*Add the password field*/
	passwordView = new BTextControl(BRect(5,30,250,25), "Password", "Password", theSettings.password, new BMessage());
	passwordView->TextView()->HideTyping(true);
	passwordView->SetDivider(passwordView->Divider() -25);
	backgroundView->AddChild(passwordView);
	
	/*Add the refresh field*/
	char refreshTime[32];
	sprintf(refreshTime, "%i", theSettings.refreshTime);
	refreshView = new BTextControl(BRect(5,55,250,25), "Refresh", "Refresh interval (in minutes)", refreshTime, new BMessage());
	refreshView->SetDivider(refreshView->Divider() +70);
	backgroundView->AddChild(refreshView);
}

void HTGAccountSettingsWindow::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case REVERT:
			_retrieveSettings();
			usernameView->SetText(theSettings.username);
			passwordView->SetText(theSettings.password);
			char refreshTime[32];
			sprintf(refreshTime, "%i", theSettings.refreshTime);
			refreshView->SetText(refreshTime);
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}

void HTGAccountSettingsWindow::_retrieveSettings() {
	BPath path;
	
	if (_getSettingsPath(path) < B_OK)
		return;
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK)
		return;
	file.ReadAt(0, &theSettings, sizeof(twitter_settings));
	
	if(theSettings.refreshTime < 1) {
		sprintf(theSettings.username, "changeme");
		sprintf(theSettings.password, "hackme");
		theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	}
}

status_t HTGAccountSettingsWindow::_saveSettings() {
	if (strcmp(usernameView->Text(), theSettings.username) == 0 && strcmp(passwordView->Text(), theSettings.password) == 0 && theSettings.refreshTime == atoi(refreshView->Text()))
		return B_OK;
		
	BAlert *theAlert = new BAlert("Please restart!", "You must restart HaikuTwitter for the changes to take place.", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);	
	int32 button_index = theAlert->Go();
		
	sprintf(theSettings.username, usernameView->Text());
	sprintf(theSettings.password, passwordView->Text());
	theSettings.refreshTime = atoi(refreshView->Text());
	
	BPath path;
	status_t status = _getSettingsPath(path);
	if (status < B_OK)
		return status;
		
	BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE);
	if (status < B_OK)
		return status;
		
	file.WriteAt(0, &theSettings, sizeof(twitter_settings));
	std::cout << "Settings saved" << std::endl;
}
