/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTimeLineView.h"

/*InfoPopper constants*/
const int32 kNotify		= 1000;
const int32 kInformationType	= 1001;
const int32 kImportantType	= 1002;
const int32 kErrorType		= 1003;
const int32 kProgressType	= 1004;
const int32 kAttributeIcon	= 1005;
const int32 kContentsIcon	= 1006;

HTGTimeLineView::HTGTimeLineView(twitCurl *twitObj, const int32 TYPE, BRect rect, const char* requestInfo) : BView(rect, "ContainerView", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS) {	
	this->twitObj = twitObj;
	this->TYPE = TYPE;
	thread_id previousThread = B_NAME_NOT_FOUND;
	searchID = 0;
	
	/*Set view name*/
	switch(TYPE) {
		case TIMELINE_HOME:
			SetName("Home");
			break;
		case TIMELINE_FRIENDS:
			SetName("Friends");
			break;
		case TIMELINE_MENTIONS:
			SetName("Mentions");
			break;
		case TIMELINE_PUBLIC:
			SetName("Public");
			break;
		case TIMELINE_USER:
			SetName(requestInfo);
			break;
		case TIMELINE_SEARCH:
			SetName(requestInfo);
			break;
		default:
			SetName("Public");
	}
	
	/*Set up listview*/
	this->listView = new BListView(BRect(0, 0, 300, Bounds().Height()), "ListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	
	/*Prepare the list for unhandled tweets*/
	unhandledList = new BList();
	
	/*Set up scrollview*/
	theScrollView = new BScrollView("scrollView", listView, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS, false, true);
	this->AddChild(theScrollView);
		
	/*Load infopopper settings (if supported)*/
	wantsNotifications = false; //Default should be false
	#ifdef INFOPOPPER_SUPPORT
	wantsNotifications = _retrieveInfoPopperBoolFromSettings();
	#endif
	
	/*All done, ready to display tweets*/
	waitingForUpdate = true;
	updateTimeLine();
}

void HTGTimeLineView::setSearchID(int32 id) {
	searchID = id;
}

int32 HTGTimeLineView::getSearchID() {
	return searchID;
}

void HTGTimeLineView::AttachedToWindow() {
	BView::AttachedToWindow();
	if(waitingForUpdate)
		updateTimeLine();
}

void HTGTimeLineView::updateTimeLine() {
	bool looperLocked = listView->LockLooper();
	/*Update timeline only if we can lock window looper -or we want notifications for this timeline*/
	if(!looperLocked && !wantsNotifications)
		waitingForUpdate = true;
	else {
		if(looperLocked) listView->UnlockLooper(); //Assume we can lock it later
		previousThread = spawn_thread(updateTimeLineThread, "UpdateThread", 10, this);
		resume_thread(previousThread);
		waitingForUpdate = false;
	}
}

std::string& htmlFormatedString(const char *orig) {
	std::string newString(orig);
	if(orig[0] == '#') {
		//newString = std::string(orig+1); THIS IS NOT ANYMORE.
		newString = std::string(orig);
	}
	std::string *returnPtr = new std::string(newString);
	return *returnPtr;
}

void HTGTimeLineView::savedSearchDestoySelf() {
	/*Destroy saved search on twitter*/
	std::string id;
	std::stringstream out;
	out << this->getSearchID(); //Converting int to string
	id = out.str();
	if(this->getSearchID() > 0)
		twitObj->savedSearchDestroy(id);
}

void HTGTimeLineView::savedSearchCreateSelf() {
	/*Save search to twitter*/
	std::string query(::htmlFormatedString(Name()));
	twitObj->savedSearchCreate(query);
	std::string replyMsg(" ");
	twitObj->getLastWebResponse(replyMsg);

	/*Parse result*/
	int pos = 0;
	const char *idTag = "<id>";
	pos = replyMsg.find(idTag, pos);
	if(pos != std::string::npos) {
		int start = pos+strlen(idTag);
		int end = replyMsg.find("</id>", start);
		std::string searchID(replyMsg.substr(start, end-start));
		setSearchID(atoi(searchID.c_str()));
	}	
}

status_t updateTimeLineThread(void *data) {
	//Could not figure out how to update a BListItem with a child view (BTextView).
	//Could be a bug in Haiku API's. After hours of investigation without any
	//result, I just don't care anymore. Reallocating all HTGTweetItem on update.
	
	HTGTimeLineView *super = (HTGTimeLineView*)data;
	
	/*Wait for previous thread to end*/
	status_t junkId;
	wait_for_thread(find_thread("UpdateThread"), &junkId);
	
	BListView *listView = super->listView;
	BView *containerView;
	char *tabName;
	
	int32 TYPE = super->TYPE;
	twitCurl *twitObj = super->twitObj;
	TimeLineParser *timeLineParser = new TimeLineParser();
	SearchParser *searchParser = new SearchParser();
	
	switch(TYPE) {
		case TIMELINE_HOME:
			twitObj->timelineHomeGet();
			break;
		case TIMELINE_FRIENDS:
			twitObj->timelineFriendsGet();
			break;
		case TIMELINE_MENTIONS:
			twitObj->mentionsGet();
			break;
		case TIMELINE_PUBLIC:
			twitObj->timelinePublicGet();
			break;
		case TIMELINE_USER:
			twitObj->timelineUserGet(*new std::string(super->Name()), false);
			break;
		case TIMELINE_SEARCH:
			twitObj->search(htmlFormatedString(super->Name()));
			break;
		default:
			twitObj->timelinePublicGet();
	}
	std::string replyMsg(" ");
	twitObj->getLastWebResponse(replyMsg);
	if(replyMsg.length() < 100)  { //Length of data is less than 100 characters. Clearly,
		replyMsg = "error";			//something is wrong... abort.
	}
	
	if(TYPE == TIMELINE_SEARCH) {
			try {
				searchParser->readData(replyMsg.c_str());
			} catch( xercesc::XMLException& e ) {
				std::cout << "Error while parsing data." << std::endl;
				delete timeLineParser;
				timeLineParser = NULL;
				return B_OK;
			}
			timeLineParser = (TimeLineParser *)searchParser; //I'm not so sure this is a good way to go,
															//so don't tell anyone;-)
	}
	else {
		try {
			timeLineParser->readData(replyMsg.c_str());
		} catch( xercesc::XMLException& e ) {
				std::cout << "Error while parsing data." << std::endl;
				delete timeLineParser;
				timeLineParser = NULL;
				return B_OK;
		}
	}
	
	if(timeLineParser->count() < 1) {//timeLineParser failed, return!
			std::cout << "Parser didn't find any tweets." << std::endl;
			delete timeLineParser;
			timeLineParser = NULL;
			return B_OK;
		}
	
	HTGTweetItem *mostRecentItem = NULL;
	HTTweet *mostRecentTweet = NULL;
	HTTweet *currentTweet = NULL;
	
	bool initialLoad = (listView->FirstItem() == NULL && super->unhandledList->FirstItem() == NULL);
	
	if(!initialLoad && listView->FirstItem() != NULL) {
		mostRecentItem = (HTGTweetItem *)listView->FirstItem();
		mostRecentTweet = mostRecentItem->getTweetPtr();
		currentTweet = timeLineParser->getTweets()[0];
	
		/*If we are up to date: redraw, clean up and return*/
		if(!(*mostRecentTweet < *currentTweet)) {
			if(listView->LockLooper()) {
				for(int i = 0; i < listView->CountItems(); i++) {
					listView->InvalidateItem(i); //Update date
				}
				listView->UnlockLooper();
			}
			delete timeLineParser;
			timeLineParser = NULL;
			return B_OK;
		}
	}
	
	/*Check to see if there is some unhandled tweets*/
	if(!super->unhandledList->IsEmpty()) {
		mostRecentItem = (HTGTweetItem *)super->unhandledList->FirstItem();
		mostRecentTweet = mostRecentItem->getTweetPtr();
	}
		
	BList *newList = new BList();
	
	for (int i = 0; i < timeLineParser->count(); i++) {
		currentTweet = timeLineParser->getTweets()[i];
		bool addItem = initialLoad;
		if(!initialLoad) {
			if(mostRecentTweet != NULL)
				addItem = (*mostRecentTweet < *currentTweet);
			else
				addItem = true;
		}
		if(addItem) {
			/*Make a copy, download bitmap and add it to newList*/
			HTTweet *copiedTweet = new HTTweet(currentTweet);
			copiedTweet->downloadBitmap();
			newList->AddItem(new HTGTweetItem(copiedTweet));
			
			if(!initialLoad && super->wantsNotifications) { //New tweet arrived, send notification
				super->sendNotificationFor(copiedTweet);
			}
		}
		else
			break;
	}
	
	/*Try to lock listView*/
	if(!listView->LockLooper()) {
		/*Not active view: Copy tweetptrs to unhandledList and return*/
		super->unhandledList->AddList(newList, 0); //Add new tweets to the top
		super->waitingForUpdate = true;
		delete timeLineParser;
		timeLineParser = NULL;
		if(newList->IsEmpty())
			delete newList;
		return B_OK;
	}
	
	/*Add the unhandled tweets to newList*/
	newList->AddList(super->unhandledList);
	super->unhandledList->MakeEmpty();
	
	HTGTweetItem *currentItem;
	while(!listView->IsEmpty()) {
		currentItem = (HTGTweetItem *)listView->FirstItem();
		currentTweet = currentItem->getTweetPtr();
		listView->RemoveItem(currentItem); //Must lock looper before we do this!
		if(newList->CountItems() < 20) //Only allow 20 tweets to be displayed at once... for now.
			newList->AddItem(new HTGTweetItem(new HTTweet(currentTweet)));
			
		delete currentItem;
	}
	
	/*Update the view*/
	listView->AddList(newList); //Must lock looper before we do this!
			
	/*Cleanup*/
	super->waitingForUpdate = false;
	listView->UnlockLooper();
	delete timeLineParser;
	timeLineParser = NULL;
	delete newList;

	return B_OK;
}

void HTGTimeLineView::sendNotificationFor(HTTweet *theTweet) {	
	std::string title("New tweet from ");
	title.append(theTweet->getScreenName());
	std::cout << title << std::endl;
	
	#ifdef INFOPOPPER_SUPPORT
	
	IPConnection *conn = new IPConnection;
	IPMessage *msg = new IPMessage(InfoPopper::Information);

	//Prepare the message
	msg->Application("HaikuTwitter");
	msg->Title(title.c_str());
	msg->Content(theTweet->getText().c_str());

	// Icon maybe in the future
	/*if (strlen(fIconFile->Text()) > 0) {
		entry_ref ref;

		if (get_ref_for_path(fIconFile->Text(), &ref) == B_OK) {
			if (fSelectedIconType == InfoPopper::Attribute) {
				msg->MainIcon(ref);
				msg->MainIconType(fSelectedIconType);
			}
			if (fSelectedIconType == InfoPopper::Contents) {
				msg->OverlayIcon(ref);
				msg->OverlayIconType(fSelectedIconType);
			}
		}
	}*/

	//Send the notification
	conn->Send(msg);

	delete msg;
	delete conn;
		
	#endif
}

bool HTGTimeLineView::_retrieveInfoPopperBoolFromSettings() {
	BPath path;
	infopopper_settings theSettings;
	
	if (HTGInfoPopperSettingsWindow::_getSettingsPath(path) < B_OK) {
		theSettings = HTGInfoPopperSettingsWindow::_getDefaults();
		switch(TYPE) {
		case TIMELINE_FRIENDS:
			return theSettings.friendsNotify;
			break;
		case TIMELINE_MENTIONS:
			return theSettings.mentionsNotify;
			break;
		case TIMELINE_PUBLIC:
			return theSettings.publicNotify;
			break;
		}
	}
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK) {
		theSettings = HTGInfoPopperSettingsWindow::_getDefaults();
		switch(TYPE) {
		case TIMELINE_FRIENDS:
			return theSettings.friendsNotify;
			break;
		case TIMELINE_MENTIONS:
			return theSettings.mentionsNotify;
			break;
		case TIMELINE_PUBLIC:
			return theSettings.publicNotify;
			break;
		}
	}
	
	file.ReadAt(0, &theSettings, sizeof(infopopper_settings));
	switch(TYPE) {
		case TIMELINE_FRIENDS:
			return theSettings.friendsNotify;
			break;
		case TIMELINE_MENTIONS:
			return theSettings.mentionsNotify;
			break;
		case TIMELINE_PUBLIC:
			return theSettings.publicNotify;
			break;
		}
	return false; //Just in case nothing get's picked up... no compile error/warning without this though
}

HTGTimeLineView::~HTGTimeLineView() {
	/*Kill the update thread*/
	kill_thread(previousThread);
	
	listView->RemoveSelf();
	while(!listView->IsEmpty()) {
		HTGTweetItem *currentItem = (HTGTweetItem *)listView->FirstItem();
		listView->RemoveItem(currentItem);			
		delete currentItem;
	}
	delete listView;
	delete twitObj;
	theScrollView->RemoveSelf();
	delete theScrollView;
}
