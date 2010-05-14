/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */

 
#include "HTGTweetTextView.h"

HTGTweetTextView::HTGTweetTextView(BRect frame, const char *name, BRect textRect, uint32 resizingMode, uint32 flags) : BTextView(frame, name, textRect, resizingMode, flags) {
		tweetId = "";
}
	
HTGTweetTextView::HTGTweetTextView(BRect frame, const char *name, BRect textRect, const BFont* font, const rgb_color* color, uint32 resizingMode, uint32 flags) : BTextView(frame, name, textRect, font, color, resizingMode, flags) {
		tweetId = "";
}

void HTGTweetTextView::setTweetId(const char* tweetId) {
	this->tweetId = tweetId;
}

void HTGTweetTextView::MouseDown(BPoint point) {	
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		BMenuItem* selected;
	
		ConvertToScreen(&point);
	
		BPopUpMenu *myPopUp = new BPopUpMenu("TweetOptions", false, true, B_ITEMS_IN_COLUMN);
		
		int32 selectionStart;
		int32 selectionFinish;
		GetSelection(&selectionStart, &selectionFinish);
		if((selectionFinish - selectionStart) > 0) {
			myPopUp->AddItem(new BMenuItem("Copy", new BMessage(B_COPY)));
			myPopUp->AddSeparatorItem();
		}
	
		myPopUp->AddItem(new BMenuItem("Retweet...", new BMessage(GO_RETWEET)));
		myPopUp->AddItem(new BMenuItem("Reply...", new BMessage(GO_REPLY)));
		myPopUp->AddSeparatorItem();
	
		BList *screenNameList = this->getScreenNames();
		for(int i = 0; i < screenNameList->CountItems(); i++)
			myPopUp->AddItem((BMenuItem *)screenNameList->ItemAt(i));
		
		BList *urlList = this->getUrls();
		if(urlList->CountItems() > 0)
			myPopUp->AddSeparatorItem();
		for(int i = 0; i < urlList->CountItems(); i++)
			myPopUp->AddItem((BMenuItem *)urlList->ItemAt(i));
		
		BList *tagList = this->getTags();
		if(tagList->CountItems() > 0)
			myPopUp->AddSeparatorItem();
		for(int i = 0; i < tagList->CountItems(); i++)
			myPopUp->AddItem((BMenuItem *)tagList->ItemAt(i));
	
		selected = myPopUp->Go(point, true, true, true);
	
		if (selected) {
    		this->MessageReceived(selected->Message());
   		}
	
		delete myPopUp;
		delete screenNameList;
		delete urlList;
	}
	else
		BTextView::MouseDown(point);
}

BList* HTGTweetTextView::getScreenNames() {
	BList *theList = new BList();
	
	std::string tweetersName(this->Name());
	tweetersName.insert(0, "@");
	tweetersName.append("...");
	BMessage *firstMessage = new BMessage(GO_USER);
	firstMessage->AddString("text", this->Name());
	theList->AddItem(new BMenuItem(tweetersName.c_str(), firstMessage));
			
	for(int i = 0; Text()[i] != '\0'; i++) {
		if(Text()[i] == '@') {
			i++; //Skip leading '@'
			std::string newName("");
			while(isValidScreenNameChar(Text()[i])) {
				newName.append(1, Text()[i]);
				i++;
			}
			BMessage *theMessage = new BMessage(GO_USER);
			theMessage->AddString("text", newName.c_str());
			newName.insert(0, "@");
			newName.append("...");
			theList->AddItem(new BMenuItem(newName.c_str(), theMessage));
		}
	}
	
	return theList;
}

