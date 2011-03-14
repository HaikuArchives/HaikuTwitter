/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTGStatusBar.h"

HTGStatusBar::HTGStatusBar(BRect frame)
	: BView(BRect(frame.left, frame.Height()-13, frame.right-14, frame.Height()), "StatusBar", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,  B_WILL_DRAW | B_FRAME_EVENTS)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	BFont textFont;
	BFont currentFont;
	GetFont(&textFont);
	textFont.SetEncoding(B_UNICODE_UTF8);
	textFont.SetSize(textFont.Size()-2);
	SetFont(&textFont, B_FONT_ALL);
	
	status = "";
}


HTGStatusBar::~HTGStatusBar()
{

}

void
HTGStatusBar::AttachedToWindow()
{
   if ( Parent() )
      SetViewColor(Parent()->ViewColor());
   BView::AttachedToWindow();
}

void
HTGStatusBar::SetStatus(const char* str)
{
	status = str;
}

void
HTGStatusBar::Draw(BRect updateRect)
{
	/*Draw seperator*/
	SetHighColor(152, 152, 152); //Should find some B_COLOR for this
	StrokeLine(BPoint(Bounds().left, Bounds().top), BPoint(Bounds().right, Bounds().top));
	
	SetHighColor(16, 16, 16);
	MovePenTo(Bounds().right-StringWidth("Content from Twitter")-3, 11);
	DrawString("Content from Twitter");
	
	MovePenTo(3, 11);
	DrawString(status);
}
