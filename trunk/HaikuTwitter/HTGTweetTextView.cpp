/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */

 
#include "HTGTweetTextView.h"

HyperTextAction::HyperTextAction()
{
}


HyperTextAction::~HyperTextAction()
{
}


void
HyperTextAction::MouseOver(HTGTweetTextView* view, BPoint where, BMessage* message)
{
	BCursor linkCursor(B_CURSOR_ID_FOLLOW_LINK);
	view->SetViewCursor(&linkCursor);
}


void
HyperTextAction::Clicked(HTGTweetTextView* view, BPoint where, BMessage* message)
{
}

struct HTGTweetTextView::ActionInfo {
	ActionInfo(int32 startOffset, int32 endOffset, HyperTextAction* action)
		:
		startOffset(startOffset),
		endOffset(endOffset),
		action(action)
	{
	}

	~ActionInfo()
	{
		delete action;
	}

	static int Compare(const ActionInfo* a, const ActionInfo* b)
	{
		return a->startOffset - b->startOffset;
	}

	static int CompareEqualIfIntersecting(const ActionInfo* a,
		const ActionInfo* b)
	{
		if (a->startOffset < b->endOffset && b->startOffset < a->endOffset)
			return 0;
		return a->startOffset - b->startOffset;
	}

	int32				startOffset;
	int32				endOffset;
	HyperTextAction*	action;
};

static size_t WriteUrlCallback(void *ptr, size_t size, size_t nmemb, void *data);
status_t _threadDownloadLinkIconURLs(void *data);

HTGTweetTextView::HTGTweetTextView(BRect frame, const char *name, BRect textRect, uint32 resizingMode, uint32 flags)
	: BTextView(frame, name, textRect, resizingMode, flags),
	fActionInfos(new ActionInfoList(100, true))
{
		tweetId = std::string("");
		urls = new BList();
		currentThread = B_NAME_NOT_FOUND;
		SetWordWrap(true);
		MakeEditable(false);
		SetStylable(true);
}
	
HTGTweetTextView::HTGTweetTextView(BRect frame, const char *name, BRect textRect, const BFont* font, const rgb_color* color, uint32 resizingMode, uint32 flags)
	: BTextView(frame, name, textRect, font, color, resizingMode, flags),
	fActionInfos(new ActionInfoList(100, true))
{
		tweetId = std::string("");
		SetWordWrap(true);
		MakeEditable(false);
		SetStylable(true);
}

