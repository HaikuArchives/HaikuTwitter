/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HaikuTwitter.h"

HaikuTwitter::HaikuTwitter()
	: BApplication ("application/x-vnd.HaikuTwitter")
{
	/*Make mimetype*/
	HTStorage::makeMimeType(true);
	HTStorage::makeIndices();
	
	/*Get configuration*/
	struct twitter_settings theSettings = retrieveSettings();
	struct oauth_settings oauth = retrieveOAuth();
    std::string key(oauth.key);
    std::string secret(oauth.secret);
		
	/*Display timeline or authorize*/
	if(key.length() < 5 || secret.length() < 5) {
			HTGAuthorizeWindow *theWindow = new HTGAuthorizeWindow(theSettings.refreshTime, theSettings.position, theSettings.height);
			theWindow->Show();
	}
	else {
		mainWindow= new HTGMainWindow(key, secret, theSettings.refreshTime, theSettings.position, theSettings.height);
		mainWindow->Show();
	}
	
	tweetViewWindow = new HTGTweetViewWindow(mainWindow);
}

void
HaikuTwitter::RefsReceived(BMessage* message)
{
	int32 index = 0;
	entry_ref ref;
	BList* tweets = new BList();
	for(int i = 0; (message->FindRef("refs", i, &ref) == B_OK); i++) {
		HTTweet *theTweet = HTStorage::loadTweet(&ref);
		if(theTweet != NULL)
			tweets->AddItem(theTweet);
	}

	if(!tweets->IsEmpty()) {
		if(tweetViewWindow == NULL)
			tweetViewWindow = new HTGTweetViewWindow(mainWindow);
		tweetViewWindow->AddList(tweets);
		tweetViewWindow->Show();
	}
	else
		HTGErrorHandling::displayError("An unexpected error occurred.");
}

void
HaikuTwitter::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case TWEETVIEWWINDOW_CLOSED:
			tweetViewWindow = NULL;
			std::cout << "OK" << std::endl;
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

HaikuTwitter::~HaikuTwitter()
{
	//Nothing to do
}

status_t
getSettingsPath(BPath &path) 
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	return B_OK;
}

struct twitter_settings
retrieveSettings() 
{
	twitter_settings theSettings;
	
	/*Set the defaults, just in case anything bad happens*/
	sprintf(theSettings.username, "changeme");
	sprintf(theSettings.password, "hackme");
	theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	theSettings.position = BPoint(300, 300);
	theSettings.height = 600;
	
	BPath path;
	
	if (getSettingsPath(path) < B_OK)
		return theSettings;	
	
	path.Append("HaikuTwitter_settings");
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK)
		return theSettings;

	file.ReadAt(0, &theSettings, sizeof(twitter_settings));
	
	if(theSettings.refreshTime <= 0 || theSettings.refreshTime > 10000) {
		std::cout << "Bad refreshtime, reverting to defaults." << std::endl;
		theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	}
	
	return theSettings;
}

struct oauth_settings
retrieveOAuth()
{
	oauth_settings theSettings;
	
	/*Set the defaults, just in case anything bad happens*/
	sprintf(theSettings.key, "");
	sprintf(theSettings.secret, "");
	
	BPath path;
	
	if (getSettingsPath(path) < B_OK)
		return theSettings;	
	
	path.Append("HaikuTwitter_oauth");
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK)
		return theSettings;

	file.ReadAt(0, &theSettings, sizeof(oauth_settings));
	
	return theSettings;
}

int 
main(int argc, char** argv)
{   
	HaikuTwitter haikuTwitter;
	haikuTwitter.Run();
    return 0;
}
