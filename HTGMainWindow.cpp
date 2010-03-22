/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGMainWindow.h"

HTGMainWindow::HTGMainWindow(string username, string password, int refreshTime, BPoint position, int height) : BWindow(BRect(position.x, position.y, position.x+315, position.y+height), "HaikuTwitter (Alpha)", B_TITLED_WINDOW, B_NOT_H_RESIZABLE) {	
	this->username = username;
	this->password = password;
	this->refreshTime = refreshTime;
	
	_retrieveSettings();
	
	newTweetObj = new twitCurl();
	newTweetObj->setTwitterUsername( username );
	newTweetObj->setTwitterPassword( password );
	
	/*Set up the menu bar*/
	_SetupMenu();
	
	/*Set up tab view*/
	BRect tabViewRect(Bounds().left, fMenuBar->Bounds().bottom, Bounds().right, Bounds().bottom);
	tabView = new SmartTabView(tabViewRect, "TabView", B_WIDTH_FROM_LABEL);
	this->AddChild(tabView);
	
	/*Set up friends timeline*/
	twitCurl *timelineTwitObj = new twitCurl();
	timelineTwitObj->setTwitterUsername( username );
    timelineTwitObj->setTwitterPassword( password );
	friendsTimeLine = new HTGTimeLineView(timelineTwitObj, TIMELINE_FRIENDS, Bounds());
	tabView->AddTab(friendsTimeLine);
	
	/*Set up mentions timeline*/
	twitCurl *mentionsTwitObj = new twitCurl();
	mentionsTwitObj->setTwitterUsername( username );
    mentionsTwitObj->setTwitterPassword( password );
	mentionsTimeLine = new HTGTimeLineView(mentionsTwitObj, TIMELINE_MENTIONS, Bounds());
	tabView->AddTab(mentionsTimeLine);
	
	/*Set up public timeline - Weird symbols on the public timeline, probably a language with non UTF-8 symbols*/
	twitCurl *publicTwitObj = new twitCurl();
	publicTimeLine = new HTGTimeLineView(publicTwitObj, TIMELINE_PUBLIC, Bounds());
	tabView->AddTab(publicTimeLine);

	/*Fire a REFRESH message every 'refreshTime' minute*/
	BMessageRunner *refreshTimer = new BMessageRunner(this, new BMessage(REFRESH), refreshTime*1000000*60);
}

bool HTGMainWindow::QuitRequested() {
	_retrieveSettings();
	_saveSettings();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

status_t HTGMainWindow::_getSettingsPath(BPath &path) {
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("HaikuTwitter_settings");
	return B_OK;
}

void HTGMainWindow::_retrieveSettings() {
	/*Set the defaults, just in case anything bad happens*/
	sprintf(theSettings.username, "changeme");
	sprintf(theSettings.password, "hackme");
	theSettings.refreshTime = 5; //Default refresh time: 5 minutes.
	theSettings.position = BPoint(300, 300);
	theSettings.height = 600;
	theSettings.useTabs = true;
	
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

status_t HTGMainWindow::_saveSettings() {	
	theSettings.height = this->Bounds().Height() -23;
	theSettings.position = BPoint(this->Frame().left, this->Frame().top);
	theSettings.useTabs = fOpenInTabsMenuItem->IsMarked();
	
	BPath path;
	status_t status = _getSettingsPath(path);
	if (status < B_OK)
		return status;
		
	BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE);
	if (status < B_OK)
		return status;
		
	file.WriteAt(0, &theSettings, sizeof(twitter_settings));
	std::cout << "Window position saved" << std::endl;
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
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("Go to user...", new BMessage(FIND_USER), 'G'));
	fTwitterMenu->AddItem(new BMenuItem("Search for...", new BMessage(SEARCH_FOR), 'F'));
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("Refresh", new BMessage(REFRESH), 'R'));
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("About HaikuTwitter...", new BMessage(ABOUT)));
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("Close active tab", new BMessage(CLOSE_TAB), 'W', B_SHIFT_KEY));
	fTwitterMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(fTwitterMenu);
	
	/*Make Edit Menu*/
	fEditMenu = new BMenu("Edit");
	//fEditMenu->AddItem(new BMenuItem("Copy", new BMessage(B_COPY), 'C')); //This is not implemented yet.
	fMenuBar->AddItem(fEditMenu);
	
	/*Make Settings Menu*/
	fSettingsMenu = new BMenu("Settings");
	fSettingsMenu->AddItem(new BMenuItem("Account...", new BMessage(ACCOUNT_SETTINGS)));
	#ifdef INFOPOPPER_SUPPORT
	fSettingsMenu->AddItem(new BMenuItem("InfoPopper...", new BMessage(INFOPOPPER_SETTINGS)));
	#endif
	fMenuBar->AddItem(fSettingsMenu);
	fSettingsMenu->AddSeparatorItem();
	fOpenInTabsMenuItem = new BMenuItem("Use tabs", new BMessage(TOGGLE_TABS));
	fSettingsMenu->AddItem(fOpenInTabsMenuItem);
	fOpenInTabsMenuItem->SetMarked(theSettings.useTabs);
	
	AddChild(fMenuBar);
}

