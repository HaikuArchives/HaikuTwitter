/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTimeLineView.h"

HTGTimeLineView::HTGTimeLineView(twitCurl *twitObj, const int32 TYPE, const char* requestInfo) : BScrollView("Loading...", new BView(BRect(0, 0, 300-4, 552), "ContainerView", B_FOLLOW_LEFT | B_FOLLOW_TOP, 0), B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true, B_FANCY_BORDER) {	
	this->twitObj = twitObj;
	this->TYPE = TYPE;
	thread_id previousThread = B_NAME_NOT_FOUND;
	
	/*Set view name*/
	switch(TYPE) {
		case TIMELINE_FRIENDS:
			SetName("Timeline");
			break;
		case TIMELINE_MENTIONS:
			SetName("Mentions");
			break;
		case TIMELINE_PUBLIC:
			SetName("Public");
			break;
		case TIMELINE_USER:
			SetName(requestInfo);
			break;
		case TIMELINE_SEARCH:
			SetName(requestInfo);
			break;
		default:
			SetName("Public");
	}
	
	/*Set up listview*/
	this->listView = new BListView(BRect(0, 0, 300-5, 72*20), "ListView");
	
	/*Set up scrollview*/
	containerView = new BView(Bounds(), "ContainerView", B_FOLLOW_LEFT | B_FOLLOW_TOP, 0);
	containerView->AddChild(listView);
	this->SetTarget(containerView);
	
	waitingForUpdate = true;
}

void HTGTimeLineView::AttachedToWindow() {
	BScrollView::AttachedToWindow();
	if(waitingForUpdate)
		updateTimeLine();
}

void HTGTimeLineView::updateTimeLine() {
	/*Update timeline only if we can lock window looper*/
	if(!listView->LockLooper())
		waitingForUpdate = true;
	else {
		listView->UnlockLooper(); //Assume we can lock it later
		previousThread = spawn_thread(updateTimeLineThread, "UpdateThread", 10, this);
		resume_thread(previousThread);
		waitingForUpdate = false;
	}
}

std::string& htmlFormatedString(const char *orig) {
	std::string newString(orig);
	if(orig[0] == '#') {
		newString = std::string(orig+1);
		newString.insert(0, "%23");
	}
	std::string *returnPtr = new std::string(newString);
	return *returnPtr;
}

status_t updateTimeLineThread(void *data) {
	//Could not figure out how to update a BListItem with a child view (BTextView).
	//Could be a bug in Haiku API's. After hours of investigation without any
	//result, I just don't care anymore. Reallocating all HTGTweetItem on update.
	
	HTGTimeLineView *super = (HTGTimeLineView*)data;
	
	/*Wait for previous thread to end*/
	status_t junkId;
	wait_for_thread(find_thread("UpdateThread"), &junkId);
	
	BListView *listView = super->listView;
	BView *containerView;
	char *tabName;
		
	TimeLineParser *timeLineParser = new TimeLineParser();
	twitCurl *twitObj = super->twitObj;
	int32 TYPE = super->TYPE;
	
	switch(TYPE) {
		case TIMELINE_FRIENDS:
			twitObj->timelineFriendsGet();
			break;
		case TIMELINE_MENTIONS:
			twitObj->mentionsGet();
			break;
		case TIMELINE_PUBLIC:
			twitObj->timelinePublicGet();
			break;
		case TIMELINE_USER:
			twitObj->timelineUserGet(*new std::string(super->Name()), false);
			break;
		case TIMELINE_SEARCH:
			twitObj->search(htmlFormatedString(super->Name()));
			break;
		default:
			twitObj->timelinePublicGet();
	}	
	std::string replyMsg(" ");
	twitObj->getLastWebResponse(replyMsg);
	if(replyMsg.length() < 100)  { //Length of data is less than 100 characters. Clearly,
		replyMsg = "error";				//something is wrong... abort.
	}
		
	timeLineParser->readData(replyMsg.c_str());
	
	HTGTweetItem *mostRecentItem;
	HTTweet *mostRecentTweet;
	HTTweet *currentTweet;
	
	bool initialLoad = (listView->FirstItem() == NULL);
	
	if(!initialLoad) {
		mostRecentItem = (HTGTweetItem *)listView->FirstItem();
		mostRecentTweet = mostRecentItem->getTweetPtr();
		currentTweet = timeLineParser->getTweets()[0];
	
		/*If we are up to date, don't do anything more*/
		if(!(*mostRecentTweet < *currentTweet)) {
			delete timeLineParser;
			timeLineParser = NULL;
			return B_OK;
		}
	}
		
	BList *newList = new BList();
	
	for (int i = 0; i < timeLineParser->count(); i++) {
		currentTweet = timeLineParser->getTweets()[i];
		bool addItem = initialLoad;
		if(!initialLoad)
			addItem = (*mostRecentTweet < *currentTweet);
		if(addItem) {
			/*Make a copy, download bitmap and add it to newList*/
			HTTweet *copiedTweet = new HTTweet(currentTweet);
			copiedTweet->downloadBitmap();
			newList->AddItem(new HTGTweetItem(copiedTweet));
		}
		else
			break;
	}
	
	/*Try to lock listView*/
	if(!listView->LockLooper()) {
		/*Not active view: Cleanup and return*/
		super->waitingForUpdate = true;
		delete timeLineParser;
		timeLineParser = NULL;
		delete newList;
		return B_OK;
	}
	
	HTGTweetItem *currentItem;
	while(!listView->IsEmpty()) {
		currentItem = (HTGTweetItem *)listView->FirstItem();
		currentTweet = currentItem->getTweetPtr();
		listView->RemoveItem(currentItem); //Must lock looper before we do this!
		if(newList->CountItems() < 20) //Only allow 20 tweets to be displayed at once... for now.
			newList->AddItem(new HTGTweetItem(new HTTweet(currentTweet)));
			
		delete currentItem;
	}
	
	/*Update the view*/
	listView->AddList(newList); //Must lock looper before we do this!
			
	/*Cleanup*/
	super->waitingForUpdate = false;
	listView->UnlockLooper();
	delete timeLineParser;
	timeLineParser = NULL;
	delete newList;
	
	return B_OK;
}

HTGTimeLineView::~HTGTimeLineView() {
	/*Kill the update thread*/
	kill_thread(previousThread);
	
	listView->RemoveSelf();
	while(!listView->IsEmpty()) {
		HTGTweetItem *currentItem = (HTGTweetItem *)listView->FirstItem();
		listView->RemoveItem(currentItem);			
		delete currentItem;
	}
	delete listView;
	delete twitObj;
	containerView->RemoveSelf();
	delete containerView;
}
