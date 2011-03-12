/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTGAvatarView.h"

HTGAvatarView::HTGAvatarView()
	: BView("AvatarView", B_WILL_DRAW)
{
	fLogo = BTranslationUtils::GetBitmap(B_VECTOR_ICON_TYPE, "twitter_icon");
	
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	avatarTweet = NULL;
}

HTGAvatarView::HTGAvatarView(BRect frame, uint32 resizingMode)
	: BView(frame, "AvatarView", resizingMode, B_WILL_DRAW)
{
	fLogo = BTranslationUtils::GetBitmap(B_VECTOR_ICON_TYPE, "twitter_icon");
	
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	avatarTweet = NULL;
	
	//Set up text view 
	BRect viewRect(5,17,frame.right-60,40);
	fMessage = new HTGTextView(viewRect, "Text", BRect(5,5,viewRect.right-5,viewRect.bottom-5), B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	AddChild(fMessage);
	
	//Set up post button
	fPostButton = new BButton(BRect(frame.right-110, 58, frame.right-60, 63), NULL, "Tweet", new BMessage(POST));
	AddChild(fPostButton);
	fPostButton->SetEnabled(false);
	
	//Set up symbol counter
	fCounterView = new BStringView(BRect(frame.right-135, 60, frame.right-110, 80), "Counter", "140");
	fCounterView->SetHighColor(128, 128, 128);
	AddChild(fCounterView);
}

void
HTGAvatarView::SetAvatarTweet(HTTweet* theTweet)
{
	avatarTweet = theTweet;
	
	avatarTweet->setView(this);
	Invalidate();
}

HTGAvatarView::~HTGAvatarView()
{
	delete fLogo;
}

void HTGAvatarView::AttachedToWindow()
{
   if ( Parent() )
      SetViewColor(Parent()->ViewColor());
      
   fMessage->WindowActivated(true);
   BView::AttachedToWindow();
}

void
HTGAvatarView::_UpdateCounter()
{
	char counterString[32];
	int symbolsLeft =  NUMBER_OF_ALLOWED_CHARS;
	
	/*Have to check every character for a character with 2 byte representation
	 *Twitter count them as one character... and yeah, this is an UGLY FIX;p
	 *It's really late, so I'm not that interested in testing this for every char.
	 *Btw, I assume that two-byte chars is marked as negative.
	 */
	for(int i = 0;fMessage->Text()[i] != '\0';i++) {
		if(fMessage->Text()[i] < 0) //If negative, then skip a step.
			i++;
		symbolsLeft--;
	}
	sprintf(counterString, "%i", symbolsLeft);
	
	/*Check symbolsLeft, disable/enable post button and change counter color.*/
	if(symbolsLeft < 0) {
		fCounterView->SetHighColor(255, 0, 0);
		fPostButton->SetEnabled(false);
	}
	else if(symbolsLeft < 140) {
		fCounterView->SetHighColor(128, 128, 128);
		fPostButton->SetEnabled(true);
		ResizeTo(Bounds().Width(), 83);
		((HTGMainWindow *)Window())->AvatarViewResized();
		BRect textRect(5,17,Bounds().right-60,55);
		fMessage->ResizeTo(textRect.Width(), textRect.Height());
	}
	else if(symbolsLeft >= 140) {
		fPostButton->SetEnabled(false);
		ResizeTo(Bounds().Width(), 52);
		((HTGMainWindow *)Window())->AvatarViewResized();
		BRect textRect(5,17,Bounds().right-60,40);
		fMessage->ResizeTo(textRect.Width(), textRect.Height());
	}
	fCounterView->SetText(counterString);
}

void
HTGAvatarView::Draw(BRect updateRect)
{
	if(avatarTweet == NULL)
		return;
	
	SetDrawingMode(B_OP_OVER);
	
	SetHighColor(000, 000, 000);
	MovePenTo(5, 13);
	DrawString("What's Happening, ");
	
	//Write name
	SetHighColor(051,102,152);
	DrawString(avatarTweet->getScreenName().c_str());
	
	SetHighColor(000, 000, 000);
	DrawString("?");
	
	//Draw avatar
	if(!avatarTweet->isDownloadingBitmap()) {
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmapAsync(avatarTweet->getBitmap(), _AvatarBounds());
		SetDrawingMode(B_OP_OVER);
	}
}

void
HTGAvatarView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		/*case POST:
			this->postTweet();
			this->Close();
			break;*/
		case UPDATED:
			this->_UpdateCounter();
			break;
		default:
			BView::MessageReceived(msg);
	}
}

BRect
HTGAvatarView::_AvatarBounds()
{
	BRect frame = Bounds();
	BRect bounds = BRect();
	
	bounds.left = frame.right-1-kMargin-47;
	bounds.top = frame.top+1+kMargin;
	bounds.right = frame.right-1-kMargin;
	bounds.bottom = bounds.top+47;
			
	return bounds;
}
