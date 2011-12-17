/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTweetViewWindow.h"

HTGTweetViewWindow::HTGTweetViewWindow(BWindow *parent, BList *tweets)
	: BWindow(BRect(300, 300, 615, 840), "Tweet Viewer", B_TITLED_WINDOW, B_NOT_H_RESIZABLE)
{
	/*Set parent window (used for handeling messages)*/
	this->parent = parent;
	
	/*Set up timeline*/
	theTimeLine = new HTGTimeLineView(TIMELINE_HDD, Bounds(), tweets);
	this->AddChild(theTimeLine);
}

void
HTGTweetViewWindow::AddList(BList *tweets)
{
	theTimeLine->AddList(tweets);
}

void
HTGTweetViewWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {
		default:
			be_app->PostMessage(message);
			break;
	}
}

bool
HTGTweetViewWindow::QuitRequested()
{
	MessageReceived(new BMessage(TWEETVIEWWINDOW_CLOSED));
	return true;
}

HTGTweetViewWindow::~HTGTweetViewWindow()
{
	theTimeLine->RemoveSelf();
	delete theTimeLine;
}
