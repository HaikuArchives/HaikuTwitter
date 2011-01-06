/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTweetItem.h"

HTGTweetItem::HTGTweetItem(HTTweet *theTweet, bool displayFullName)
	: BListItem()
{
	this->theTweet = theTweet;
	this->displayFullName = displayFullName;
	timelineView = NULL;
	textView = NULL;
}

HTGTweetItem::HTGTweetItem(BMessage* archive)
{
	archive->FindBool("HTGTweetItem::displayFullName", &displayFullName);
	
	BMessage msg;
	BArchivable* unarchived;
	if(archive->FindMessage("HTGTweetItem::theTweet", &msg) == B_OK) {
		unarchived = instantiate_object(&msg);
		if(unarchived)
			theTweet = dynamic_cast<HTTweet *>(unarchived);
		else
			theTweet = new HTTweet();
	}
	
	timelineView = NULL;
	textView = NULL;
}

BArchivable*
HTGTweetItem::Instantiate(BMessage* archive)
{
	if(validate_instantiation(archive, "HTGTweetItem"))
		return new HTGTweetItem(archive);
	return NULL;
}

status_t
HTGTweetItem::Archive(BMessage* archive, bool deep) const
{
	BListItem::Archive(archive, deep);
	archive->AddString("class", "HTGTweetItem");

	/*Archive ivars*/
	archive->AddBool("HTGTweetItem::displayFullName", displayFullName);

	/*Archive tweet*/
	BMessage msg;
	theTweet->Archive(&msg, deep);
	return archive->AddMessage("HTGTweetItem::theTweet", &msg);
}

int
HTGTweetItem::calculateSize(BView *owner)
{
	BFont textFont;
	owner->GetFont(&textFont);
	
	float calculatedSize = 0;
	float sizeOfTextView = 0;
	string tweetContent = theTweet->getText();
	
	/*Create a testView for the text, so we can calculate the number of line breaks*/
	BRect textRect(72,0, owner->Frame().right, 0);
	HTGTweetTextView *calcView = new HTGTweetTextView(textRect, theTweet->getScreenName().c_str(), BRect(0,0,owner->Frame().right-72,300), B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	calcView->setTweetId(theTweet->getId());
	textFont.SetEncoding(B_UNICODE_UTF8);
	textFont.SetSize(textFont.Size()-2);
	calcView->SetFontAndColor(&textFont);
	calcView->SetWordWrap(true);
	calcView->MakeEditable(false);
	calcView->SetText(theTweet->getText().c_str());
	
	font_height height;
	calcView->GetFontHeight(&height);
	float lineHeight = (height.ascent + height.descent + height.leading);
	if(textFont.Size() > 10)
		lineHeight += (textFont.Size()-11)*1.8f;
	sizeOfTextView = calcView->CountLines()*lineHeight;

	calculatedSize = sizeOfTextView+(lineHeight+3)*1.8;
	if(calculatedSize < 60)
		calculatedSize = 60;
		
	delete calcView;
	return (int)(calculatedSize + 0.5f);
}

void
HTGTweetItem::Update(BView *owner, const BFont* font)
{
	theTweet->setView(owner);
	
	/*Set colors*/
	timelineView = dynamic_cast<HTGTimeLineView*>(owner->Parent());
	if(timelineView != NULL) { //We're a replicant
		displayColors.nameColor	=	compileColor(235,235,235);
		displayColors.textColor	=	compileColor(255,255,255);
		displayColors.timeColor	=	compileColor(192,192,192);
		displayColors.sourceColor =	compileColor(192,192,192);
	}else {
		displayColors.nameColor	=	compileColor(051,102,152);
		displayColors.textColor	=	compileColor(000,000,000);
		displayColors.timeColor	=	compileColor(128,128,128);
		displayColors.sourceColor =	compileColor(128,128,128);
	}
	SetHeight(calculateSize(owner));
}

rgb_color
HTGTweetItem::compileColor(uint8 red, uint8 green, uint8 blue){
	rgb_color color;
	color.red = red;
	color.green = green;
	color.blue = blue;
	
	return color;
}

void
HTGTweetItem::DrawItem(BView *owner, BRect frame, bool complete)
{	
	BFont textFont;
	owner->GetFont(&textFont);
	font_height height;
	owner->GetFontHeight(&height);
	float lineHeight = (height.ascent + height.descent + height.leading);
	
	owner->SetDrawingMode(B_OP_OVER);

	/*Write name*/
	owner->SetHighColor(displayColors.nameColor);
	owner->MovePenTo(frame.left+60+4, frame.top+lineHeight);
	if(displayFullName)
		owner->DrawString(theTweet->getFullName().c_str());
	else
		owner->DrawString(theTweet->getScreenName().c_str());
		
	/*Write time*/
	owner->SetHighColor(displayColors.timeColor);
	owner->MovePenTo(frame.right-textFont.StringWidth(theTweet->getRelativeDate().c_str())-5, frame.top+lineHeight);
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
		textFont.SetSize(textFont.Size()-2);
		owner->SetFont(&textFont, B_FONT_ALL);
		owner->SetHighColor(displayColors.sourceColor);
		owner->MovePenTo(frame.right-textFont.StringWidth(viaString.c_str())-5, frame.bottom-5);
		
		owner->DrawString(viaString.c_str());
		owner->SetFont(&currentFont);
	}
	
	/*Write text*/
	BRect textRect(60+4,frame.top+lineHeight+3, frame.right, frame.bottom-lineHeight);
	if(textView == NULL) {
		textView = new HTGTweetTextView(textRect, theTweet->getScreenName().c_str(), BRect(0,0,frame.right-60-4,frame.bottom-lineHeight), B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
		owner->AddChild(textView);
		textFont.SetEncoding(B_UNICODE_UTF8);
		textFont.SetSize(textFont.Size()-2);
		textView->SetFontAndColor(&textFont, B_FONT_ALL, &displayColors.textColor);
		textView->SetViewColor(owner->ViewColor());
		textView->SetWordWrap(true);
		textView->MakeEditable(false);
		textView->setTweetId(theTweet->getId());
		textView->SetText(theTweet->getText().c_str());
	}
	else if(complete) {
		textFont.SetEncoding(B_UNICODE_UTF8);
		textFont.SetSize(textFont.Size()-2);
		textView->SetFontAndColor(&textFont, B_FONT_ALL, &displayColors.textColor);
		textView->SetViewColor(owner->ViewColor());
		textView->MoveTo(textRect.left, textRect.top);
		textView->ResizeTo(textRect.Width(), textRect.Height());
		textView->SetTextRect(BRect(0,0,frame.right-60-4,frame.bottom));
	}
	
	/*Draw seperator*/
	owner->StrokeLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom));
	
	/*Draw userIcon*/
	if(!theTweet->isDownloadingBitmap()) {
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmapAsync(theTweet->getBitmap(), BRect(frame.left+9, frame.top+5+((Height()-60)/2), frame.left+48+8, frame.top+72-20+((Height()-60)/2)));
	}
}

HTTweet*
HTGTweetItem::getTweetPtr()
{
	return theTweet;
}

HTGTweetItem::~HTGTweetItem()
{
	if(textView != NULL) {
		textView->RemoveSelf();
		delete textView;
	}
	delete theTweet;
}
