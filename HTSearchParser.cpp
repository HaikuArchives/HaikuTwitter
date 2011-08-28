/*
 * Copyright 2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTSearchParser.h"

#include <string>
#include <iostream>
#include <sstream>
#include <List.h>

using namespace std;

//Define some constants for parsing
namespace TwitterTags {
	const char* ENTRY_TAG 			= "<entry>";
	const char* WHEN_TAG			= "<published>";
	const char* CONTENT_TAG			= "<title>";
	const char* AUTHOR_TAG			= "<author>";
	const char* NAME_TAG		 	= "<name>";
	const char* WHERE_TAG			= "<uri>";
	const char* ID_TAG				= "<id>";
	const char* PROFILEIMAGEURL_TAG	= "<link>";
}

//Define some constants for using the twitter api
namespace TwitterAPI {	
	const string TIME_FORMAT			= "%d-%d-%dT%d:%d:%dZ";//2011-04-02T13:37:57Z
	const string ID_FORMAT				= "tag:search.twitter.com,2005:%llu";//tag:search.twitter.com,2005:50093060641656832
}

HTSearchParser::HTSearchParser()
{
   fTweets = new BList();
}

HTSearchParser::~HTSearchParser()
{
	while(!fTweets->IsEmpty())
		delete (HTTweet *)fTweets->RemoveItem((int32)0);
	delete fTweets;
}

BList*
HTSearchParser::Tweets() {
	
	return fTweets;
}

status_t
HTSearchParser::Parse(const std::string& data)
{
	//text, id, screen_name, fullname, profileimageurl, published, source
    
    status_t status = B_OK;
	
	if(data.length() < 30)
		return B_ERROR;
	
	//Devide data into nodes.
	BList* nodeList = new BList();
	string buffer("");
	size_t pos = 0;
	while(true) {
		pos = FindValue(&buffer, TwitterTags::ENTRY_TAG, data, pos);
		if(pos == string::npos)
			break;
		nodeList->AddItem(new string(buffer));
	}
	if(nodeList->IsEmpty())	
		return B_OK;
	
	//Parse nodes
	status = _ParseNodes(nodeList, fTweets);
	
	//Delete all nodes
	while(!nodeList->IsEmpty())
		delete (string *)nodeList->RemoveItem((int32)0);
	delete nodeList;
	
	return status;
}

status_t
HTSearchParser::_ParseNodes(BList* nodeList, BList* resultList)
{
	status_t status = B_OK;
	
	string* parsingNode;
	HTTweet* currentTweet;
	string buffer("");
	
	while(!nodeList->IsEmpty()) {
		status = B_OK;
		buffer = string("");
		parsingNode = (string *)nodeList->RemoveItem((int32)0);
		if(parsingNode == NULL)
			return B_BAD_INDEX;	
		currentTweet = new HTTweet();
		
		//Content
		if(FindValue(&buffer, TwitterTags::CONTENT_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setText(buffer);
			
		//Author
		if(status == B_OK && FindValue(&buffer, TwitterTags::NAME_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setScreenName(buffer);
			
		//Author
		if(status == B_OK && FindValue(&buffer, TwitterTags::PROFILEIMAGEURL_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setProfileImageUrl(buffer);
		
		//When
		if(status == B_OK && FindValue(&buffer, TwitterTags::WHEN_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setDate( _StrToTime(buffer.c_str()) );
			
		//ExternalId
		/*if(status == B_OK && FindValue(&buffer, TwitterTags::ID_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->SetExternalId( _StrToId(buffer.c_str()) );*/
		
		if(status == B_OK)
			resultList->AddItem(currentTweet);
		else
			delete currentTweet;
	}
	
	return status;
}

//Convert from Twitter format to time_t
time_t
HTSearchParser::_StrToTime(const char* str)
{	
	struct tm when;	
	int32 yyyy=0, mm=0, dd=0, hour=0, min=0, sec=0;
	
	sscanf(str, TwitterAPI::TIME_FORMAT.c_str(), &yyyy, &mm, &dd, &hour, &min, &sec);
		
	when.tm_year = yyyy-1900;	//tm_year is year since 1900
	when.tm_mon = mm-1;			//tm_mon range:		0-11
	when.tm_mday = dd;			//tm_mday range:	1-31
	when.tm_hour = hour;		//tm_hour range:	0-23
	when.tm_min = min;			//tm_min range:		0-59
	when.tm_sec = sec;			//tm_sec range:		0-59
		
	return mktime(&when);
}

//Convert from Twitter id string to uint64
/*uint64
HTSearchParser::_StrToId(const char* str)
{
	uint64 id = 0;
		
	sscanf(str, TwitterAPI::ID_FORMAT.c_str(), &id);
		
	return id;
}*/

size_t
HTSearchParser::FindValue(std::string* buffer, const char* tag, const std::string& data, size_t pos)
{
	std::string endTag(tag);
	endTag.insert(1, "/");
	
	size_t start = data.find(tag, pos);
	size_t end;
	if(start == std::string::npos)
		return std::string::npos;
		
	start += strlen(tag);
	end = data.find(endTag, start);
	
	if(end != std::string::npos)
		*buffer = data.substr(start, end-start).c_str();
	else
		*buffer = std::string("");

	return end;
}
