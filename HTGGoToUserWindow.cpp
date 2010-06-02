/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGGoToUserWindow.h"

HTGGoToUserWindow::HTGGoToUserWindow(BWindow *delegate) : BWindow(BRect(100, 100, 360, 160), "Go to user...", B_TITLED_WINDOW, B_NOT_RESIZABLE) {
	this->delegate = delegate;
	
	/*Add a grey view*/
	theView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	theView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(theView);
	
	/*Set up text control*/
	username = new BTextControl(BRect(5,5,250,25), "Username", "Username", NULL, new BMessage(GO));
	theView->AddChild(username);
	username->WindowActivated(true);
	
	/*Set up buttons*/
	goButton = new BButton(BRect(170, 30, 247, 55), NULL, "Go", new BMessage(GO));
	theView->AddChild(goButton);
	
	username->MakeFocus();
	goButton->MakeDefault(true);
}

void HTGGoToUserWindow::findUser() {
	/*Create new BMessage for opening a new timeline for username*/
	BMessage *newMsg = new BMessage('GUSR');
	newMsg->AddString("text", username->Text());
	
	/*Let main window handle the message*/
	DispatchMessage(newMsg, delegate);
}

void HTGGoToUserWindow::MessageReceived(BMessage *msg) {
	const char* text_label = "text";
	switch(msg->what) {
		case GO:
			this->findUser();
			this->Close();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}

HTGGoToUserWindow::~HTGGoToUserWindow() {
	username->RemoveSelf();
	delete username;
	
	goButton->RemoveSelf();
	delete goButton;
	
	theView->RemoveSelf();
	delete theView;
}
