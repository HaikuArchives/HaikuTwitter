/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGAuthorizeWindow.h"

HTGAuthorizeWindow::HTGAuthorizeWindow(int refreshTime, BPoint position, int height) : BWindow(BRect(200, 200, 565, 475), "Authorize with twitter.com", B_TITLED_WINDOW, B_NOT_RESIZABLE) {
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
	HTGLogoView *logoView = new HTGLogoView();
	logoView->MoveTo(BPoint(155, 10));
	logoView->ResizeTo(logoView->MinSize());
	theView->AddChild(logoView);
	
	/*Set up header text*/
	BTextView *headerText = new BTextView(BRect(60, logoView->Bounds().Height()+5, 390, logoView->Bounds().Height()+5+30), "Heading", BRect(0, 0, 390, 50), B_FOLLOW_TOP, B_WILL_DRAW);
	headerText->SetText("Welcome to HaikuTwitter!");
	BFont headerFont;
	theView->GetFont(&headerFont);
	headerFont.SetEncoding(B_UNICODE_UTF8);
	headerFont.SetSize(18);
	headerText->SetFontAndColor(&headerFont);
	headerText->SetViewColor(theView->ViewColor());
	theView->AddChild(headerText);
	
	BButton *openButton = new BButton(BRect(100, 120, 260, 150), NULL, "Log in to twitter.com", new BMessage(GO_TO_AUTH_URL));
	theView->AddChild(openButton);
	
	
	/*Set up text control*/
	query = new BTextControl(BRect(100,165,260,185), "Enter PIN", "Enter PIN:", NULL, NULL);
	theView->AddChild(query);
	query->WindowActivated(true);
	
	goButton = new BButton(BRect(130, 230, 230, 270), NULL, "Let's tweet!", new BMessage(GO_AUTH));
	theView->AddChild(goButton);
	
	/*Set up buttons*/
	/*goButton = new BButton(BRect(170, 30, 247, 55), NULL, "Go", new BMessage(GO_TO_AUTH_URL));
	theView->AddChild(goButton);*/
	
	query->MakeFocus();
	openButton->MakeDefault(true);
}

void HTGAuthorizeWindow::openUrl(std::string url) {
	entry_ref ref;
	if (get_ref_for_path("/bin/open", &ref))
		return;
		
	const char* args[] = { "/bin/open", url.c_str(), NULL };
	be_roster->Launch(&ref, 2, args);
}

void HTGAuthorizeWindow::storeTokens(std::string key, std::string secret) {
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

status_t HTGAuthorizeWindow::_getSettingsPath(BPath &path) {
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("HaikuTwitter_oauth");
	return B_OK;
}

void HTGAuthorizeWindow::MessageReceived(BMessage *msg) {
	const char* text_label = "text";
	switch(msg->what) {
		case GO_TO_AUTH_URL: {
			goButton->MakeFocus();
			std::string url(twitObj->oauthGetAuthorizeUrl());
			if(url.length() < 10)
				HTGErrorHandling::displayError("Error while requesting authorization URL.\nPlease try again!\n\nPlease note that system time must be set correctly.\n");
			else
				openUrl(url);
			break;
		}
		case GO_AUTH: {
			if(!twitObj->oauthAuthorize(query->Text()))
				HTGErrorHandling::displayError("Could not confirm your PIN.\nPlease try again from the top!");
			else {
				storeTokens(twitObj->getAccessKey(), twitObj->getAccessSecret());
				HTGMainWindow *theWindow = new HTGMainWindow(oauth.key, oauth.secret, refreshTime, position, height);
				theWindow->Show();
				this->Close();
			}
		}
		default:
			BWindow::MessageReceived(msg);
	}
}

HTGAuthorizeWindow::~HTGAuthorizeWindow() {
	query->RemoveSelf();
	delete query;
	
	goButton->RemoveSelf();
	delete goButton;
	
	theView->RemoveSelf();
	delete theView;
}
