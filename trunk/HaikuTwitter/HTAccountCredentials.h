/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include <String.h>
#include <Handler.h>
#include <iostream>

#include "twitcurl/twitcurl.h"

#ifndef HT_ACCOUNTCREDENTIALS_H
#define HT_ACOOUNTCREDENTIALS_H


const int32 READY = 'ACRD';

class HTAccountCredentials {
public:
						HTAccountCredentials(twitCurl* twitObj, BHandler* msgHandler);
	virtual				~HTAccountCredentials();

			status_t	Fetch(int32 id = -1);
			status_t	FetchSelf();
			
			const char*	ScreenName() const;
			const char*	RealName() const;
			const char*	Location() const;
			const char*	Description() const;
			const char*	ProfileImageUrl() const;
			int32		CountFollowers() const;
			int32		Id() const;
			bool		Verified() const;	
private:
			BString		screenName;
			BString		realName;
			BString		location;
			BString		description;
			BString		profileImageUrl;
			int32		countFollowers;
			int32		id;
			bool		verified;
			
			twitCurl*	twitObj;
			BHandler*	msgHandler;
			
			const char* _FindValue(const char* tag, const std::string&);
};
#endif
