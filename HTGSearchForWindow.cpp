/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGSearchForWindow.h"

HTGSearchForWindow::HTGSearchForWindow(BWindow *delegate) : BWindow(BRect(100, 100, 360, 160), "Search for...", B_TITLED_WINDOW, B_NOT_RESIZABLE) {
	this->delegate = delegate;
	
	/*Add a grey view*/
	theView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	theView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(theView);
	
	/*Set up text control*/
	query = new BTextControl(BRect(5,5,250,25), "Search for", "Search for", NULL, new BMessage(SEARCH));
	theView->AddChild(query);
	query->WindowActivated(true);
	
	/*Set up buttons*/
	goButton = new BButton(BRect(170, 30, 247, 55), NULL, "Go", new BMessage(SEARCH));
	theView->AddChild(goButton);
	
	query->MakeFocus();
	goButton->MakeDefault(true);
}

void HTGSearchForWindow::search() {
	/*Create new BMessage for opening a new timeline for username*/
	BMessage *newMsg = new BMessage('SRCH');
	newMsg->AddString("text", query->Text());
	
	/*Let main window handle the message*/
	delegate->MessageReceived(newMsg);
}

void HTGSearchForWindow::MessageReceived(BMessage *msg) {
	const char* text_label = "text";
	switch(msg->what) {
		case SEARCH:
			this->search();
			this->Close();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}

HTGSearchForWindow::~HTGSearchForWindow() {
	query->RemoveSelf();
	delete query;
	
	goButton->RemoveSelf();
	delete goButton;
	
	theView->RemoveSelf();
	delete theView;
}
