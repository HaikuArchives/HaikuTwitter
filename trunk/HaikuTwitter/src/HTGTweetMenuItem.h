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
#include <MimeType.h>

#include <curl/curl.h>

#ifndef HTG_TWEETMENUITEM_H
#define HTG_TWEETMENUITEM_H

enum icon_source {
	kNoIcon = 0,
	kOwnIcon,
	kApplicationIcon,
	kSupertypeIcon
};

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
	
	~HTGTweetMenuItem();
	
private:
	thread_id downloadThread;
	std::string linkIconUrl;
	BBitmap *linkIcon;

};

extern status_t icon_for_type(const BMimeType& type, BBitmap& bitmap,
	icon_size size, icon_source* _source = NULL);

#endif
