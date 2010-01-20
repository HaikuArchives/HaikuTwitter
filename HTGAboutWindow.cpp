/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGAboutWindow.h"

HTGAboutWindow::HTGAboutWindow() : BWindow(BRect(100, 100, 350, 225), "About HaikuTwitter", B_TITLED_WINDOW, B_NOT_RESIZABLE) {
	_setupWindow();
}

HTGAboutWindow::~HTGAboutWindow() {	
	backgroundView->RemoveSelf();
	delete backgroundView;
}

void HTGAboutWindow::_setupWindow() {
	/*Add the view*/
	backgroundView = new BView(Bounds(), "BackgroundView", B_NOT_RESIZABLE, B_WILL_DRAW);
	backgroundView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(backgroundView);
	
	/*Add authors*/
	backgroundView->AddChild(new BStringView(BRect(50, 0, 250, 20), "Martin", "Martin Hebnes Pedersen"));
	backgroundView->AddChild(new BStringView(BRect(30, 45, 250, 60), "Martin", "Web: martinhpedersen.mine.nu"));
	backgroundView->AddChild(new BStringView(BRect(45, 60, 250, 75), "Martin", "Twitter: @martinhpedersen"));
	
	/*Add copyright*/
	backgroundView->AddChild(new BStringView(BRect(70, 90, 250, 105), "Copyright", "Copyright 2010"));
	backgroundView->AddChild(new BStringView(BRect(60, 105, 250, 120), "Copyright", "All rights reserved."));
}
