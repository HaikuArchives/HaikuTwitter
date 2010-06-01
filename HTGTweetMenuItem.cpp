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
	
	if(linkIconUrl.find(".ico") == std::string::npos) {
		/*Load generic icon*/
		BRect iconRect(0, 0, B_MINI_ICON-1, B_MINI_ICON-1);
		linkIcon = new BBitmap(iconRect, B_RGBA32);
		BMimeType mimeType("text/html");
		status_t status=icon_for_type(mimeType,*linkIcon,B_MINI_ICON);
		
		if(Menu()->LockLooper()) {
			Menu()->Invalidate();
			Menu()->UnlockLooper();
		}
	}
	else if(!linkIconDownloadInProgress) {
		downloadLinkIcon();
	}
}

const std::string HTGTweetMenuItem::getLinkIconUrl() {
	return linkIconUrl;
}

void HTGTweetMenuItem::downloadLinkIcon() {
	downloadThread = spawn_thread(_threadDownloadLinkIcon, linkIconUrl.c_str(), 10, this);
	resume_thread(downloadThread);
}

void HTGTweetMenuItem::DrawContent() {
	if(linkIcon != NULL) {
		Menu()->SetDrawingMode(B_OP_ALPHA);
		Menu()->DrawBitmap(linkIcon, BPoint(ContentLocation().x-11.5, ContentLocation().y-1.5));
		Menu()->SetDrawingMode(B_OP_OVER);
	}
	
	Menu()->DrawString(Label(), BPoint(ContentLocation().x+6, ContentLocation().y+10.5));
}

HTGTweetMenuItem::~HTGTweetMenuItem() {
	/*Kill the update thread*/
	kill_thread(downloadThread);
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

status_t
icon_for_type(const BMimeType& type, BBitmap& bitmap, icon_size size,
	icon_source* _source)
{
	icon_source source = kNoIcon;

	if (type.GetIcon(&bitmap, size) == B_OK)
		source = kOwnIcon;

	if (source == kNoIcon) {
		// check for icon from preferred app

		char preferred[B_MIME_TYPE_LENGTH];
		if (type.GetPreferredApp(preferred) == B_OK) {
			BMimeType preferredApp(preferred);

			if (preferredApp.GetIconForType(type.Type(), &bitmap, size) == B_OK)
				source = kApplicationIcon;
		}
	}

	if (source == kNoIcon) {
		// check super type for an icon

		BMimeType superType;
		if (type.GetSupertype(&superType) == B_OK) {
			if (superType.GetIcon(&bitmap, size) == B_OK)
				source = kSupertypeIcon;
			else {
				// check the super type's preferred app
				char preferred[B_MIME_TYPE_LENGTH];
				if (superType.GetPreferredApp(preferred) == B_OK) {
					BMimeType preferredApp(preferred);

					if (preferredApp.GetIconForType(superType.Type(),
							&bitmap, size) == B_OK)
						source = kSupertypeIcon;
				}
			}
		}
	}

	if (_source)
		*_source = source;

	return source != kNoIcon ? B_OK : B_ERROR;
}
