/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "HTGTweetMenuItem.h"

HTGTweetMenuItem::HTGTweetMenuItem(const char* label, BMessage* message, char shortcut, uint32 modifiers) : BMenuItem(label, message, shortcut, modifiers) {
	linkIconUrl = std::string("");
	linkIcon = NULL;
	linkIconDownloadInProgress = false;
}

HTGTweetMenuItem::HTGTweetMenuItem(BMenu* submenu, BMessage* message) : BMenuItem(submenu, message) {

}

HTGTweetMenuItem::HTGTweetMenuItem(BMessage* archive) : BMenuItem(archive) {

}

void HTGTweetMenuItem::setLinkIcon(BBitmap* linkIcon) {
	this->linkIcon = linkIcon;
}

void HTGTweetMenuItem::setLinkIconUrl(std::string& url) {
	this->linkIconUrl = url;
	downloadLinkIcon();
}

const std::string HTGTweetMenuItem::getLinkIconUrl() {
	return linkIconUrl;
}

void HTGTweetMenuItem::downloadLinkIcon() {
	if(linkIconUrl.find(".ico") == std::string::npos) {
		linkIcon = NULL;
	}
	else if(!linkIconDownloadInProgress) {
		downloadThread = spawn_thread(_threadDownloadLinkIcon, linkIconUrl.c_str(), 10, this);
		resume_thread(downloadThread);
	}
}

void HTGTweetMenuItem::DrawContent() {
	if(linkIcon != NULL) {
		Menu()->SetDrawingMode(B_OP_ALPHA);
		Menu()->DrawBitmap(linkIcon, BPoint(ContentLocation().x-10, ContentLocation().y-2));
		Menu()->SetDrawingMode(B_OP_OVER);
	}
	Menu()->MoveTo(0,0);
	BMenuItem::DrawContent();
}

status_t _threadDownloadLinkIcon(void *data) {
	HTGTweetMenuItem *super = (HTGTweetMenuItem*)data;
	super->linkIconDownloadInProgress = true;
	CURL *curl_handle;
	BMallocIO *mallocIO = new BMallocIO();
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, super->getLinkIconUrl().c_str());
	
	/*send all data to this function*/
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	
	/*we pass out 'mallocIO' object to the callback function*/
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)mallocIO);
	
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "haikutwitter-agent/1.0");
	
	/*get the data*/
	if(curl_easy_perform(curl_handle) < 0)
		std::cout << "libcURL: Download of bitmap failed." << std::endl;
	
	/*cleanup curl stuff*/
	curl_easy_cleanup(curl_handle);
	
	/*Translate downloaded data to bitmap*/
	BBitmap *theBitmap = BTranslationUtils::GetBitmap(mallocIO);
	if(theBitmap->IsValid())
		super->setLinkIcon(BTranslationUtils::GetBitmap(mallocIO));

	if(super->Menu() != NULL && theBitmap->IsValid()) {
		if(super->Menu()->LockLooper()) {
			std::string newLabel(super->Label());
			newLabel.insert(0, "  ");
			super->SetLabel(newLabel.c_str());
			super->Menu()->Invalidate();
			super->Menu()->UnlockLooper();
		}
	}
	
	/*Delete the buffer*/
	delete mallocIO;
	
	super->linkIconDownloadInProgress = false;
	return B_OK;
}

/*Callback function for cURL (favIcon download)*/
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
	size_t realsize = size *nmemb;
	BMallocIO *mallocIO = (BMallocIO *)data;
	
	return mallocIO->Write(ptr, realsize);
}