void
HTGTweetTextView::MakeHyperText()
{
	size_t pos = 0;
	std::string theText(this->Text());
	
	/*Search for :// (URIs)*/
	while(pos != std::string::npos) {
		pos = theText.find("://", pos);
		if(pos != std::string::npos) {
			int start = pos-1;
			int end = pos;
			while(start > -1 && (theText[start] >= 'A' && theText[start] <= 'z')) {
				start--;
			}
			while(end < theText.length() && theText[end] != ' ' && theText[end] != '\n')
				end++;
			
			BMessage *theMessage = new BMessage(GO_TO_URL);
			theMessage->AddString("url", theText.substr(start+1, end-start-1).c_str());
			
			BFont font;
			GetFont(&font);
			font.SetSize(font.Size()-2);
			SetFontAndColor(start+1, end, &font, B_FONT_ALL, &kLinkBlue);
			AddHyperTextAction(start+1, end, new URLAction(theText.substr(start+1, end-start-1).c_str()));
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
			while(end < theText.length() && theText[end] != ' ' && theText[end] != '\n')
				end++;
			
			BMessage *theMessage = new BMessage(GO_TO_URL);
			std::string httpString(theText.substr(start, end-start).c_str());
			httpString.insert(0, "http://"); //Add the http:// prefix.
			theMessage->AddString("url", httpString.c_str());
			
			BFont font;
			GetFont(&font);
			font.SetSize(font.Size()-2);
			SetFontAndColor(start, end, &font, B_FONT_ALL, &kLinkBlue);
			AddHyperTextAction(start, end, new URLAction(httpString.c_str()));
			pos = end;
		}
	}

	/*# tags*/
	pos = 0;
	while(pos != std::string::npos) {
		pos = theText.find("#", pos);
		if(pos != std::string::npos) {
			int start = pos;
			int end = pos;
			while(end < theText.length() && theText[end] != ' ' && theText[end] != '\n' && theText[end] != ',' && theText[end] != '.' && theText[end] != '"') {
				end++;
			}
			if(end == theText.length()-2) //For some reason, we have to do this.
				end--;
			BMessage theMessage(GO_SEARCH);
			theMessage.AddString("text", theText.substr(start, end-start).c_str());
			BFont font;
			GetFont(&font);
			font.SetSize(font.Size()-2);
			SetFontAndColor(start, end, &font, B_FONT_ALL, &kLinkBlue);
			AddHyperTextAction(start, end, new MessageAction(theMessage));
			pos = end;
		}
	}
	
	/*Screen names*/
	for(int i = 0; Text()[i] != '\0'; i++) {
		string newName;
		if(Text()[i] == '@') {
			i++;
			int start = i;
			while(isValidScreenNameChar(Text()[i]) && i < strlen(Text()))
				i++;
			int end = i;
			BFont font;
			GetFont(&font);
			font.SetSize(font.Size()-2);
			SetFontAndColor(start, end, &font, B_FONT_ALL, &kLinkBlue);
			BMessage theMessage(GO_USER);
			theMessage.AddString("text", theText.substr(start, end-start).c_str());
			AddHyperTextAction(start, end, new MessageAction(theMessage));
		}
	}
}

void
HTGTweetTextView::setTweetId(const char* tweetId)
{
	this->tweetId = std::string(tweetId);
}

bool
HTGTweetTextView::CanEndLine(int32 offset)
{
	char symbol = ByteAt(offset);
	if ( symbol == ' ' || symbol == ',' )
		return true;
	
	return false;
}

void
HTGTweetTextView::MouseDown(BPoint point)
{
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
	
		ConvertToScreen(&point);
	
		BPopUpMenu *myPopUp = new BPopUpMenu("TweetOptions", false, true, B_ITEMS_IN_COLUMN);
		
		int32 selectionStart;
		int32 selectionFinish;
		GetSelection(&selectionStart, &selectionFinish);
		if((selectionFinish - selectionStart) > 0) {
			myPopUp->AddItem(new HTGTweetMenuItem("Copy", new BMessage(B_COPY)));
			myPopUp->AddSeparatorItem();
		}
	
		myPopUp->AddItem(new HTGTweetMenuItem("Retweet...", new BMessage(GO_RETWEET)));
		myPopUp->AddItem(new HTGTweetMenuItem("Reply...", new BMessage(GO_REPLY)));
		myPopUp->AddSeparatorItem();
	
		BList *screenNameList = this->getScreenNames();
		for(int i = 0; i < screenNameList->CountItems(); i++)
			myPopUp->AddItem((HTGTweetMenuItem *)screenNameList->ItemAt(i));
		
		urls = getUrls();
		currentThread = spawn_thread(_threadDownloadLinkIconURLs, "favIcon downloader", 10, this);
		resume_thread(currentThread);
		if(urls->CountItems() > 0)
			myPopUp->AddSeparatorItem();
		for(int i = 0; i < urls->CountItems(); i++) {
			myPopUp->AddItem((HTGTweetMenuItem *)urls->ItemAt(i));
		}
		
		BList *tagList = this->getTags();
		if(tagList->CountItems() > 0)
			myPopUp->AddSeparatorItem();
		for(int i = 0; i < tagList->CountItems(); i++)
			myPopUp->AddItem((HTGTweetMenuItem *)tagList->ItemAt(i));
	
		myPopUp->SetAsyncAutoDestruct(true);
		myPopUp->SetTargetForItems(BMessenger(this));
		myPopUp->Go(point+BPoint(1,1), true, true, false); //Do this synchronously
		//Kill the update thread so it does not try to access an allready freed object*
		if(currentThread != B_NAME_NOT_FOUND)
			kill_thread(currentThread);
	}
	else
		BTextView::MouseDown(point);
}

