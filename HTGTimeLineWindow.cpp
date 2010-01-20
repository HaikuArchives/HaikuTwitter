/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTimeLineWindow.h"

HTGTimeLineWindow::HTGTimeLineWindow(twitCurl *twitObj) : BWindow(BRect(300, 300, 615, 900), "HaikuTwitter (Timeline)", B_TITLED_WINDOW, 0) {	
	this->twitObj = twitObj;
	
	/*Set up the menu bar*/
	_SetupMenu();
	
	/*Get current timeline*/
	this->timeLineParser = new TimeLineParser();
	twitObj->timelineFriendsGet();
	std::string replyMsg(" ");
	twitObj->getLastWebResponse(replyMsg);
	timeLineParser->readData(replyMsg.c_str());
	
	/*Set up listview*/
	this->listView = new BListView(BRect(Bounds().left, Bounds().top, Bounds().right-15, 72*20), "ListView");
	
	/*Set up scrollview*/
	containerView = new BView(BRect(Bounds().left, Bounds().top+20, Bounds().right-15, Bounds().bottom), "ContainerView", B_FOLLOW_LEFT | B_FOLLOW_TOP, 0);
	containerView->AddChild(listView);
	this->scrollView = new BScrollView("ScrollView", containerView, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true, B_FANCY_BORDER);
	this->AddChild(scrollView);
	
	/*Fill the listView with tweets*/
	for (int i = 0; i < timeLineParser->count(); i++) {
		HTGTweetItem *currentTweet = new HTGTweetItem(timeLineParser->getTweets()[i]);
		this->listView->AddItem(currentTweet);
	}
	
	/*Clean up*/
	delete timeLineParser;
	timeLineParser = NULL;
}

void HTGTimeLineWindow::updateTimeLine() {
	//Could not figure out how to update a BListItem with a child view (BTextView).
	//Could be a bug in Haiku API's. After hours of investigation without any
	//result, I just don't care anymore. Reallocating all HTGTweetItem on update.
			
	twitObj->timelineFriendsGet();
	std::string replyMsg(" ");
	twitObj->getLastWebResponse(replyMsg);
	if(timeLineParser == NULL) {
		timeLineParser = new TimeLineParser();
	}
	timeLineParser->readData(replyMsg.c_str());
	
	HTGTweetItem *mostRecentItem = (HTGTweetItem *)listView->FirstItem();
	HTTweet *mostRecentTweet = mostRecentItem->getTweetPtr();
	HTTweet *currentTweet = timeLineParser->getTweets()[0];
	
	/*If we are up to date, don't do anything more*/
	if(!(*mostRecentTweet < *currentTweet)) {
		delete timeLineParser;
		timeLineParser = NULL;
		return ;
	}
		
	BList *newList = new BList();
	
	for (int i = 0; i < timeLineParser->count(); i++) {
		currentTweet = timeLineParser->getTweets()[i];
		if(*mostRecentTweet < *currentTweet) {
			newList->AddItem(new HTGTweetItem(currentTweet));
			std::cout << "Added a new item" << std::endl;
		}
		else
			break;
	}
	
	while(!listView->IsEmpty()) {
		HTGTweetItem *currentItem = (HTGTweetItem *)listView->FirstItem();
		currentTweet = currentItem->getTweetPtr();
		listView->RemoveItem(currentItem);
		delete currentItem;
		newList->AddItem(new HTGTweetItem(currentTweet));
	}
	listView->Invalidate();
	listView->AddList(newList);
	delete timeLineParser;
	timeLineParser = NULL;
}

bool HTGTimeLineWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void HTGTimeLineWindow::_SetupMenu() {
	/*Menu bar object*/
	fMenuBar = new BMenuBar(Bounds(), "mbar");
	
	/*Make Twitter Menu*/
	fTwitterMenu = new BMenu("Twitter");
	fTwitterMenu->AddItem(new BMenuItem("New tweet", new BMessage(NEW_TWEET), 'N'));
	fTwitterMenu->AddItem(new BMenuItem("Refresh", new BMessage(REFRESH), 'R'));
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("About HaikuTwitter...", new BMessage(ABOUT)));
	fTwitterMenu->AddSeparatorItem();
	fTwitterMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(fTwitterMenu);
	
	/*Make Edit Menu*/
	fEditMenu = new BMenu("Edit");
	fEditMenu->AddItem(new BMenuItem("Copy", new BMessage(B_COPY), 'C')); //This is not implemented yet.
	fMenuBar->AddItem(fEditMenu);
	
	/*Make Settings Menu*/
	fSettingsMenu = new BMenu("Settings");
	fSettingsMenu->AddItem(new BMenuItem("Account...", new BMessage(ACCOUNT_SETTINGS)));
	fMenuBar->AddItem(fSettingsMenu);
	
	AddChild(fMenuBar);
}

void HTGTimeLineWindow::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case NEW_TWEET:
			newTweetWindow = new HTGNewTweetWindow(twitObj);
			newTweetWindow->Show();
			break;
		case REFRESH:
			this->updateTimeLine();
			break;
		case ACCOUNT_SETTINGS:
			accountSettingsWindow = new HTGAccountSettingsWindow();
			accountSettingsWindow->Show();
			break;
		case ABOUT:
			aboutWindow = new HTGAboutWindow();
			aboutWindow->Show();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}

HTGTimeLineWindow::~HTGTimeLineWindow() {
	containerView->RemoveSelf();
	delete containerView;
	delete timeLineParser;
	
	be_app->PostMessage(B_QUIT_REQUESTED);
}
