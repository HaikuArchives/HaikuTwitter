/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGInfoPopperSettingsWindow.h"

HTGInfoPopperSettingsWindow::HTGInfoPopperSettingsWindow() : BWindow(BRect(100, 100, 500, 210), "Notification settings", B_TITLED_WINDOW, B_NOT_RESIZABLE) {
	_retrieveSettings();
	_setupWindow();
}

HTGInfoPopperSettingsWindow::~HTGInfoPopperSettingsWindow() {
	_saveSettings();
	
	friendsNotifyBox->RemoveSelf();
	mentionsNotifyBox->RemoveSelf();
	publicNotifyBox->RemoveSelf();
	revertButton->RemoveSelf();
	backgroundView->RemoveSelf();
	delete friendsNotifyBox;
	delete mentionsNotifyBox;
	delete publicNotifyBox;
	delete revertButton;
	delete backgroundView;
}

status_t HTGInfoPopperSettingsWindow::_getSettingsPath(BPath &path) {
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("HaikuTwitter_infopopper");
	return B_OK;
}

void HTGInfoPopperSettingsWindow::_setupWindow() {
	/*Add the view*/
	backgroundView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	backgroundView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(backgroundView);
	
	/*Add revertButton*/
	revertButton = new BButton(BRect(3, 85, 100, -1), NULL, "Revert", new BMessage(kRevert));
	backgroundView->AddChild(revertButton);
	
	/*Add the check boxes*/
	friendsNotifyBox = new BCheckBox(BRect(5, 5, 400, 15), "Friends checkbox", "Display notifications for friends timeline", new BMessage());
	mentionsNotifyBox = new BCheckBox(BRect(5, 25, 400, 35), "Mentions checkbox", "Display notifications for mentions", new BMessage());
	searchesNotifyBox = new BCheckBox(BRect(5, 45, 400, 55), "Searches checkbox", "Display notifications for searches", new BMessage());
	publicNotifyBox = new BCheckBox(BRect(5, 65, 400, 75), "Public checkbox", "Display notifications for public timeline (Not recommended)", new BMessage());
	backgroundView->AddChild(friendsNotifyBox);
	backgroundView->AddChild(mentionsNotifyBox);
	backgroundView->AddChild(searchesNotifyBox);
	backgroundView->AddChild(publicNotifyBox);
	
	/*Set values*/
	if(theSettings.friendsNotify)
		friendsNotifyBox->SetValue(B_CONTROL_ON);
	else
		friendsNotifyBox->SetValue(B_CONTROL_OFF);
	if(theSettings.mentionsNotify)
		mentionsNotifyBox->SetValue(B_CONTROL_ON);
	else
		mentionsNotifyBox->SetValue(B_CONTROL_OFF);
	if(theSettings.publicNotify)
		publicNotifyBox->SetValue(B_CONTROL_ON);
	else
		publicNotifyBox->SetValue(B_CONTROL_OFF);
	if(theSettings.searchesNotify)
		searchesNotifyBox->SetValue(B_CONTROL_ON);
	else
		searchesNotifyBox->SetValue(B_CONTROL_OFF);

}

void HTGInfoPopperSettingsWindow::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case kRevert:
			_retrieveSettings();
			
			if(theSettings.friendsNotify)
				friendsNotifyBox->SetValue(B_CONTROL_ON);
			else
				friendsNotifyBox->SetValue(B_CONTROL_OFF);
			if(theSettings.mentionsNotify)
				mentionsNotifyBox->SetValue(B_CONTROL_ON);
			else
				mentionsNotifyBox->SetValue(B_CONTROL_OFF);
			if(theSettings.publicNotify)
				publicNotifyBox->SetValue(B_CONTROL_ON);
			else
				publicNotifyBox->SetValue(B_CONTROL_OFF);
			if(theSettings.searchesNotify)
				searchesNotifyBox->SetValue(B_CONTROL_ON);
			else
				searchesNotifyBox->SetValue(B_CONTROL_OFF);
				
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}

void HTGInfoPopperSettingsWindow::_retrieveSettings() {
	BPath path;
	
	if (_getSettingsPath(path) < B_OK) {
		theSettings = _getDefaults();
		return;
	}
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK) {
		theSettings = _getDefaults();
		return;
	}
	
	file.ReadAt(0, &theSettings, sizeof(infopopper_settings));
}

infopopper_settings HTGInfoPopperSettingsWindow::_getDefaults() {
	infopopper_settings returnSettings;
	
	returnSettings.friendsNotify = false;
	returnSettings.mentionsNotify = true;
	returnSettings.searchesNotify = false;
	returnSettings.publicNotify = false;
	
	return returnSettings;
}

status_t HTGInfoPopperSettingsWindow::_saveSettings() {
	if (((friendsNotifyBox->Value() == B_CONTROL_ON) == theSettings.friendsNotify) && ((mentionsNotifyBox->Value() == B_CONTROL_ON) == theSettings.mentionsNotify) && ((publicNotifyBox->Value() == B_CONTROL_ON) == theSettings.publicNotify) && ((searchesNotifyBox->Value() == B_CONTROL_ON) == theSettings.searchesNotify))
		return B_OK;
		
	BAlert *theAlert = new BAlert("Please restart!", "You must restart HaikuTwitter for the changes to take place.", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);	
	int32 button_index = theAlert->Go();
	
	theSettings.friendsNotify = (friendsNotifyBox->Value() == B_CONTROL_ON);
	theSettings.mentionsNotify = (mentionsNotifyBox->Value() == B_CONTROL_ON);
	theSettings.searchesNotify = (searchesNotifyBox->Value() == B_CONTROL_ON);
	theSettings.publicNotify = (publicNotifyBox->Value() == B_CONTROL_ON);
	
	BPath path;
	status_t status = _getSettingsPath(path);
	if (status < B_OK)
		return status;
		
	BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE);
	if (status < B_OK)
		return status;
		
	file.WriteAt(0, &theSettings, sizeof(infopopper_settings));
	std::cout << "Settings saved" << std::endl;
}
