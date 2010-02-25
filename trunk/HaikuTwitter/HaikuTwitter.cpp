/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include <iostream>
#include <cstdlib>
#include <string.h>
#include <Application.h>

#include "twitcurl/twitcurl.h"

#include "HTGMainWindow.h"

status_t getSettingsPath(BPath &path);
struct twitter_settings retrieveSettings();

int main()
{   
	BApplication HaikuApp("application/HaikuTwitter");
	
	/*Get configuration*/
	struct twitter_settings theSettings = retrieveSettings();
    std::string username(theSettings.username);
    std::string password(theSettings.password);
    int refreshTime = theSettings.refreshTime;
	
	/*Display timeline*/
	HTGMainWindow *theWindow = new HTGMainWindow(username, password, refreshTime);
	theWindow->Show();
	
	HaikuApp.Run();
    return 0;
}

status_t getSettingsPath(BPath &path) {
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("HaikuTwitter_settings");
	return B_OK;
}

struct twitter_settings retrieveSettings() {
	struct twitter_settings theSettings;
	
	/*Set the defaults, just in case anything bad happens*/
	sprintf(theSettings.username, "changeme");
	sprintf(theSettings.password, "hackme");
	theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	
	BPath path;
	
	if (getSettingsPath(path) < B_OK)
		return theSettings;	
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK)
		return theSettings;

	file.ReadAt(0, &theSettings, sizeof(twitter_settings));
	
	if(theSettings.refreshTime < 0 || theSettings.refreshTime > 10000) {
		std::cout << "Bad refreshtime, reverting to defaults." << std::endl;
		theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	}
	
	return theSettings;
}