BList*
HTGTweetTextView::getScreenNames()
{
	BList *theList = new BList();
	
	std::string tweetersName(this->Name());
	tweetersName.insert(0, "@");
	tweetersName.append("...");
	BMessage *firstMessage = new BMessage(GO_USER);
	firstMessage->AddString("text", this->Name());
	theList->AddItem(new HTGTweetMenuItem(tweetersName.c_str(), firstMessage));
			
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
			theList->AddItem(new HTGTweetMenuItem(newName.c_str(), theMessage));
		}
	}
	
	return theList;
}

BList*
HTGTweetTextView::getUrls()
{
	BList *theList = new BList();
	size_t pos = 0;
	std::string theText(this->Text());
	
	/*Search for :// (URIs)*/
	while(pos != std::string::npos) {
		pos = theText.find("://", pos);
		if(pos != std::string::npos) {
			int start = pos;
			int end = pos;
			while(start > -1 && (theText[start] >= 'A' && theText[start] <= 'z')) {
				start--;
			}
			while(end < theText.length() && theText[end] != ' ' && theText[end] != '\n') {
				end++;
			}
			BMessage *theMessage = new BMessage(GO_TO_URL);
			theMessage->AddString("url", theText.substr(start+1, end-start-1).c_str());
			theList->AddItem(new HTGTweetMenuItem(theText.substr(start+1, end-start-1).c_str(), theMessage));
			BFont font;
			GetFont(&font);
			font.SetSize(font.Size()-2);
			SetFontAndColor(start+1, end, &font, B_FONT_ALL, &kLinkBlue);
			AddHyperTextAction(start+1, end, new URLAction(theText.substr(start+1, end-start-1).c_str()));
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
			while(end < theText.length() && theText[end] != ' ' && theText[end] != '\n') {
				end++;
			}
			BMessage *theMessage = new BMessage(GO_TO_URL);
			std::string httpString(theText.substr(start, end-start).c_str());
			httpString.insert(0, "http://"); //Add the http:// prefix.
			theMessage->AddString("url", httpString.c_str());
			theList->AddItem(new HTGTweetMenuItem(httpString.c_str(), theMessage));
			BFont font;
			GetFont(&font);
			font.SetSize(font.Size()-2);
			SetFontAndColor(start+1, end, &font, B_FONT_ALL, &kLinkBlue);
			AddHyperTextAction(start+1, end, new URLAction(httpString.c_str()));
			pos = end;
		}
	}
	
	return theList;
}

BList*
HTGTweetTextView::getTags()
{
	BList *theList = new BList();
	size_t pos = 0;
	std::string theText(this->Text());
	
	while(pos != std::string::npos) {
		pos = theText.find("#", pos);
		if(pos != std::string::npos) {
			int start = pos;
			int end = pos;
			while(end < theText.length() && theText[end] != ' ' && theText[end] != '\n' && theText[end] != ',' && theText[end] != '.' && theText[end] != '"') {
				end++;
			}
			if(end == theText.length()-2) //For some reason, we have to do this.
				end--;
			BMessage *theMessage = new BMessage(GO_SEARCH);
			theMessage->AddString("text", theText.substr(start, end-start).c_str());
			theList->AddItem(new HTGTweetMenuItem(theText.substr(start, end-start).c_str(), theMessage));
			pos = end;
		}
	}
	
	return theList;
}

