/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGAuthorizeWindow.h"

HTGAuthorizeWindow::HTGAuthorizeWindow(int refreshTime, BPoint position, int height)
	: BWindow(BRect(200, 200, 565, 405), "Authorize with twitter.com", B_TITLED_WINDOW, B_NOT_RESIZABLE)
{
	CenterOnScreen();
	
	/*These will be used to open the main window*/
	this->refreshTime = refreshTime;
	this->position = position;
	this->height = height;
	
	twitObj = new twitCurl();
	
	/*Add a grey view*/
	theView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	theView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	//theView->SetViewColor(128, 128, 128);
	this->AddChild(theView);
	
	/* Add logo */
	logoView = new HTGLogoView();
	logoView->MoveTo(BPoint(155, 10));
	logoView->ResizeTo(logoView->MinSize());
	theView->AddChild(logoView);
	
	/*Set up header text*/
	headerText = new BTextView(BRect(60, logoView->Bounds().Height()+5, 390, logoView->Bounds().Height()+5+30), "Heading", BRect(0, 0, 390, 50), B_FOLLOW_TOP, B_WILL_DRAW);
	headerText->SetText("Welcome to HaikuTwitter!");
	BFont headerFont;
	theView->GetFont(&headerFont);
	headerFont.SetEncoding(B_UNICODE_UTF8);
	headerFont.SetSize(18);
	headerText->SetFontAndColor(&headerFont);
	headerText->SetViewColor(theView->ViewColor());
	theView->AddChild(headerText);
	
	openButton = new BButton(BRect(100, 120, 260, 150), NULL, "Log in to twitter.com", new BMessage(GO_TO_AUTH_URL));
	theView->AddChild(openButton);
	
	
	/*Set up text control*/
	query = new BTextControl(BRect(100,120,260,150), "Enter PIN", "Enter PIN:", NULL, NULL);
	
	/*Set up go button*/
	goButton = new BButton(BRect(130, 170, 230, 200), NULL, "Let's tweet!", new BMessage(GO_AUTH));
	
	
	query->MakeFocus();
	openButton->MakeDefault(true);
}

void
HTGAuthorizeWindow::openUrl(std::string url)
{	
	char* args[] = { (char *)url.c_str(), NULL };
	const char* mime = "application/x-vnd.Be.URL.http";
	
	int32 launched = be_roster->Launch(mime, 1, args);
	if(launched != B_OK && launched != B_ALREADY_RUNNING) {
		copyToClipboard(url.c_str());
		HTGErrorHandling::displayError("Unable to launch your web browser...\nAn url has been copied to your clipboard.\n\n Please paste this link into your web browser and follow the directions.\n");
	}
}

void
HTGAuthorizeWindow::copyToClipboard(const char* theString)
{
	BMessage *clip = (BMessage *)NULL;
	if (be_clipboard->Lock()) {
		be_clipboard->Clear();
		if (clip = be_clipboard->Data()) {
			clip->AddData("text/plain", B_MIME_TYPE, theString, strlen(theString));
			be_clipboard->Commit();
		}
	be_clipboard->Unlock();
	}
}

void
HTGAuthorizeWindow::storeTokens(std::string key, std::string secret)
{
	sprintf(oauth.key, key.c_str());
	sprintf(oauth.secret, secret.c_str());
	
	BPath path;
	status_t status = _getSettingsPath(path);
	if (status < B_OK)
		return ;
		
	BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE);
	if (status < B_OK)
		return ;
		
	file.WriteAt(0, &oauth, sizeof(oauth_settings));
	std::cout << "Settings saved" << std::endl;
}

status_t
HTGAuthorizeWindow::_getSettingsPath(BPath &path)
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("HaikuTwitter_oauth");
	return B_OK;
}

void
HTGAuthorizeWindow::MessageReceived(BMessage *msg)
{
	const char* text_label = "text";
	switch(msg->what) {
		case GO_TO_AUTH_URL: {
			goButton->MakeFocus();
			std::string url(twitObj->oauthGetAuthorizeUrl());
			if(url.length() < 10)
				HTGErrorHandling::displayError("Error while requesting authorization URL.\nPlease try again!\n\nPlease note that system time must be set correctly.\n");
			else {
				openUrl(url);
				openButton->RemoveSelf();
				theView->AddChild(query);
				theView->AddChild(goButton);
				query->WindowActivated(true);
			}
			break;
		}
		case GO_AUTH: {
			if(!twitObj->oauthAuthorize(query->Text())) {
				query->RemoveSelf();
				goButton->RemoveSelf();
				theView->AddChild(openButton);
				HTGErrorHandling::displayError("Could not confirm your PIN.\nPlease try again from the top!");
			}
			else {
				storeTokens(twitObj->getAccessKey(), twitObj->getAccessSecret());
				be_app->PostMessage(new BMessage(AUTHORIZATION_DONE));
				this->Close();
			}
		}
		default:
			BWindow::MessageReceived(msg);
	}
}

HTGAuthorizeWindow::~HTGAuthorizeWindow()
{
	query->RemoveSelf();
	delete query;
	
	goButton->RemoveSelf();
	delete goButton;
	
	openButton->RemoveSelf();
	delete openButton;
	
	logoView->RemoveSelf();
	delete logoView;
	
	theView->RemoveSelf();
	delete theView;
}
