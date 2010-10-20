/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTGAccountSettingsWindow.h"

HTGAccountSettingsWindow::HTGAccountSettingsWindow(BWindow *parent) 
	: BWindow(BRect(100, 100, 500, 180), "Account settings", B_TITLED_WINDOW, B_NOT_RESIZABLE)
{
	this->parent = parent;
	_retrieveSettings();
	_setupWindow();
}

HTGAccountSettingsWindow::~HTGAccountSettingsWindow()
{
	_saveSettings();
	
	revertButton->RemoveSelf();
	refreshView->RemoveSelf();
	usernameView->RemoveSelf();
	passwordView->RemoveSelf();
	backgroundView->RemoveSelf();
	delete revertButton;
	delete usernameView;
	delete passwordView;
	delete refreshView;
	delete backgroundView;
}

status_t
HTGAccountSettingsWindow::_getSettingsPath(BPath &path)
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("HaikuTwitter_settings");
	return B_OK;
}

void
HTGAccountSettingsWindow::_setupWindow()
{
	/*Add the view*/
	backgroundView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	backgroundView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(backgroundView);
	
	/*Add the username field*/
	usernameView = new BTextControl(BRect(5,5,250,25), "Username", "Username", theSettings.username, new BMessage());
	usernameView->SetDivider(usernameView->Divider() -25);
	
	/*Add the password field*/
	passwordView = new BTextControl(BRect(5,30,250,25), "Password", "Password", theSettings.password, new BMessage());
	passwordView->TextView()->HideTyping(true);
	passwordView->SetDivider(passwordView->Divider() -25);
	
	/*Add the refresh field*/
	char refreshTime[32];
	sprintf(refreshTime, "%i", theSettings.refreshTime);
	refreshView = new BTextControl(BRect(5,5,250,25), "Refresh", "Refresh interval (in minutes)", refreshTime, new BMessage());
	refreshView->SetDivider(refreshView->Divider() +70);
	backgroundView->AddChild(refreshView);
	
	/*Add the saveSearches field*/
	savedSearchesBox = new BCheckBox(BRect(5, 30, 300, 25), "SaveSearches Checkbox", "Synchronize tabs with my saved searches", new BMessage());
	if(theSettings.saveSearches)
		savedSearchesBox->SetValue(B_CONTROL_ON);
	backgroundView->AddChild(savedSearchesBox);

	/*Reset authorization*/
	BButton *resetOAuthButton = new BButton(BRect(130, 50, 320, -1), NULL, "Log in as different user", new BMessage(RESET_AUTH));
	backgroundView->AddChild(resetOAuthButton);
	
	/*Add revertButton*/
	revertButton = new BButton(BRect(3, 50, 120, -1), NULL, "Revert", new BMessage(REVERT));
	backgroundView->AddChild(revertButton);
}

void
HTGAccountSettingsWindow::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case REVERT:
			_retrieveSettings();
			usernameView->SetText(theSettings.username);
			passwordView->SetText(theSettings.password);
			char refreshTime[32];
			sprintf(refreshTime, "%i", theSettings.refreshTime);
			refreshView->SetText(refreshTime);
			break;
		case RESET_AUTH: {
			_invalidateOAuth();
			BAlert *theAlert = new BAlert("Please restart!", "You must restart HaikuTwitter for the changes to take place.", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);	
			int32 button_index = theAlert->Go();
			this->Close();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}

void
HTGAccountSettingsWindow::_invalidateOAuth()
{
	BPath settingsPath;
	
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	if (status < B_OK) {
		HTGErrorHandling::displayError("Unable to locate settings path.");
		return;
	}
	
	//settingsPath.Append("boot/launch/");
	
	BDirectory settingsDir(settingsPath.Path());
	
	if (settingsDir.Contains("HaikuTwitter_oauth")) {//Delete symlink
		BEntry *entry = new BEntry();
		settingsDir.FindEntry("HaikuTwitter_oauth", entry, false);
		if(entry->Remove() < B_OK)
			HTGErrorHandling::displayError("Unable to delete HaikuTwitter_oauth in settings directory.\n Please delete it manually.");
		delete entry;
	}
}

void
HTGAccountSettingsWindow::_retrieveSettings()
{
	/*Set the defaults, just in case anything bad happens*/
	sprintf(theSettings.username, "changeme");
	sprintf(theSettings.password, "hackme");
	theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	theSettings.position = BPoint(300, 300);
	theSettings.height = 600;
	theSettings.useTabs = true;
	theSettings.enablePublic = false;
	theSettings.saveSearches = false;
	theSettings.textSize = BFont().Size();
	theSettings.saveTweets = false;
	
	BPath path;
	
	if (_getSettingsPath(path) < B_OK)
		return;	
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK)
		return;

	file.ReadAt(0, &theSettings, sizeof(twitter_settings));
	
	if(theSettings.refreshTime < 0 || theSettings.refreshTime > 10000) {
		std::cout << "Bad refreshtime, reverting to defaults." << std::endl;
		theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	}
}

status_t
HTGAccountSettingsWindow::_saveSettings()
{
	if (strcmp(usernameView->Text(), theSettings.username) == 0 && ((savedSearchesBox->Value() == B_CONTROL_ON) == theSettings.saveSearches) && (strcmp(passwordView->Text(), theSettings.password) == 0 || strlen(passwordView->Text()) <= 0) && theSettings.refreshTime == atoi(refreshView->Text()))
		return B_OK;
		
	BAlert *theAlert = new BAlert("Please restart!", "You must restart HaikuTwitter for the changes to take place.", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);	
	int32 button_index = theAlert->Go();
		
	sprintf(theSettings.username, usernameView->Text());
	if(strlen(passwordView->Text()) > 0)
		sprintf(theSettings.password, passwordView->Text());
	theSettings.refreshTime = atoi(refreshView->Text());
	theSettings.saveSearches = (savedSearchesBox->Value() == B_CONTROL_ON);
	
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
