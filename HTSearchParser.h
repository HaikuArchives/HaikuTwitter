/*
 * Copyright 2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#ifndef HTSEARCHPARSER_H
#define HTSEARCHPARSER_H

#include <string>
#include "HTTweet.h"

class BList;
class HTSearchParser
{
public:
								HTSearchParser();
	virtual						~HTSearchParser();
	
			status_t			Parse(const std::string&);
	
			BList*				Tweets();

private:
			status_t			_ParseNodes(BList* nodeList, BList* result);
			
			time_t				_StrToTime(const char*); //Convert from Twitter format to time_t
			//uint64				_StrToId(const char*); //Extract id from <id> tag
			
			size_t				FindValue(std::string*, const char*, const std::string&, size_t);
			
			BList*				fTweets;
};
#endif
