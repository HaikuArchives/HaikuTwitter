/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
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
	dragger = NULL;
	isReplicant = false;
	this->twitObj = twitObj;
	this->TYPE = TYPE;
	this->saveTweets = saveTweets;
	this->errorCount = 0;
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
	this->listView = new HTGListView(BRect(0, 0, Bounds().Width()+1-B_V_SCROLL_BAR_WIDTH, Bounds().Height()), "ListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	
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
	dragger = NULL;
	isReplicant = false;
	this->TYPE = TYPE;
	this->errorCount = 0;
	previousThread = B_NAME_NOT_FOUND;
	searchID = 0;
	twitObj = NULL;
	
	/*Set up listview*/
	this->listView = new HTGListView(BRect(0, 0, Bounds().Width()+1-B_V_SCROLL_BAR_WIDTH, Bounds().Height()), "ListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	
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
	unhandledList->SortItems(*HTGTweetItem::sortByDateFunc);
	
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

HTGTimeLineView::HTGTimeLineView(BMessage* archive)
	: BView(archive)
{
	dragger = NULL;
	const char **ptr;
	
	isReplicant = true;
	errorCount = 0;

	archive->FindBool("HTGTimeLineView::waitingForUpdate", &waitingForUpdate);
	archive->FindBool("HTGTimeLineView::wantsNotifications", &wantsNotifications);
	archive->FindBool("HTGTimeLineView::saveTweets", &saveTweets);
	archive->FindString("HTGTimeLineView::name", ptr);
	archive->FindInt32("HTGTimeLineView::TYPE", &TYPE);
	SetName(*ptr);
	
	BMessage msg;
	BArchivable* unarchived;
	
	/*Set up listView*/
	if(archive->FindMessage("HTGTimeLineView::listView", &msg) == B_OK) {
		unarchived = instantiate_object(&msg);
		if(unarchived) {
			std::cout << "Unable to instantiate archived <HTGTimeLineView::listView>." << std::endl;
			listView = dynamic_cast<HTGListView *>(unarchived);
			listView->ResizeTo(Bounds().Width(), Bounds().Height()); //No scrollview, so resize to fit whole view
		}else {
			listView = new HTGListView(Bounds(), "ListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
		}
	}
	
	/*Set up unhandled list*/
	int i;
	unhandledList = new BList();
	while(archive->FindMessage("HTGTimeLineView::unhandled", i++, &msg) == B_OK) {
		unarchived = instantiate_object(&msg);
		if(unarchived)
			unhandledList->AddItem(dynamic_cast<HTGTweetItem *>(unarchived));
		else
			std::cout << "Unable to instantiate archived <HTGTimeLineView::unhandled>." << std::endl;
	}
	
	/*Set up twitcurl*/
	twitObj = new twitCurl();
	archive->FindString("HTGTimeLineView::oauthKey", ptr);
	std::string key(*ptr);
	archive->FindString("HTGTimeLineView::oauthSecret", ptr);
	std::string secret(*ptr);
	twitObj->setAccessKey(key);
	twitObj->setAccessSecret(secret);
	
	previousThread = B_NAME_NOT_FOUND;
	
	//NOTE: Should we also set the font?
	AddChild(listView);

	waitingForUpdate = true;
	updateTimeLine();
}

BArchivable*
HTGTimeLineView::Instantiate(BMessage* archive)
{
	if(!validate_instantiation(archive, "HTGTimeLineView"))
		return NULL;
	return new HTGTimeLineView(archive);
}

status_t
HTGTimeLineView::Archive(BMessage* archive, bool deep) const
{
	/*Kill the update thread*/
	if(previousThread != B_NAME_NOT_FOUND)
		kill_thread(previousThread);
	
	status_t status = BView::Archive(archive, deep);
	if(status == B_OK)
		status = archive->AddString("add_on", "application/x-vnd.HaikuTwitter");
	if(status == B_OK)
		status = archive->AddString("class", "HTGTimeLineView");

	/*Archive ivars*/
	if(status == B_OK)
		status = archive->AddBool("HTGTimeLineView::waitingForUpdate", waitingForUpdate);
	if(status == B_OK)
		status = archive->AddBool("HTGTimeLineView::wantsNotifications", wantsNotifications);
	if(status == B_OK)
		status = archive->AddBool("HTGTimeLineView::saveTweets", saveTweets);
	if(status == B_OK)
		status = archive->AddInt32("HTGTimeLineView::searchID", searchID);
	if(status == B_OK)
		status = archive->AddInt32("HTGTimeLineView::TYPE", TYPE);
	if(status == B_OK)
		status = archive->AddString("HTGTimeLineView::name", Name());
	
	/*Archive login information*/
	if(status == B_OK)
		status = archive->AddString("HTGTimeLineView::oauthSecret", twitObj->getAccessSecret().c_str());
	if(status == B_OK)
		status = archive->AddString("HTGTimeLineView::oauthKey", twitObj->getAccessKey().c_str());
	
	if(deep) {	
		BMessage listViewArchive;
		if(listView->Archive(&listViewArchive, deep) == B_OK) {
			if(status == B_OK)
				status = archive->AddMessage("HTGTimeLineView::listView", &listViewArchive);
		}
		
		BMessage unhandled;
		HTGTweetItem* currentItem;
		for(int i; i < unhandledList->CountItems(); i++) {
			currentItem = (HTGTweetItem* )(unhandledList->ItemAt(i));
			if(currentItem->Archive(&unhandled, deep) == B_OK) {
				if(status == B_OK)
					status = archive->AddMessage("HTGTimeLineView::unhandled", &unhandled);
			}
		}
	}
	
	return status;
}

void
HTGTimeLineView::FrameResized(float width, float height)
{
	if(dragger != NULL) {
		dragger->Invalidate();
	}
}

void
HTGTimeLineView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case REFRESH:
			this->updateTimeLine();
			break;
		default:
			BView::MessageReceived(msg);
	}
}

void
HTGTimeLineView::showScrollbar(bool show)
{
	if(show) {
		listView->RemoveSelf();
		dragger->RemoveSelf();
		delete dragger;
		dragger = NULL;
		AddChild(theScrollView);
		listView->ResizeTo(Bounds().Width()-15, Bounds().Height());
	}
	else {
		theScrollView->RemoveSelf();
		listView->RemoveSelf();
		listView->ResizeTo(Bounds().Size());
		dragger = new BDragger(Bounds(), this, B_FOLLOW_ALL);
		AddChild(dragger);
		AddChild(listView);
	}
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
		this->addUnhandledTweets();
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

bool
HTGTimeLineView::IsReplicant()
{
	return isReplicant;
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
	
	if(IsReplicant()) {
		/*Make background transparent*/
		listView->SetViewColor(Parent()->ViewColor());
		SetViewColor(Parent()->ViewColor());
		listView->SetHighColor(ViewColor());
		
		/*Setup runner to refresh the timeline*/
		BMessageRunner *refreshTimer = new BMessageRunner(this, new BMessage(REFRESH), 3*1000000*60);
		
		/*We don't want notifications for replicants*/
		wantsNotifications = false;
	}
	
	/*Add all unhandled tweets to timeline*/
	addUnhandledTweets(); 
}

void
HTGTimeLineView::addUnhandledTweets()
{
	if(unhandledList->IsEmpty())
		return;
		
	BList *newList = new BList();

	newList->AddList(unhandledList);
	unhandledList->MakeEmpty();
	
	HTGTweetItem *currentItem;
	while(!listView->IsEmpty()) {
		currentItem = (HTGTweetItem *)listView->FirstItem();
		listView->RemoveItem(currentItem);
		if(newList->CountItems() < 30) {//Only allow 30 tweets to be displayed at once... for now.
			newList->AddItem(currentItem);
			currentItem->ClearView(); //No need to keep everything in memory
									//if the view is at the bottom of the scrollbar (invisible)
		}
		else
			delete currentItem;
	}
	
	/*Sort tweets by date*/
	newList->SortItems(*HTGTweetItem::sortByDateFunc);
	
	/*Add our new list*/
	listView->AddList(newList);
		
	/*Clean up*/
	unhandledList->MakeEmpty();
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
	
	HTGListView *listView = super->listView;
	bool saveTweets = super->saveTweets;
	int32 errorCount = super->errorCount;
	
	int32 TYPE = super->TYPE;
	twitCurl *twitObj = super->twitObj;
	HTTimelineParser *timeLineParser;
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

	if(replyMsg.length() < 10) {
		errorCount++;
		if(errorCount < kMaximumRetries) {
			super->waitingForUpdate = true;
			super->updateTimeLine();
			std::cout << "No connection, retry: " << errorCount << "/" << kMaximumRetries << std::endl;
			return B_ERROR;
		}
		else
			replyMsg= string("error");
	}
	else
		errorCount = 0;

	if(TYPE == TIMELINE_SEARCH)
		timeLineParser = new HTSearchParser();
	else
		timeLineParser = new HTTimelineParser();

	try {
		timeLineParser->Parse(replyMsg);
	}
	catch( ... ) {
		std::cout << super->Name() << ": Error while parsing data." << std::endl;
		delete timeLineParser;
		timeLineParser = NULL;
		return B_OK;
	}
	
	if(timeLineParser->Tweets()->CountItems() < 1) {//timeLineParser failed, return!
		std::cout << super->Name() << ": Parser didn't find any tweets." << std::endl;
		delete timeLineParser;
		timeLineParser = NULL;
		return B_OK;
	}
	
	bool initialLoad = (listView->FirstItem() == NULL && super->unhandledList->FirstItem() == NULL);
	
	if(!initialLoad && listView->FirstItem() != NULL) {
		mostRecentItem = (HTGTweetItem *)listView->FirstItem();
		mostRecentTweet = mostRecentItem->getTweetPtr();
		currentTweet = (HTTweet *)timeLineParser->Tweets()->ItemAt(0);
	
		/*If we are up to date: redraw, clean up and return - Note this should not be done here,
		rather as a result of some BPulse I guess...*/
		if(!(*mostRecentTweet < *currentTweet)) {
			if(listView->LockLooper()) {
				for(int i = 0; i < listView->CountItems(); i++)
					listView->InvalidateItem(i); //Update date
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
	
	for (int i = 0; i < timeLineParser->Tweets()->CountItems(); i++) {
		currentTweet = (HTTweet *)timeLineParser->Tweets()->ItemAt(i);
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
	
	super->unhandledList->AddList(newList, 0);
	
	/*Try to lock listView*/
	if(!listView->LockLooper()) {
		/*Not active view: return*/
		super->waitingForUpdate = true;
		delete timeLineParser;
		timeLineParser = NULL;
		delete newList;
		
		return B_OK;
	}
	
	/*Add the tweets to timeline*/
	super->addUnhandledTweets();
			
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
	title.append(theTweet->getFullName());
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
	if(!IsReplicant()) {
		theScrollView->RemoveSelf();
		delete theScrollView;
	}
}
