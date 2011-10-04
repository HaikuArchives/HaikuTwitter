/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 
 
#include "HTAccountCredentials.h"

namespace CredentialTags {
	const char* ID_TAG 				= "<id>";
	const char* REALNAME_TAG		= "<name>";
	const char* SCREENNAME_TAG		= "<screen_name>";
	const char* LOCATION_TAG		= "<location>";
	const char* DESCRIPTION_TAG 	= "<description>";
	const char* PROFILEIMAGEURL_TAG	= "<profile_image_url>";
	const char* COUNTFOLLOWERS_TAG	= "<followers_count>";
}

extern "C" size_t decode_html_entities_utf8(char *dest, const char *src);
using namespace std;

HTAccountCredentials::HTAccountCredentials(twitCurl* twitObj, BHandler* msgHandler)
{
	this->twitObj = twitObj;
	this->msgHandler = msgHandler;
	
	verified = false;
}

HTAccountCredentials::~HTAccountCredentials()
{
	delete twitObj;
}

bool
HTAccountCredentials::Verified() const
{
	return verified;
}

status_t
HTAccountCredentials::Fetch(int32 id)
{
	if(id < 0)
		return FetchSelf();
	else
		return B_ERROR; //Fetching information on other users is not implemented
}

status_t
HTAccountCredentials::FetchSelf()
{	
	int32 errorCount = 0;
	while(!twitObj->verifyCredentials()) {
		if(errorCount < kMaxRetries)
			errorCount++;
		else
			return B_ERROR;
	}
		
	std::string reply(" ");
	twitObj->getLastWebResponse(reply);
	
	std::cout << reply << std::endl;

	status_t status = B_OK;
	string buffer("");
	
	//Id
	if(FindValue(&buffer, CredentialTags::ID_TAG, reply, 0) == string::npos)
		status = B_ERROR;
	else
		id = atoi(buffer.c_str());
	
	//Screen name
	if(FindValue(&buffer, CredentialTags::SCREENNAME_TAG, reply, 0) == string::npos)
		status = B_ERROR;
	else
		screenName = buffer.c_str();
		
	//realName
	if(FindValue(&buffer, CredentialTags::REALNAME_TAG, reply, 0) == string::npos)
		status = B_ERROR;
	else
		realName = buffer.c_str();
		
	//location
	if(FindValue(&buffer, CredentialTags::LOCATION_TAG, reply, 0) == string::npos)
		status = B_ERROR;
	else
		location = buffer.c_str();
		
	//description
	if(FindValue(&buffer, CredentialTags::DESCRIPTION_TAG, reply, 0) == string::npos)
		status = B_ERROR;
	else
		description = buffer.c_str();
		
	//profileImageUrl
	if(FindValue(&buffer, CredentialTags::PROFILEIMAGEURL_TAG, reply, 0) == string::npos)
		status = B_ERROR;
	else
		profileImageUrl = buffer.c_str();
	
	//count followers
	if(FindValue(&buffer, CredentialTags::COUNTFOLLOWERS_TAG, reply, 0) == string::npos)
		status = B_ERROR;
	else
		countFollowers = atoi(buffer.c_str());
	
	verified = (screenName.Length() > 1);
	
	return B_OK;
}

size_t
HTAccountCredentials::FindValue(std::string* buffer, const char* tag, const std::string& data, size_t pos, bool decodeHtml)
{
	std::string ourBuffer;
	
	std::string endTag(tag);
	endTag.insert(1, "/");
	
	size_t start = data.find(tag, pos);
	size_t end;
	if(start == std::string::npos)
		return std::string::npos;
		
	start += strlen(tag);
	end = data.find(endTag, start);
	
	if(end != std::string::npos)
		ourBuffer = data.substr(start, end-start).c_str();
		
	if(decodeHtml) {
		char decodeBuffer[1024];
		decode_html_entities_utf8(&decodeBuffer[0], ourBuffer.c_str());
		*buffer = std::string(&decodeBuffer[0]);
	}
	else
		*buffer = std::string(ourBuffer);

	return end;
}

const char*
HTAccountCredentials::ScreenName() const
{
	return screenName.String();
}

const char*
HTAccountCredentials::RealName() const
{
	return realName.String();
}

const char*
HTAccountCredentials::Location() const
{
	return location.String();
}

const char*
HTAccountCredentials::Description() const
{
	return description.String();
}

const char*
HTAccountCredentials::ProfileImageUrl() const
{
	return profileImageUrl.String();	
}

int32
HTAccountCredentials::CountFollowers() const
{
	return countFollowers;
}

int32
HTAccountCredentials::Id() const
{
	return id;
}
