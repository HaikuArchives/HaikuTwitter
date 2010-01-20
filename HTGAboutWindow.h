/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include <string>
#include <iostream>
#include <string.h>

#include <Window.h>
#include <Message.h>
#include <StringView.h>

#ifndef HTG_ABOUT_WINDOW
#define HTG_ABOUT_WINDOW

using namespace std;

class HTGAboutWindow : public BWindow {
public:
	HTGAboutWindow();
	~HTGAboutWindow();
	
private:
	void _setupWindow();

	
	BButton *applyButton;
	BButton *revertButton;
	BView *backgroundView;
	BStringView *informationView;
	
};
#endif
