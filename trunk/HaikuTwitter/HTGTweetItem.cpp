/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
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
	isReplicant = false;
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
	isReplicant = false;
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
HTGTweetItem::sortByDateFunc(const void* va, const void* vb)
{	
	if(va == NULL || vb == NULL)
		return 0;

	const HTGTweetItem** ia = (const HTGTweetItem**)(va);
	const HTGTweetItem** ib = (const HTGTweetItem**)(vb);

	HTTweet* a = (*ia)->getTweetPtr();
	HTTweet* b = (*ib)->getTweetPtr();
	
	if(a->getUnixTime() < b->getUnixTime())
		return 1;
	if(a->getUnixTime() > b->getUnixTime())
		return -1;
	else
		return 0;
}

int
HTGTweetItem::calculateSize(BRect frame, BView *owner)
{
	float calculatedSize = 0;
	float sizeOfTextView = 0;
	string tweetContent = theTweet->getText();
	
	BFont textFont;
	owner->GetFont(&textFont);
	textFont.SetEncoding(B_UNICODE_UTF8);
	textFont.SetSize(textFont.Size()-2);
	font_height height;
	owner->GetFontHeight(&height);
	float lineHeight = (height.ascent + height.descent + height.leading);
	
	BRect textRect(kTextPoint.x,frame.top+lineHeight+kTextPoint.y, frame.right, frame.bottom-lineHeight);
	HTGTweetTextView* textView = new HTGTweetTextView(textRect, theTweet->getScreenName().c_str(), BRect(0,0,frame.right-kAvatarRect.right-kMargin,frame.bottom-lineHeight-kMargin*2), B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	textView->SetFontAndColor(&textFont, B_FONT_ALL, &displayColors.textColor);
	textView->SetWordWrap(true);
	textView->MakeEditable(false);
	textView->setTweetId(theTweet->getId());
	textView->SetText(theTweet->getText().c_str());
	
	sizeOfTextView = textView->CountLines()*(lineHeight+3);

	calculatedSize = sizeOfTextView+(lineHeight+3)*1.5;
	if(calculatedSize < kAvatarRect.top+kAvatarRect.bottom)
		calculatedSize = kAvatarRect.top+kAvatarRect.bottom;
		
	delete textView;
	return (int)(calculatedSize + 0.5f);
}

void
HTGTweetItem::Update(BView *owner, const BFont* font)
{
	theTweet->setView(owner);
	timelineView = dynamic_cast<HTGTimeLineView*>(owner->Parent());

	isReplicant = (timelineView != NULL && timelineView->IsReplicant());
	
	/*Set colors*/
	if(isReplicant) { //We're a replicant
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
	SetHeight(calculateSize(owner->Bounds(), owner));
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
	owner->SetDrawingMode(B_OP_OVER);
	
	BFont textFont;
	owner->GetFont(&textFont);
	font_height height;
	owner->GetFontHeight(&height);
	float lineHeight = (height.ascent + height.descent + height.leading);

	/*Write name*/
	BRect nameBounds = NameBounds(frame, owner, lineHeight, theTweet->getScreenName().c_str());
	owner->SetHighColor(displayColors.nameColor);
	owner->MovePenTo(nameBounds.left, nameBounds.top);	
	owner->DrawString(theTweet->getScreenName().c_str());
		
	/*Write time*/
	BRect timeBounds = TimeBounds(frame, owner, lineHeight, theTweet->getRelativeDate().c_str());
	owner->SetHighColor(displayColors.timeColor);
	owner->MovePenTo(timeBounds.left, timeBounds.top);
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
		owner->GetFontHeight(&height);
		lineHeight = (height.ascent + height.descent + height.leading);
		BRect sourceBounds = SourceBounds(frame, owner, lineHeight, viaString.c_str());
		owner->MovePenTo(sourceBounds.left, sourceBounds.top);
		
		owner->DrawString(viaString.c_str());
		owner->SetFont(&currentFont);
	}
	
	/*Write text*/
	owner->GetFontHeight(&height);
	lineHeight = (height.ascent + height.descent + height.leading);
	BRect textRect(kTextPoint.x,frame.top+lineHeight+kTextPoint.y, frame.right, frame.bottom-lineHeight+1);
	if(textView == NULL) {
		textView = new HTGTweetTextView(textRect, theTweet->getScreenName().c_str(), BRect(0,0,frame.right-kAvatarRect.right-kMargin,frame.bottom-lineHeight-kMargin), B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
		owner->AddChild(textView);
		textFont.SetEncoding(B_UNICODE_UTF8);
		textFont.SetSize(textFont.Size()-2);
		textView->SetFontAndColor(&textFont, B_FONT_ALL, &displayColors.textColor);
		textView->SetViewColor(owner->ViewColor());
		textView->setTweetId(theTweet->getId());
		textView->SetText(theTweet->getText().c_str());
		if(!isReplicant)
			textView->MakeHyperText();
	}
	else{
		textFont.SetEncoding(B_UNICODE_UTF8);
		textFont.SetSize(textFont.Size()-2);
		//textView->SetFontAndColor(&textFont, B_FONT_ALL, &displayColors.textColor);
		textView->MoveTo(textRect.left, textRect.top);
		textView->ResizeTo(textRect.Width()+2, textRect.Height()+1);
		textView->SetTextRect(BRect(0,0,frame.right-kAvatarRect.right,frame.bottom-lineHeight-kMargin));
	}
	
	/*Draw seperator*/
	owner->StrokeLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom));
	
	/*Draw userIcon*/
	if(!theTweet->isDownloadingBitmap()) {
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmapAsync(theTweet->getBitmap(), AvatarBounds(frame));
	}
}

BRect
HTGTweetItem::NameBounds(BRect frame, BView* view, float lineHeight, const char* name)
{
	BRect bounds = BRect();
	
	bounds.left = kNamePoint.x+frame.left;
	bounds.top = kNamePoint.y+frame.top+lineHeight;
	bounds.right = bounds.left+view->StringWidth(name);
	bounds.bottom = bounds.top+lineHeight;
	
	return bounds;
}

BRect
HTGTweetItem::TimeBounds(BRect frame, BView* view, float lineHeight, const char* time)
{
	BRect bounds = BRect();
	
	bounds.left = frame.right-view->StringWidth(time)-kMargin;
	bounds.top = frame.top+lineHeight;
	bounds.right = frame.right-kMargin;
	bounds.bottom = bounds.top+lineHeight;
	
	return bounds;
}

BRect
HTGTweetItem::SourceBounds(BRect frame, BView* view, float lineHeight, const char* source)
{
	BRect bounds = BRect();

	bounds.left = kSourcePoint.x+frame.right-view->StringWidth(source)-kMargin;
	bounds.top = kSourcePoint.y+frame.bottom-kMargin;
	bounds.right = frame.right-kMargin;
	bounds.bottom = frame.top+lineHeight;
	
	return bounds;
}

BRect
HTGTweetItem::AvatarBounds(BRect frame)
{
	BRect bounds = BRect();
	
	bounds.left = kAvatarRect.left+frame.left;
	bounds.top = kAvatarRect.top+frame.top+((Height()-60)/2);
	bounds.right = kAvatarRect.right+frame.left;
	bounds.bottom = kAvatarRect.bottom+frame.top+((Height()-60)/2);
			
	return bounds;
}

void
HTGTweetItem::ClearView()
{
	if(textView != NULL) {
		textView->RemoveSelf();
		delete textView;
		textView = NULL;
	}
}

HTTweet*
HTGTweetItem::getTweetPtr() const
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