void HTGMainWindow::MessageReceived(BMessage *msg) {
	const char* text_label = "text";
	switch(msg->what) {
		case TOGGLE_TABS:
			fOpenInTabsMenuItem->SetMarked(!fOpenInTabsMenuItem->IsMarked());
			theSettings.useTabs = fOpenInTabsMenuItem->IsMarked();
			break;
		case NEW_TWEET:
			newTweetWindow = new HTGNewTweetWindow(newTweetObj);
			newTweetWindow->SetText(msg->FindString(text_label, (int32)0)); //Set text (RT, reply, ie)
			newTweetWindow->Show();
			break;
		case REFRESH:
			friendsTimeLine->updateTimeLine();
			mentionsTimeLine->updateTimeLine();
			publicTimeLine->updateTimeLine();
			break;
		case FIND_USER:
			goToUserWindow = new HTGGoToUserWindow(this);
			goToUserWindow->Show();
			break;
		case SEARCH_FOR:
			searchForWindow = new HTGSearchForWindow(this);
			searchForWindow->Show();
			break;
		case GO_USER:
			if(!fOpenInTabsMenuItem->IsMarked()) {
				timeLineWindow = new HTGTimeLineWindow(this, username, password, refreshTime, TIMELINE_USER, msg->FindString(text_label, (int32)0));
				timeLineWindow->Show();
			}
			else if (LockLooper()) {
				newTabObj = new twitCurl();
				newTabObj->setTwitterUsername( username );
				newTabObj->setTwitterPassword( password );
				tabView->AddTab(new HTGTimeLineView(newTabObj, TIMELINE_USER, Bounds(), msg->FindString(text_label, (int32)0)));
				tabView->Select(tabView->CountTabs()-1); //Select the new tab
				UnlockLooper();
			}
			break;
		case GO_SEARCH:
			if(!fOpenInTabsMenuItem->IsMarked()) {
				timeLineWindow = new HTGTimeLineWindow(this, username, password, refreshTime, TIMELINE_SEARCH, msg->FindString(text_label, (int32)0));
				timeLineWindow->Show();
			}
			else if (LockLooper()) {
				newTabObj = new twitCurl();
				newTabObj->setTwitterUsername( username );
				newTabObj->setTwitterPassword( password );
				tabView->AddTab(new HTGTimeLineView(newTabObj, TIMELINE_SEARCH, Bounds(), msg->FindString(text_label, (int32)0)));
				tabView->Select(tabView->CountTabs()-1); //Select the new tab
				UnlockLooper();
			}
			break;
		case CLOSE_TAB:
			if(tabView->Selection() >= 3) //Don't close hardcoded tabs
				tabView->RemoveAndDeleteTab(tabView->Selection());
			break;
		case ACCOUNT_SETTINGS:
			accountSettingsWindow = new HTGAccountSettingsWindow();
			accountSettingsWindow->Show();
			break;
		case INFOPOPPER_SETTINGS:
			infopopperSettingsWindow = new HTGInfoPopperSettingsWindow();
			infopopperSettingsWindow->Show();
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
