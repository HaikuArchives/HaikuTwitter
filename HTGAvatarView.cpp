/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTGAvatarView.h"

HTGAvatarView::HTGAvatarView(twitCurl* twitObj, BRect frame, uint32 resizingMode)
	: BView(frame, "AvatarView", resizingMode, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{	
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	avatarTweet = NULL;
	this->twitObj = twitObj;
	displayAvatar = true;
		
	//Set up text view 
	BRect viewRect(5,22,frame.right-60,45);
	fMessage = new HTGTextView(viewRect, "Text", BRect(5,5,viewRect.right-5,viewRect.bottom-5), B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	AddChild(fMessage);
	
	//Set up post button
	fPostButton = new BButton(BRect(frame.right-55, frame.top+1+kMargin, frame.right-5, frame.top), NULL, "Tweet", new BMessage(POST));
	fPostButton->SetEnabled(false);
	
	//Set up symbol counter
	fCounterView = new BStringView(BRect(frame.right-40, frame.top+1+kMargin+fPostButton->Bounds().Height(), frame.right-15, frame.top+1+kMargin+fPostButton->Bounds().Height()+12), "Counter", "140");
	fCounterView->SetHighColor(128, 128, 128);
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
	delete twitObj;
	if(avatarTweet != NULL)
		delete avatarTweet;
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
		
		if(displayAvatar) {
			displayAvatar = false;
			Invalidate();
			AddChild(fPostButton);
			AddChild(fCounterView);
			ResizeTo(Bounds().Width(), 83);
			BPoint buttonPoint(Frame().right-55, Frame().top+1+kMargin);
			BPoint counterPoint(Frame().right-40, Frame().top+1+kMargin+fPostButton->Bounds().Height());
			fPostButton->MoveTo(buttonPoint);
			fCounterView->MoveTo(counterPoint);
			((HTGMainWindow *)Window())->AvatarViewResized();
			BRect textRect(5,22,Bounds().right-60,65);
			fMessage->ResizeTo(textRect.Width(), textRect.Height());
		}
	}
	else if(symbolsLeft >= 140) {
		fPostButton->SetEnabled(false);
		ResizeTo(Bounds().Width(), 52);
		((HTGMainWindow *)Window())->AvatarViewResized();
		BRect textRect(5,22,Bounds().right-60,45);
		displayAvatar = true;
		Invalidate();
		fPostButton->RemoveSelf();
		fCounterView->RemoveSelf();
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
	MovePenTo(5, 17);
	DrawString("What's Happening, ");
	
	SetHighColor(051,102,152);
	DrawString(avatarTweet->getScreenName().c_str());
	
	SetHighColor(000, 000, 000);
	DrawString("?");
	
	//Draw avatar
	if(!avatarTweet->isDownloadingBitmap() && displayAvatar) {
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmapAsync(avatarTweet->getBitmap(), _AvatarBounds());
		SetDrawingMode(B_OP_OVER);
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

void
HTGAvatarView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case POST:
			postTweet();
			fMessage->SetText("");
			_UpdateCounter();
			break;
		case UPDATED:
			_UpdateCounter();
			break;
		default:
			BView::MessageReceived(msg);
	}
}


void
HTGAvatarView::postTweet()
{
	std::string replyMsg( "" );
	std::string postMsg = urlEncode(fMessage->Text());
	if( twitObj->statusUpdate(postMsg) )  {
		printf( "Status update: OK\n" );
		twitObj->getLastWebResponse(replyMsg);
		int errorStart = replyMsg.find("<error>");
		int errorEnd = replyMsg.find("<\error>");
		if(errorStart-errorEnd > 0)
			HTGErrorHandling::displayError(replyMsg.substr(errorStart, errorEnd-errorStart).c_str());
	}
	else {
		twitObj->getLastCurlError( replyMsg );
		printf( "\ntwitterClient:: twitCurl::updateStatus error:\n%s\n", replyMsg.c_str() );
		BAlert *theAlert = new BAlert("Oops, sorry!", replyMsg.c_str(), "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT); 	
	}
}

std::string
HTGAvatarView::urlEncode(const char* input)
{
	std::string output(input);
	
	for(int i = 0; i < output.length(); i++) {
		if(output[i] == '%')
			output.replace(i, 1, "%25");
		if(output[i] == '&')
			output.replace(i, 1, "%26");
		if(output[i] == '+')
			output.replace(i, 1, "%2B");
		if(output[i] == '@')
			output.replace(i, 1, "%40");
		if(output[i] == '?')
			output.replace(i, 1, "%3F");
		if(output[i] == '=')
			output.replace(i, 1, "%3D");
		if(output[i] == '$')
			output.replace(i, 1, "%24");
		if(output[i] == '/')
			output.replace(i, 1, "%2F");
		if(output[i] == ':')
			output.replace(i, 1, "%3A");
		if(output[i] == ';')
			output.replace(i, 1, "%3B");
		if(output[i] == ',')
			output.replace(i, 1, "%2C");
	}
	
	return output;
}
