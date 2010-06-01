/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include <MenuItem.h>
#include <View.h>
#include <string>
#include <iostream>

#include <TranslationUtils.h>
#include <TranslatorFormats.h>
#include <DataIO.h>
#include <Bitmap.h>

#include <curl/curl.h>

#ifndef HTG_TWEETMENUITEM_H
#define HTG_TWEETMENUITEM_H

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);
status_t _threadDownloadLinkIcon(void *);

class HTGTweetMenuItem : public BMenuItem {
public:
	HTGTweetMenuItem(const char* label, BMessage* message, char shortcut = 0, uint32 modifiers = 0);
	HTGTweetMenuItem(BMenu* submenu, BMessage* message = NULL);
	HTGTweetMenuItem(BMessage* archive);
	
	void DrawContent();
	
	const std::string getLinkIconUrl();
	void setLinkIconUrl(std::string& url);
	
	void setLinkIcon(BBitmap* theBitmap);	
	bool linkIconDownloadInProgress;
	void downloadLinkIcon();
	
private:
	thread_id downloadThread;
	std::string linkIconUrl;
	BBitmap *linkIcon;

};
#endif
