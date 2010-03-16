/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "Window.h"
#include "TextControl.h"
#include "Button.h"
#include "View.h"

#ifndef HTG_SEARCHFORWINDOW_H
#define HTG_SEARCHFORWINDOW_H

class HTGMainWindow;

const int32 SEARCH = 'GO';

class HTGSearchForWindow : public BWindow {
public:
	HTGSearchForWindow(BWindow *delegate);
	~HTGSearchForWindow();
	void MessageReceived(BMessage *msg);

private:
	void search();
	
	BTextControl *query;
	BButton *goButton;
	BView *theView;
	BWindow *delegate;
};
#endif
