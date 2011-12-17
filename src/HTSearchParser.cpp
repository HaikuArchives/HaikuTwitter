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
namespace TwitterSearchTags {
	const char* ENTRY_TAG 			= "<entry>";
	const char* WHEN_TAG			= "<published>";
	const char* CONTENT_TAG			= "<title>";
	const char* NAME_TAG		 	= "<name>";
	const char* WHERE_TAG			= "<uri>";
	const char* SOURCE_TAG			= "<twitter:source>";
	const char* ID_TAG				= "<id>";
	const char* PROFILEIMAGEURL_TAG	= "<link>";
}

//Define some constants for using the twitter api
namespace TwitterSearchAPI {	
	const string TIME_FORMAT			= "%d-%d-%dT%d:%d:%dZ";//2011-04-02T13:37:57Z
	const string ID_FORMAT				= "tag:search.twitter.com,2005:%llu";//tag:search.twitter.com,2005:50093060641656832
}

HTSearchParser::HTSearchParser()
	: HTTimelineParser()
{
   fTweets = new BList();
}

HTSearchParser::~HTSearchParser()
{ }

status_t
HTSearchParser::Parse(const std::string& data)
{    
    status_t status = B_OK;
	
	#ifdef DEBUG_ENABLED
	std::cout << data << std::endl;
	#endif
	
	if(data.length() < 30) {
		std::cout << "Parse error: data was empty" << std::endl;
		return B_ERROR;	
	}
	
	//Devide data into nodes.
	BList* nodeList = new BList();
	string buffer("");
	size_t pos = 0;
	while(true) {
		pos = FindValue(&buffer, TwitterSearchTags::ENTRY_TAG, data, pos, false); //We don't want to bother with decoding html right now
		if(pos == string::npos)
			break;
		nodeList->AddItem(new string(buffer));
	}
	if(nodeList->IsEmpty()) {	
		return B_OK;
	}
		
	//Parse nodes
	status = _ParseNodes(nodeList, fTweets);
	
	//Delete all nodes (data has been copied to HTTweets)
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
	
	for(int32 i = 0; i < nodeList->CountItems(); i++) {
		status = B_OK;
		buffer = string("");
		parsingNode = (string *)nodeList->ItemAt(i);
		if(parsingNode == NULL)
			return B_BAD_INDEX;	
		currentTweet = new HTTweet();
								
		//Content
		if(FindValue(&buffer, TwitterSearchTags::CONTENT_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setText(buffer.c_str());
			
		//Screen name & real name
		if(status == B_OK && FindValue(&buffer, TwitterSearchTags::NAME_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else {
			//Parse (Format: "martinhpedersen (Martin H. Pedersen)"
			size_t screenNameEndIndex = buffer.find(" (");
			size_t fullNameEndIndex = buffer.find(")");
			
			if(fullNameEndIndex != std::string::npos) {
				currentTweet->setFullName(buffer.substr(screenNameEndIndex+2, fullNameEndIndex-2-screenNameEndIndex).c_str());
				currentTweet->setScreenName(buffer.substr(0, screenNameEndIndex).c_str());
			}
			else {
				status = B_ERROR;
				std::cout << "Error parsing for full/screen name" << std::endl;
			}
		}
			
		//Profile image url
		if(status == B_OK && _FindProfileImage(&buffer, *parsingNode) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setProfileImageUrl(buffer.c_str());
		
		//When
		if(status == B_OK && FindValue(&buffer, TwitterSearchTags::WHEN_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setDate( _StrToTime(buffer.c_str()) );
			
		//Source
		if(status == B_OK && FindValue(&buffer, TwitterSearchTags::SOURCE_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else {
			// Parse the data for Application name
            size_t pos = buffer.find(">", 0); //<a href="http://www.tweetdeck.com/" rel="nofollow">TweetDeck</a>
			if(pos != std::string::npos) {
				size_t start = pos;
				size_t end = pos;
				while(end < buffer.length() && buffer[end] != '<') {
					end++;
				}
				string sourceName = buffer.substr(start+1, end-start-1);
				currentTweet->setSourceName(sourceName.c_str());
			}
			else
				currentTweet->setSourceName(buffer.c_str());
		}
			
		//ExternalId
		if(status == B_OK && FindValue(&buffer, TwitterSearchTags::ID_TAG, *parsingNode, 0) == string::npos)
			status = B_ERROR;
		else
			currentTweet->setId( _StrToId(buffer.c_str()) );
		
		if(status == B_OK)
			resultList->AddItem(currentTweet);
		else
			delete currentTweet;			
	}
	
	return status;
}

size_t
HTSearchParser::_FindProfileImage(std::string* buffer, const std::string& data)
{
	const char* tag = "<link type=\"image/png\" href=\"";
	std::string endTag("\" rel=\"image\"/>");
	
	size_t start = data.find(tag, 0);
	size_t end;
	if(start == std::string::npos)
		return std::string::npos;
		
	start += strlen(tag);
	end = data.find(endTag, start);
	
	if(end != std::string::npos)
		*buffer = data.substr(start, end-start).c_str();

	return end;
}

//Convert from Twitter format to time_t
time_t
HTSearchParser::_StrToTime(const char* str)
{	
	struct tm when;	
	int32 yyyy=0, mm=0, dd=0, hour=0, min=0, sec=0;
	
	sscanf(str, TwitterSearchAPI::TIME_FORMAT.c_str(), &yyyy, &mm, &dd, &hour, &min, &sec);
		
	when.tm_year = yyyy-1900;	//tm_year is year since 1900
	when.tm_mon = mm-1;			//tm_mon range:		0-11
	when.tm_mday = dd;			//tm_mday range:	1-31
	when.tm_hour = hour;		//tm_hour range:	0-23
	when.tm_min = min;			//tm_min range:		0-59
	when.tm_sec = sec;			//tm_sec range:		0-59
		
	return mktime(&when);
}

//Convert from Twitter id string to uint64
uint64
HTSearchParser::_StrToId(const char* str)
{
	uint64 id = 0;
		
	sscanf(str, TwitterSearchAPI::ID_FORMAT.c_str(), &id);
		
	return id;
}
