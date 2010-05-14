/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTweetItem.h"

HTGTweetItem::HTGTweetItem(HTTweet *theTweet) : BListItem() {
	this->theTweet = theTweet;
	textView = NULL;
}

int HTGTweetItem::calculateSize(BView *owner) {
	BFont textFont;
	owner->GetFont(&textFont);
	
	int calculatedSize = 0;
	int sizeOfTextView = 0;
	string tweetContent = theTweet->getText();
	
	/*Create a testView for the text, so we can calculate the number of line breaks*/
	BRect textRect(72,0, owner->Frame().right, 150);
	HTGTweetTextView *calcView = new HTGTweetTextView(textRect, theTweet->getScreenName().c_str(), BRect(0,0,owner->Frame().right-72,100), B_NOT_RESIZABLE, B_WILL_DRAW);
	calcView->setTweetId(theTweet->getId());
	textFont.SetEncoding(B_UNICODE_UTF8);
	textFont.SetSize(10);
	calcView->SetFontAndColor(&textFont);
	calcView->SetWordWrap(true);
	calcView->MakeEditable(false);
	calcView->SetText(theTweet->getText().c_str());
	
	font_height height;
	calcView->GetFontHeight(&height);
	int lineHeight = (height.ascent + height.descent + height.leading);
	
	sizeOfTextView = calcView->CountLines()*lineHeight;
	calculatedSize = sizeOfTextView+15+15;
	if(calculatedSize < 60)
		calculatedSize = 60;
		
	delete calcView;
	return calculatedSize;
}

void HTGTweetItem::Update(BView *owner, const BFont* font) {
	SetHeight(calculateSize(owner));
}

void HTGTweetItem::DrawItem(BView *owner, BRect frame, bool complete) {
	BFont textFont;
	owner->GetFont(&textFont);
	
	/*Write screen name*/
	owner->SetHighColor(100,100,100);
	owner->MovePenTo(frame.left+60+4, frame.top+12);
	owner->DrawString(theTweet->getScreenName().c_str());
	
	/*Write time*/
	owner->SetHighColor(128,128,128);
	owner->MovePenTo(frame.right-textFont.StringWidth(theTweet->getRelativeDate().c_str())-5, frame.top+12);
	owner->DrawString(theTweet->getRelativeDate().c_str());
	
	/*Write source name*/
	if(theTweet->getSourceName().length() < 25 && theTweet->getSourceName().length() > 1) {
		std::string viaString = theTweet->getSourceName();
		viaString.insert(0, "via ");
		
		BFont textFont;
		BFont currentFont;
		
		owner->GetFont(&textFont);
		owner->GetFont(&currentFont);
		textFont.SetEncoding(B_UNICODE_UTF8);
		textFont.SetSize(10);
		owner->SetFont(&textFont, B_FONT_ALL);
		owner->SetHighColor(128,128,128);
		owner->MovePenTo(frame.right-textFont.StringWidth(viaString.c_str())-5, frame.bottom-5);
		
		owner->DrawString(viaString.c_str());
		owner->SetFont(&currentFont);
	}
	
	/*Write text*/
	BRect textRect(60+4,frame.top+15, frame.right, frame.bottom-15);
	if(textView == NULL) {
		textView = new HTGTweetTextView(textRect, theTweet->getScreenName().c_str(), BRect(0,0,frame.right-60-4,frame.bottom-15), B_NOT_RESIZABLE, B_WILL_DRAW);
		owner->AddChild(textView);
	}

	textFont.SetEncoding(B_UNICODE_UTF8);
	textFont.SetSize(10);
	textView->SetFontAndColor(&textFont);
	textView->SetWordWrap(true);
	textView->MakeEditable(false);
	textView->setTweetId(theTweet->getId());
	textView->SetText(theTweet->getText().c_str());
	
	/*Draw seperator*/
	owner->StrokeLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom));
	
	/*Draw userIcon*/
	if(theTweet->isDownloadingBitmap())
		theTweet->waitUntilDownloadComplete();
	owner->SetDrawingMode(B_OP_ALPHA);
	owner->DrawBitmapAsync(theTweet->getBitmap(), BRect(frame.left+6, frame.top+7+((Height()-60)/2), frame.left+48+6, frame.top+72-18+((Height()-60)/2)));
	owner->SetDrawingMode(B_OP_OVER);
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