bool
HTGTweetTextView::isValidScreenNameChar(const char& c)
{
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

void
HTGTweetTextView::sendRetweetMsgToParent()
{
	BMessage *retweetMsg = new BMessage(NEW_TWEET);
	std::string RTString(this->Text());
	RTString.insert(0, ": ");
	RTString.insert(0, this->Name());
	RTString.insert(0, "@");
	RTString.insert(0, "♺ ");
	retweetMsg->AddString("text", RTString.c_str());
	retweetMsg->AddString("reply_to_id", tweetId.c_str());
	BTextView::MessageReceived(retweetMsg);
}

void
HTGTweetTextView::sendReplyMsgToParent()
{
	BMessage *replyMsg = new BMessage(NEW_TWEET);
	std::string theString(" ");
	theString.insert(0, this->Name());
	theString.insert(0, "@");
	replyMsg->AddString("text", theString.c_str());
	replyMsg->AddString("reply_to_id", tweetId.c_str());
	BTextView::MessageReceived(replyMsg);
}

void
HTGTweetTextView::MessageReceived(BMessage *msg)
{
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

void
HTGTweetTextView::openUrl(const char *url)
{
	// be lazy and let /bin/open open the URL
	entry_ref ref;
	if (get_ref_for_path("/bin/open", &ref))
		return;
		
	const char* args[] = { "/bin/open", url, NULL };
	be_roster->Launch(&ref, 2, args);
}

HTGTweetTextView::~HTGTweetTextView()
{
	/*Kill the update thread*/
	if(currentThread != B_NAME_NOT_FOUND)
		kill_thread(currentThread);
	
	//delete fActionInfos;
}

status_t
_threadDownloadLinkIconURLs(void *data)
{
	HTGTweetTextView *super = (HTGTweetTextView*)data;
	CURL *curl_handle;
	BMallocIO *mallocIO = NULL;
	
	HTGTweetMenuItem* currentItem = NULL;
	for(int i = 0; i < super->urls->CountItems(); i++) {
		currentItem = (HTGTweetMenuItem *)super->urls->ItemAt(i);
		
		if(currentItem == NULL)
			return B_OK; //Abort: The menu has been closed and deleted.
		
		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		curl_easy_setopt(curl_handle, CURLOPT_URL, currentItem->Label());
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);
		
		/*send all data to this function*/
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteUrlCallback);
		
		/*we pass out 'mallocIO' object to the callback function*/
		mallocIO = new BMallocIO();
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)mallocIO);
		
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "haikutwitter-agent/1.0");
		
		/*get the data*/
		if(curl_easy_perform(curl_handle) < 0)
			std::cout << "libcURL: Download of linked page failed." << std::endl;
		
		/*cleanup curl stuff*/
		curl_easy_cleanup(curl_handle);
		
		std::string replyMsg((char *)mallocIO->Buffer(), mallocIO->BufferLength());
		if(!replyMsg.length()) {
			currentItem->setLinkIconUrl(*new string("-Icon not found-")); //Make the menuItem draw generic icon
			continue;
		}

		/*Check if menu has been closed, or the menu item for some other reason is gone*/
		if(currentItem == NULL)
			return B_OK;
		if(currentItem->Label() == NULL)
			return B_OK;
		
		/*Parse for base-url*/
		std::string location(currentItem->Label());
		int pos = 0;
		while(pos != std::string::npos) {
		pos = location.find("http://", pos);
			if(pos != std::string::npos) {
				int end = location.find("/", pos+8);
				location = location.substr(pos, end-pos);
				pos = end;
				i++;
			}
		}
		
		/*Parse for redirects (Location: http://)*/
		const char *queryTag = "Location: http://";
		pos = 0;
		while(pos != std::string::npos) {
		pos = replyMsg.find(queryTag, pos);
			if(pos != std::string::npos) {
				int start = pos+strlen(queryTag)-7;
				int end = replyMsg.find("/", start+8);
				location = replyMsg.substr(start, end-start);
				pos = end;
				i++;
			}
		}
		
		/*Parse for url to favIcon*/ //<link rel="shortcut icon" href="http://static.ak.fbcdn.net/rsrc.php/z9Q0Q/hash/8yhim1ep.ico" />
		queryTag = "icon\" href=\"";
		pos = 0;
		while(pos != std::string::npos) {
		pos = replyMsg.find(queryTag, pos);
			if(pos != std::string::npos) {
				int start = pos+strlen(queryTag);
				int end = replyMsg.find(".ico", start)+4;
				if(end == std::string::npos+4)
					break;
				std::string searchQuery(replyMsg.substr(start, end-start));
				if(searchQuery[0] != 'h') { //URL is relative, add the guessed location from above
					searchQuery.insert(0, "/");
					searchQuery.insert(0, location);
					std::cout << searchQuery << std::endl;
				}
				currentItem->setLinkIconUrl(searchQuery);
				std::cout << "Detected icon at: " << searchQuery << std::endl;
				pos = end;
				i++;
			}
		}
		if(currentItem->getLinkIconUrl().length() < 1) {
			queryTag = "image/ico\" href=\"";
			pos = 0;
			while(pos != std::string::npos) {
			pos = replyMsg.find(queryTag, pos);
				if(pos != std::string::npos) {
					int start = pos+strlen(queryTag);
					int end = replyMsg.find(".ico", start)+4;
					if(end == std::string::npos+4)
						break;
					std::string searchQuery(replyMsg.substr(start, end-start));
					if(searchQuery[0] != 'h') { //URL is relative, add the guessed location from above
						searchQuery.insert(0, "/");
						searchQuery.insert(0, location);
						std::cout << searchQuery << std::endl;
					}
					currentItem->setLinkIconUrl(searchQuery);
					std::cout << "Detected icon at: " << searchQuery << std::endl;
					pos = end;
					i++;
				}
			}
		}
		if(currentItem->getLinkIconUrl().length() < 1)
			currentItem->setLinkIconUrl(*new string("-Icon not found-")); //Make the menuItem draw generic icon
		
		/*Delete the buffer*/
		delete mallocIO;
	}
	
	return B_OK;
}

