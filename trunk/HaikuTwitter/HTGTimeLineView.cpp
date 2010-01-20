/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTimeLineView.h"

HTGTimeLineView::HTGTimeLineView(twitCurl *twitObj, const int32 TYPE) : BScrollView("Loading...", new BView(BRect(0, 0, 300-4, 580), "ContainerView", B_FOLLOW_LEFT | B_FOLLOW_TOP, 0), B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true, B_FANCY_BORDER) {	
	this->twitObj = twitObj;
	this->TYPE = TYPE;
	
	/*Get current timeline based on TYPE*/
	this->timeLineParser = new TimeLineParser();
	switch(TYPE) {
		case TIMELINE_FRIENDS:
			this->SetName("Timeline");
			twitObj->timelineFriendsGet();
			break;
		case TIMELINE_MENTIONS:
			this->SetName("Mentions");
			twitObj->mentionsGet();
			break;
		case TIMELINE_PUBLIC:
			this->SetName("Public");
			twitObj->timelinePublicGet();
			break;
		default:
			this->SetName("Public");
			twitObj->timelinePublicGet();
	}
	std::string replyMsg(" ");
	twitObj->getLastWebResponse(replyMsg);
	timeLineParser->readData(replyMsg.c_str());
	
	/*Set up listview*/
	this->listView = new BListView(BRect(0, 0, 300-5, 72*20), "ListView");
	
	/*Set up scrollview*/
	containerView = new BView(Bounds(), "ContainerView", B_FOLLOW_LEFT | B_FOLLOW_TOP, 0);
	containerView->AddChild(listView);
	this->SetTarget(containerView);
	
	/*Fill the listView with tweets*/
	for (int i = 0; i < timeLineParser->count(); i++) {
		HTGTweetItem *currentTweet = new HTGTweetItem(timeLineParser->getTweets()[i]);
		this->listView->AddItem(currentTweet);
	}
	
	/*Clean up*/
	delete timeLineParser;
	timeLineParser = NULL;
}

void HTGTimeLineView::updateTimeLine() {
	//Could not figure out how to update a BListItem with a child view (BTextView).
	//Could be a bug in Haiku API's. After hours of investigation without any
	//result, I just don't care anymore. Reallocating all HTGTweetItem on update.
	
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
		default:
			twitObj->timelinePublicGet();
	}	
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

HTGTimeLineView::~HTGTimeLineView() {
	containerView->RemoveSelf();
	delete containerView;
	delete timeLineParser;
	
}
