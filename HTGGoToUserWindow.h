/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "Window.h"
#include "TextControl.h"
#include "Button.h"
#include "View.h"

#ifndef HTG_GOTOUSERWINDOW_H
#define HTG_GOTOUSERWINDOW_H

class HTGMainWindow;

const int32 GO = 'GO';

class HTGGoToUserWindow : public BWindow {
public:
							HTGGoToUserWindow(BWindow *delegate);
	virtual					~HTGGoToUserWindow();
			void 			MessageReceived(BMessage *msg);

private:
			void 			findUser();
	
			BTextControl*	username;
			BButton*		goButton;
			BView*			theView;
			BWindow*		delegate;
};
#endif
