/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <MimeType.h>
#include <fs_index.h>
#include <fs_info.h>
#include <NodeInfo.h>
#include <File.h>
#include <Message.h>
#include <Path.h>
#include <FindDirectory.h>

#include "HTTweet.h"

#ifndef HT_STORAGE
#define HT_STORAGE

#define HAIKUTWITTER_ATTR_ID			"HaikuTwitter:id" 			//Tweet id
#define HAIKUTWITTER_ATTR_FULLNAME		"HaikuTwitter:fullname" 	//Full name of author
#define HAIKUTWITTER_ATTR_SCREENNAME	"HaikuTwitter:screenname" 	//Screen name (twitter username) of author
#define HAIKUTWITTER_ATTR_WHEN			"HaikuTwitter:when" 		//Time & date tweet was created
#define HAIKUTWITTER_ATTR_TEXT			"HaikuTwitter:text" 		//The tweet's content
#define HAIKUTWITTER_ATTR_IMAGEURL		"HaikuTwitter:imageurl" 	//URL to author's profile image
#define HAIKUTWITTER_ATTR_SOURCE		"HaikuTwitter:source" 		//Name of tweet's source (Application)

class HTStorage {
public:
			static status_t			saveTweet(HTTweet*);
			static HTTweet*			loadTweet(entry_ref* ref);
			static void				makeMimeType(bool remakeMIMETypes);
			static void				makeIndices();
			
private:
			static status_t			getTweetPath(BPath&);
			static void				addAttribute(BMessage& msg, const char* name, const char* publicName, int32 type = B_STRING_TYPE, 
														bool viewable = true, bool editable = false, int32 width = 200);
};
#endif
