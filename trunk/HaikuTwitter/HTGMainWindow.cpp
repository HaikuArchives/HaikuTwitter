/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGMainWindow.h"

HTGMainWindow::HTGMainWindow(string username, string password, int refreshTime) : BWindow(BRect(300, 300, 615, 900), "HaikuTwitter (Alpha)", B_TITLED_WINDOW, B_NOT_H_RESIZABLE) {	
	this->username = username;
	this->password = password;
	this->refreshTime = refreshTime;
	
	newTweetObj = new twitCurl();
	newTweetObj->setTwitterUsername( username );
	newTweetObj->setTwitterPassword( password );
	
	/*Set up the menu bar*/
	_SetupMenu();
	
	/*Set up tab view*/
	tabView = new BTabView(BRect(0, 20, 315, 600), "TabView");
	this->AddChild(tabView);
	
	/*Set up home timeline*/	//The parser does not support home timeline yet.
	/*twitCurl *homeTwitObj = new twitCurl();
	homeTwitObj->setTwitterUsername( username );
    homeTwitObj->setTwitterPassword( password );
	homeTimeLine = new HTGTimeLineView(homeTwitObj, TIMELINE_HOME);
	tabView->AddTab(homeTimeLine);*/
	
	/*Set up friends timeline*/
	twitCurl *timelineTwitObj = new twitCurl();
	timelineTwitObj->setTwitterUsername( username );
    timelineTwitObj->setTwitterPassword( password );
	friendsTimeLine = new HTGTimeLineView(timelineTwitObj, TIMELINE_FRIENDS);
	tabView->AddTab(friendsTimeLine);
	
	/*Set up mentions timeline*/
	twitCurl *mentionsTwitObj = new twitCurl();
	mentionsTwitObj->setTwitterUsername( username );
    mentionsTwitObj->setTwitterPassword( password );
	mentionsTimeLine = new HTGTimeLineView(mentionsTwitObj, TIMELINE_MENTIONS);
	tabView->AddTab(mentionsTimeLine);
	
	/*Set up public timeline - Weird symbols on the public timeline, probably a language with non UTF-8 symbols*/
	twitCurl *publicTwitObj = new twitCurl();
	publicTimeLine = new HTGTimeLineView(publicTwitObj, TIMELINE_PUBLIC);
	tabView->AddTab(publicTimeLine);
		
	/*Fire a REFRESH message every 'refreshTime' minute*/
	std::cout << refreshTime << std::endl;
	BMessageRunner *refreshTimer = new BMessageRunner(this, new BMessage(REFRESH), refreshTime*1000000*60);
}

bool HTGMainWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void HTGMainWindow::showAbout() {
	std::string text("HaikuTwitter Alpha (rev. ");
	text.append(SVN_REV);
	text.append(")\n");
	text.append("\tWritten by Martin Hebnes Pedersen\n"
				"\tCopyright 2010, All rights reserved.\n"
				"\t\n"
				"\thttp://martinhpedersen.mine.nu/\n"
				"\t@martinhpedersen\n"
				"\t\n"
				"\tDistributed under the terms of the MIT License.\n");
				
	BAlert *alert = new BAlert("about", text.c_str(), "OK");
	BTextView *view = alert->TextView();
	BFont font;

	view->SetStylable(true);
	
	view->GetFont(&font);
	font.SetSize(10);
	view->SetFontAndColor(text.length()-48, text.length(), &font);
	font.SetSize(18);
	font.SetFace(B_BOLD_FACE);
	view->SetFontAndColor(0, 18, &font);

	alert->Go();
}

void HTGMainWindow::_SetupMenu() {
	/*Menu bar object*/
	fMenuBar = new BMenuBar(Bounds(), "mbar");
	
	/*Make Twitter Menu*/
	fTwitterMenu = new BMenu("Twitter");
	fTwitterMenu->AddItem(new BMenuItem("New tweet...", new BMessage(NEW_TWEET), 'N'));
	//fTwitterMenu->AddItem(new BMenuItem("Go to user...", new BMessage(GO_USER), 'G')); //This is not implemented yet.
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("Refresh", new BMessage(REFRESH), 'R'));
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("About HaikuTwitter...", new BMessage(ABOUT)));
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(fTwitterMenu);
	
	/*Make Edit Menu*/
	fEditMenu = new BMenu("Edit");
	//fEditMenu->AddItem(new BMenuItem("Copy", new BMessage(B_COPY), 'C')); //This is not implemented yet.
	fMenuBar->AddItem(fEditMenu);
	
	/*Make Settings Menu*/
	fSettingsMenu = new BMenu("Settings");
	fSettingsMenu->AddItem(new BMenuItem("Account...", new BMessage(ACCOUNT_SETTINGS)));
	fMenuBar->AddItem(fSettingsMenu);
	
	AddChild(fMenuBar);
}

void HTGMainWindow::MessageReceived(BMessage *msg) {
	const char* text_label = "text";
	switch(msg->what) {
		case NEW_TWEET:
			newTweetWindow = new HTGNewTweetWindow(newTweetObj);
			newTweetWindow->SetText(msg->FindString(text_label, (int32)0)); //Set text (RT, reply, ie)
			newTweetWindow->Show();
			break;
		case REFRESH:
			//homeTimeLine->updateTimeLine(); //The parser does not support home timeline yet.
			friendsTimeLine->updateTimeLine();
			mentionsTimeLine->updateTimeLine();
			publicTimeLine->updateTimeLine();
			break;
		case GO_USER:
			timeLineWindow = new HTGTimeLineWindow(this, username, password, refreshTime, TIMELINE_USER, msg->FindString(text_label, (int32)0));
			timeLineWindow->Show();
			break;
		case GO_SEARCH:
			std::cout << "Search not implemented yet..." << std::endl;
			break;
		case ACCOUNT_SETTINGS:
			accountSettingsWindow = new HTGAccountSettingsWindow();
			accountSettingsWindow->Show();
			break;
		case ABOUT:
			showAbout();
			break;
		case B_CLOSE_REQUESTED:
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}

HTGMainWindow::~HTGMainWindow() {
	
	be_app->PostMessage(B_QUIT_REQUESTED);
}
