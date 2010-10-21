/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTimeLineView.h"

/*InfoPopper constants*/
const int32 kNotify				= 1000;
const int32 kInformationType	= 1001;
const int32 kImportantType		= 1002;
const int32 kErrorType			= 1003;
const int32 kProgressType		= 1004;
const int32 kAttributeIcon		= 1005;
const int32 kContentsIcon		= 1006;

HTGTimeLineView::HTGTimeLineView(twitCurl *twitObj, const int32 TYPE, BRect rect, const char* requestInfo, int textSize, bool saveTweets)
	: BView(rect, "ContainerView", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS)
{
	this->twitObj = twitObj;
	this->TYPE = TYPE;
	this->saveTweets = saveTweets;
	previousThread = B_NAME_NOT_FOUND;
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
		case TIMELINE_DIRECT:
			SetName("Msg");
			break;
		default:
			SetName("Public");
	}
	
	/*Set up listview*/
	this->listView = new BListView(BRect(0, 0, Bounds().Width()-15, Bounds().Height()), "ListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	
	/*Prepare the list for unhandled tweets*/
	unhandledList = new BList();
	
	/*Set up scrollview*/
	theScrollView = new BScrollView("scrollView", listView, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS, false, true);
	this->AddChild(theScrollView);
		
	/*Load infopopper settings*/
	wantsNotifications = false; //Default should be false
	wantsNotifications = _retrieveInfoPopperBoolFromSettings();
	
	/*Set text size*/
	BFont font;
	listView->GetFont(&font);
	font.SetSize(textSize);
	SetFont(&font);
	
	/*All done, ready to display tweets*/
	waitingForUpdate = true;
}

