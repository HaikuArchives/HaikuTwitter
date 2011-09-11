/*
 * Copyright 2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#ifndef HTTIMELINEPARSER_H
#define HTTIMELINEPARSER_H

#include <string>
#include "HTTweet.h"

class BList;
class HTTimelineParser
{
public:
								HTTimelineParser();
	virtual						~HTTimelineParser();
	
	virtual	status_t			Parse(const std::string&);
	
			BList*				Tweets();

protected:
			BList*				fTweets;
			size_t				FindValue(std::string*, const char*, const std::string&, size_t, bool decodeHtml = true);

private:
	virtual	status_t			_ParseNodes(BList* nodeList, BList* result);
			
	virtual	time_t				_StrToTime(const char*); //Convert from Twitter format to time_t
	virtual	int32				_StrToMonth(const char *) const;
			//uint64			_StrToId(const char*); //Extract id from <id> tag
};
#endif
