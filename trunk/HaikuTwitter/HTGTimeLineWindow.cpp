/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTimeLineWindow.h"

HTGTimeLineWindow::HTGTimeLineWindow(BWindow *parent, string username, string password, int refreshTime, const int32 TYPE, const char* requestInfo) : BWindow(BRect(300, 300, 615, 840), requestInfo, B_TITLED_WINDOW, B_NOT_H_RESIZABLE) {	
	/*Set parent window (used for handeling messages)*/
	this->parent = parent;
	
	/*Set up timeline*/
	twitCurl *timelineTwitObj = new twitCurl();
	timelineTwitObj->setTwitterUsername( username );
    timelineTwitObj->setTwitterPassword( password );
	theTimeLine = new HTGTimeLineView(timelineTwitObj, TYPE, BRect(0, 0, 325, 560), requestInfo);
	this->AddChild(theTimeLine);
		
	/*Fire a REFRESH message every 'refreshTime' minute*/
	refreshTimer = new BMessageRunner(this, new BMessage(REFRESH), refreshTime*1000000*60);
}

void HTGTimeLineWindow::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case REFRESH:
			theTimeLine->updateTimeLine();;
			break;
		default:
			parent->MessageReceived(msg);
			break;
	}
}

HTGTimeLineWindow::~HTGTimeLineWindow() {
	theTimeLine->RemoveSelf();
	delete theTimeLine;
	delete refreshTimer;
}