/*Callback function for cURL (favIcon download)*/
static size_t
WriteUrlCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size *nmemb;
	BMallocIO *mallocIO = (BMallocIO *)data;
	size_t written = mallocIO->Write(ptr, realsize);
	
	/*Only download head*/
	char *rawData = (char *)mallocIO->Buffer();
	std::string replyMsg(rawData, mallocIO->BufferLength());
	if(replyMsg.find("</head>") != std::string::npos)
		return 0;
	
	if(mallocIO->BufferLength() > 10000) {//We don't want to download large files.
		std::cout << "Aborting search for URL icon: Too large file" << std::endl;
		return 0;
	}
	else
		return written;
}

void
HTGTweetTextView::MouseUp(BPoint where)
{
	BMessage* message = Window()->CurrentMessage();

	HyperTextAction* action = _ActionAt(where);
	if (action != NULL)
		action->Clicked(this, where, message);

	BTextView::MouseUp(where);
}

void
HTGTweetTextView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	BMessage* message = Window()->CurrentMessage();

	uint32 buttons;
	HyperTextAction* action;
	if (message->FindInt32("buttons", (int32*)&buttons) == B_OK
		&& buttons == 0 && (action = _ActionAt(where)) != NULL) {
		action->MouseOver(this, where, message);
		return;
	}

	BTextView::MouseMoved(where, transit, dragMessage);
}

void
HTGTweetTextView::AddHyperTextAction(int32 startOffset, int32 endOffset,
	HyperTextAction* action)
{
	if (action == NULL || startOffset >= endOffset) {
		delete action;
		return;
	}

	fActionInfos->BinaryInsert(new ActionInfo(startOffset, endOffset, action),
		ActionInfo::Compare);

	// TODO: Of course we should check for overlaps...
}

HyperTextAction*
HTGTweetTextView::_ActionAt(const BPoint& where) const
{
	int32 offset = OffsetAt(where);

	ActionInfo pointer(offset, offset + 1, NULL);

    const ActionInfo* action = fActionInfos->BinarySearch(pointer,
			ActionInfo::CompareEqualIfIntersecting);
	if (action != NULL) {
		// verify that the text region was hit
		BRegion textRegion;
		GetTextRegion(action->startOffset, action->endOffset, &textRegion);
		if (textRegion.Contains(where))
			return action->action;
	}

	return NULL;
}
