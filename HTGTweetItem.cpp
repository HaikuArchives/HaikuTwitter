/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTweetItem.h"

HTGTweetItem::HTGTweetItem(HTTweet *theTweet) : BListItem() {
	this->theTweet = theTweet;
	textView = NULL;
	SetHeight(72);
}

void HTGTweetItem::DrawItem(BView *owner, BRect frame, bool complete) {
	/*Set proper height*/
	if(Height() < 72) {
		SetHeight(72);
		owner->Invalidate();
		return ;
	}
	
	/*Write screen name*/
	owner->SetHighColor(100,100,100);
	owner->MovePenTo(frame.left+72, frame.top+12);
	owner->DrawString(theTweet->getScreenName().c_str());
	
	/*Write time*/
	owner->SetHighColor(128,128,128);
	owner->MovePenTo(frame.right-85, frame.top+12);
	owner->DrawString(theTweet->getRelativeDate().c_str());
	
	/*Write text*/
	BRect textRect(72,frame.top+15, frame.right, frame.bottom-3);
	if(textView == NULL) {
		textView = new HTGTweetTextView(textRect, theTweet->getScreenName().c_str(), BRect(0,0,frame.right-72,frame.bottom-3), B_NOT_RESIZABLE, B_WILL_DRAW);
		owner->AddChild(textView);
	}

	BFont textFont;
	owner->GetFont(&textFont);
	textFont.SetEncoding(B_UNICODE_UTF8);
	textFont.SetSize(10);
	textView->SetFontAndColor(&textFont);
	textView->SetWordWrap(true);
	textView->MakeEditable(false);
	textView->SetText(theTweet->getText().c_str());
	
	/*Draw seperator*/
	owner->StrokeLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom));
	
	/*Draw userIcon*/
	if(theTweet->isDownloadingBitmap())
		theTweet->waitUntilDownloadComplete();
	owner->DrawBitmapAsync(theTweet->getBitmap(), BRect(frame.left+12, frame.top+12, frame.left+48+12, frame.bottom-12));
}

HTTweet* HTGTweetItem::getTweetPtr() {
	return theTweet;
}

HTGTweetItem::~HTGTweetItem() {
	if(textView != NULL) {
		textView->RemoveSelf();
		delete textView;
	}
	delete theTweet;
}
