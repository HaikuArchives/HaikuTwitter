/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <File.h>
#include <Path.h>
#include <Message.h>
#include <String.h>
#include <FindDirectory.h>
#include <Handler.h>

#ifndef HT_RESTPARSER
#define HT_RESTPARSER

class HTRestParser : public BHandler {
public:
							HTRestParser();
	virtual					~HTRestParser();
	
			void			parseTimeline(const BString& xmlData, BMessage& reply, const BMessenger& target);
			void			parseSearchResult(const BString& xmlData, BMessage& reply, const BMessenger& target);
			
			static BString	loadSampleData();
private:
			static status_t	_getLocalPath(BPath&);
};
#endif
