/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGNewTweetWindow.h"

HTGNewTweetWindow::HTGNewTweetWindow(twitCurl *twitObj) : BWindow(BRect(100, 100, 500, 200), "New tweet...", B_TITLED_WINDOW, B_NOT_RESIZABLE) {
	this->twitObj = twitObj;
	
	/*Add a grey view*/
	theView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	theView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(theView);
	
	/*Set up text view*/
	message = new HTGTextView(BRect(5,5,395,65), "Text", BRect(5,5,380,65), B_NOT_RESIZABLE, B_WILL_DRAW);
	theView->AddChild(message);
	message->WindowActivated(true);
	
	/*Set up buttons*/
	postButton = new BButton(BRect(3, 70, 90, 90), NULL, "Post", new BMessage(POST));
	cancelButton = new BButton(BRect(93, 70, 180, 90), NULL, "Cancel", new BMessage(CANCEL));
	theView->AddChild(postButton);
	postButton->SetEnabled(false);
	theView->AddChild(cancelButton);
	
	/*Set up symbol counter*/
	counterView = new BStringView(BRect(350, 75, 400, 95), "Counter", "140");
	counterView->SetHighColor(128, 128, 128);
	theView->AddChild(counterView);
}

void HTGNewTweetWindow::SetText(const char *text) {
	message->SetText(text);
	this->MessageReceived(new BMessage('UPDT')); //Update counter
}

void HTGNewTweetWindow::postTweet() {
	std::string replyMsg( "" );
	std::string postMsg(message->Text());
	if( twitObj->statusUpdate(postMsg) )  {
		printf( "Status update: OK\n" );
	}
	else {
		twitObj->getLastCurlError( replyMsg );
		printf( "\ntwitterClient:: twitCurl::updateStatus error:\n%s\n", replyMsg.c_str() );
		BAlert *theAlert = new BAlert("Oops, sorry!", replyMsg.c_str(), "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT); 	
	}
}

void HTGNewTweetWindow::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case POST:
			this->postTweet();
			this->Close();
			break;
		case CANCEL:
			this->Close();
		case UPDATED:
			this->updateCounter();
		default:
			BWindow::MessageReceived(msg);
	}
}

void HTGNewTweetWindow::updateCounter() {
	char counterString[32];
	int symbolsLeft =  NUMBER_OF_ALLOWED_CHARS - strlen(message->Text());
	sprintf(counterString, "%i", symbolsLeft);
	
	/*Check symbolsLeft, disable/enable post button and change counter color.*/
	if(symbolsLeft < 0) {
		counterView->SetHighColor(255, 0, 0);
		postButton->SetEnabled(false);
	}
	else {
		counterView->SetHighColor(128, 128, 128);
		if(symbolsLeft < 140)
			postButton->SetEnabled(true);
	}
	
	counterView->SetText(counterString);
}

HTGNewTweetWindow::~HTGNewTweetWindow() {
	message->RemoveSelf();
	delete message;
	
	postButton->RemoveSelf();
	delete postButton;
	
	cancelButton->RemoveSelf();
	delete cancelButton;
	
	theView->RemoveSelf();
	delete theView;
}