BList* HTGTweetTextView::getUrls() {
	BList *theList = new BList();
	size_t pos = 0;
	std::string theText(this->Text());
	
	/*Search for :// (URIs)*/
	while(pos != std::string::npos) {
		pos = theText.find("://", pos);
		if(pos != std::string::npos) {
			int start = pos;
			int end = pos;
			while(start >= 0 && theText[start] != ' ' && theText[start] != '\n' && theText[start] > 0) {
				start--;
			}
			while(end < theText.length() && theText[end] != ' ') {
				end++;
			}
			BMessage *theMessage = new BMessage(GO_TO_URL);
			theMessage->AddString("url", theText.substr(start+1, end-start-1).c_str());
			theList->AddItem(new BMenuItem(theText.substr(start+1, end-start-1).c_str(), theMessage));
			pos = end;
		}
	}
	
	/*Search for www.*/
	pos = 0;
	while(pos != std::string::npos) {
		pos = theText.find("www.", pos);
		if(theText.find("://", pos-3, 3) != std::string::npos) //So we don't add URL's detected from the method above.
			pos++;
		else if (pos != std::string::npos) {
			int start = pos;
			int end = pos;
			while(end < theText.length() && theText[end] != ' ') {
				end++;
			}
			BMessage *theMessage = new BMessage(GO_TO_URL);
			std::string httpString(theText.substr(start, end-start).c_str());
			httpString.insert(0, "http://"); //Add the http:// prefix.
			theMessage->AddString("url", httpString.c_str());
			theList->AddItem(new BMenuItem(httpString.c_str(), theMessage));
			pos = end;
		}
	}
	
	return theList;
}

BList* HTGTweetTextView::getTags() {
	BList *theList = new BList();
	size_t pos = 0;
	std::string theText(this->Text());
	
	while(pos != std::string::npos) {
		pos = theText.find("#", pos);
		if(pos != std::string::npos) {
			int start = pos;
			int end = pos;
			while(end < theText.length() && theText[end] != ' ' && theText[end] != '\n') {
				end++;
			}
			if(end == theText.length()-2) //For some reason, we have to do this.
				end--;
			BMessage *theMessage = new BMessage(GO_SEARCH);
			theMessage->AddString("text", theText.substr(start, end-start).c_str());
			theList->AddItem(new BMenuItem(theText.substr(start, end-start).c_str(), theMessage));
			pos = end;
		}
	}
	
	return theList;
}

bool HTGTweetTextView::isValidScreenNameChar(const char& c) {
	if(c <= 'z' && c >= 'a')
		return true;
	if(c <= 'Z' && c >= 'A')
		return true;
	if(c <= '9' && c >= '0')
		return true;
	if(c == '_')
		return true;
	
	return false;
}

void HTGTweetTextView::sendRetweetMsgToParent() {
	BMessage *retweetMsg = new BMessage(NEW_TWEET);
	std::string RTString(this->Text());
	RTString.insert(0, " ");
	RTString.insert(0, this->Name());
	RTString.insert(0, "@");
	RTString.insert(0, "RT ");
	retweetMsg->AddString("text", RTString.c_str());
	BTextView::MessageReceived(retweetMsg);
}

void HTGTweetTextView::sendReplyMsgToParent() {
	BMessage *replyMsg = new BMessage(NEW_TWEET);
	std::string theString(" ");
	theString.insert(0, this->Name());
	theString.insert(0, "@");
	replyMsg->AddString("text", theString.c_str());
	replyMsg->AddString("reply_to_id", tweetId);
	BTextView::MessageReceived(replyMsg);
}

void HTGTweetTextView::MessageReceived(BMessage *msg) {
	const char* url_label = "url";
	const char* name_label = "screenName";
	std::string newTweetAppend(" ");
	switch(msg->what) {
		case GO_RETWEET:
			this->sendRetweetMsgToParent();
			break;
		case GO_REPLY:
			this->sendReplyMsgToParent();
			break;
		case GO_TO_URL:
			this->openUrl(msg->FindString(url_label, (int32)0));
			break;
		default:
			BTextView::MessageReceived(msg);
	}
}

void HTGTweetTextView::openUrl(const char *url) {
	// be lazy and let /bin/open open the URL
	entry_ref ref;
	if (get_ref_for_path("/bin/open", &ref))
		return;
		
	const char* args[] = { "/bin/open", url, NULL };
	be_roster->Launch(&ref, 2, args);
}