HTGTimeLineView::HTGTimeLineView(const int32 TYPE, BRect rect, BList* tweets, int textSize)
	: BView(rect, "ContainerView", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS)
{
	this->TYPE = TYPE;
	previousThread = B_NAME_NOT_FOUND;
	searchID = 0;
	twitObj = NULL;
	
	/*Set up listview*/
	this->listView = new BListView(BRect(0, 0, Bounds().Width()-15, Bounds().Height()), "ListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	
	/*Prepare the list for unhandled tweets*/
	unhandledList = new BList();
	
	/*Add the list of tweets*/
	HTTweet *currentTweet;
	for (int i = 0; i < tweets->CountItems(); i++) {
		currentTweet = (HTTweet *)tweets->ItemAt(i);

		/*Make a copy, download bitmap and add it to newList*/
		currentTweet->downloadBitmap();
		unhandledList->AddItem(new HTGTweetItem(currentTweet));
	}
	
	/*Set up scrollview*/
	theScrollView = new BScrollView("scrollView", listView, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS, false, true);
	this->AddChild(theScrollView);
	
	/*Set text size*/
	BFont font;
	listView->GetFont(&font);
	font.SetSize(textSize);
	SetFont(&font);
	
	/*All done, ready to display tweets*/
	waitingForUpdate = true;
}

void
HTGTimeLineView::setSaveTweets(bool saveTweets)
{
	this->saveTweets = saveTweets;
}

void
HTGTimeLineView::AddList(BList *tweets)
{
	/*Add the list of tweets*/
	HTTweet *currentTweet;
	for (int i = 0; i < tweets->CountItems(); i++) {
		currentTweet = (HTTweet *)tweets->ItemAt(i);

		/*Make a copy, download bitmap and add it to newList*/
		currentTweet->downloadBitmap();
		unhandledList->AddItem(new HTGTweetItem(currentTweet));
	}
	if(LockLooper()) {
		this->AttachedToWindow();
		UnlockLooper();
	}
}

void
HTGTimeLineView::clearList()
{
	/*Kill the update thread*/
	if(previousThread != B_NAME_NOT_FOUND)
		kill_thread(previousThread);
		
	while(!listView->IsEmpty()) {
		HTGTweetItem *currentItem = (HTGTweetItem *)listView->FirstItem();
		listView->RemoveItem(currentItem);
		if(currentItem->getTweetPtr() != NULL)
			currentItem->getTweetPtr()->setView(NULL);
		delete currentItem;
	}
}

void
HTGTimeLineView::setSearchID(int32 id)
{
	searchID = id;
}

int32
HTGTimeLineView::getSearchID()
{
	return searchID;
}

void
HTGTimeLineView::SetFont(const BFont *font, uint32 properties)
{
	listView->SetFont(font, properties);
	BView::SetFont(font, properties);
	listView->Invalidate();
}

void
HTGTimeLineView::AttachedToWindow()
{
	BView::AttachedToWindow();
	listView->AttachedToWindow();
	
	/*Add the unhandled tweets*/
	if(!unhandledList->IsEmpty()) {
		BList *newList = new BList();
		newList->AddList(unhandledList);
		unhandledList->MakeEmpty();
		
		HTGTweetItem *currentItem;
		while(!listView->IsEmpty()) {
			currentItem = (HTGTweetItem *)listView->FirstItem();
			listView->RemoveItem(currentItem); //Must lock looper before we do this!
			if(newList->CountItems() < 30) //Only allow 30 tweets to be displayed at once... for now.
				newList->AddItem(currentItem);
			else
				delete currentItem;
		}
		listView->AddList(newList);
		unhandledList->MakeEmpty();
	}
}

void
HTGTimeLineView::updateTimeLine()
{
	previousThread = spawn_thread(updateTimeLineThread, "UpdateThread", 10, this);
	resume_thread(previousThread);
	waitingForUpdate = false;
}

std::string&
htmlFormatedString(const char *orig)
{
	std::string newString(orig);
	if(orig[0] == '#') {
		//newString = std::string(orig+1); THIS IS NOT NEEDED ANYMORE.
		newString = std::string(orig);
	}
	std::string *returnPtr = new std::string(newString);
	return *returnPtr;
}

void
HTGTimeLineView::savedSearchDestoySelf()
{
	/*Destroy saved search on twitter*/
	twitCurl *saveObj = new twitCurl();
	saveObj->setAccessKey(twitObj->getAccessKey());
	saveObj->setAccessSecret(twitObj->getAccessSecret());
	std::string id;
	std::stringstream out;
	out << this->getSearchID(); //Converting int to string
	id = out.str();
	if(this->getSearchID() > 0)
		saveObj->savedSearchDestroy(id);
	delete saveObj;
}

void
HTGTimeLineView::savedSearchCreateSelf()
{
	/*Save search to twitter*/
	std::string query(::htmlFormatedString(Name()));
	twitCurl *saveObj = new twitCurl();
	saveObj->setAccessKey(twitObj->getAccessKey());
	saveObj->setAccessSecret(twitObj->getAccessSecret());
	saveObj->savedSearchCreate(query);
	std::string replyMsg(" ");
	saveObj->getLastWebResponse(replyMsg);

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
	delete saveObj;
}

status_t
updateTimeLineThread(void *data)
{
	//Could not figure out how to update a BListItem with a child view (BTextView).
	//Could be a bug in Haiku APIs. After hours of investigation without any
	//result, I just don't care anymore. Reallocating all HTGTweetItem on update.
		
	HTGTimeLineView *super = (HTGTimeLineView*)data;
	
	/*Wait for previous thread to end*/
	status_t junkId;
	wait_for_thread(find_thread("UpdateThread"), &junkId);
	
	BListView *listView = super->listView;
	BView *containerView;
	char *tabName;
	bool saveTweets = super->saveTweets;
	
	int32 TYPE = super->TYPE;
	twitCurl *twitObj = super->twitObj;
	TimeLineParser *timeLineParser = new TimeLineParser();
	SearchParser *searchParser = new SearchParser();
	DirectMessageParser *directParser = new DirectMessageParser();
	std::string replyMsg(" ");
	
	HTGTweetItem *mostRecentItem = NULL;
	HTTweet *mostRecentTweet = NULL;
	HTTweet *currentTweet = NULL;
	
	BList *newList = new BList();
	
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
		case TIMELINE_DIRECT:
			twitObj->directMessageGet();
			break;
		case TIMELINE_HDD:
			return B_OK;
			break;
		default:
			twitObj->timelinePublicGet();
	}
	
	twitObj->getLastWebResponse(replyMsg);
	
	#ifdef DEBUG_ENABLED
		std::cout << "---Reply from Twitter API---" << std::endl;
	 	std::cout << replyMsg << std::endl;
	 	std::cout << "---End of reply---" << std::endl;
	 #endif

	if(replyMsg.length() < 100)   //Length of data is less than 100 characters. Clearly,
		replyMsg = "error";			//something is wrong... abort.;

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
	else if(TYPE == TIMELINE_DIRECT) {
			try {
				directParser->readData(replyMsg.c_str());
			} catch( xercesc::XMLException& e ) {
				std::cout << "Error while parsing data." << std::endl;
				delete timeLineParser;
				timeLineParser = NULL;
				return B_OK;
			}
			timeLineParser = (TimeLineParser *)directParser; //I'm not so sure this is a good way to go,
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
			if(saveTweets)
				HTStorage::saveTweet(copiedTweet);
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

void
HTGTimeLineView::sendNotificationFor(HTTweet *theTweet)
{
	BNotification notification(B_INFORMATION_NOTIFICATION);

	//Prepare the message
	std::string title("New tweet from ");
	title.append(theTweet->getScreenName());
	notification.SetApplication("HaikuTwitter");
	notification.SetTitle(title.c_str());
	notification.SetContent(theTweet->getText().c_str());
	notification.SetOnClickApp("application/x-vnd.HaikuTwitter");
	theTweet->waitUntilDownloadComplete();
	notification.SetIcon(theTweet->getBitmap());

	//Send the notification
	be_roster->Notify(notification, (bigtime_t)0);
		
}

bool
HTGTimeLineView::_retrieveInfoPopperBoolFromSettings()
{
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
		case TIMELINE_SEARCH:
			return theSettings.searchesNotify;
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
		case TIMELINE_SEARCH:
			return theSettings.searchesNotify;
			break;
		}
	return false; //Just in case nothing get's picked up... no compile error/warning without this though
}

HTGTimeLineView::~HTGTimeLineView()
{
	/*Kill the update thread*/
	if(previousThread != B_NAME_NOT_FOUND)
		kill_thread(previousThread);
	
	listView->RemoveSelf();
	while(!listView->IsEmpty()) {
		HTGTweetItem *currentItem = (HTGTweetItem *)listView->FirstItem();
		listView->RemoveItem(currentItem);
		if(currentItem->getTweetPtr() != NULL)
			currentItem->getTweetPtr()->setView(NULL);
		delete currentItem;
	}
	delete listView;
	
	while(!unhandledList->IsEmpty()) {
		HTGTweetItem *currentItem = (HTGTweetItem *)unhandledList->FirstItem();
		unhandledList->RemoveItem(currentItem);
		if(currentItem->getTweetPtr() != NULL)
			currentItem->getTweetPtr()->setView(NULL);
		delete currentItem;
	}
	delete unhandledList;
	
	if(twitObj != NULL)
		delete twitObj;
	theScrollView->RemoveSelf();
	delete theScrollView;
}
