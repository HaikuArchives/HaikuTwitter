/*
 * Copyright 2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#ifndef HTSEARCHPARSER_H
#define HTSEARCHPARSER_H

#include "HTTimelineParser.h"

class BList;
class HTSearchParser : public HTTimelineParser
{
public:
								HTSearchParser();
	virtual						~HTSearchParser();
	
			status_t			Parse(const std::string&);

private:
			status_t			_ParseNodes(BList* nodeList, BList* result);
			size_t				_FindProfileImage(std::string* buffer, const std::string& data);
			
			time_t				_StrToTime(const char*); //Convert from Twitter format to time_t
			uint64				_StrToId(const char* str);
};
#endif
