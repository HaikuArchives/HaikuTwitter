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

	id = atoi(_FindValue(CredentialTags::ID_TAG, reply));
	screenName = _FindValue(CredentialTags::SCREENNAME_TAG, reply);
	realName = _FindValue(CredentialTags::REALNAME_TAG, reply);
	location = _FindValue(CredentialTags::LOCATION_TAG, reply);
	description = _FindValue(CredentialTags::DESCRIPTION_TAG, reply);
	profileImageUrl = _FindValue(CredentialTags::PROFILEIMAGEURL_TAG, reply);
	countFollowers = atoi(_FindValue(CredentialTags::COUNTFOLLOWERS_TAG, reply));
	
	verified = (screenName.Length() > 1);
	
	return B_OK;
}

const char*
HTAccountCredentials::_FindValue(const char* tag, const std::string& str)
{
	std::string endTag(tag);
	endTag.insert(1, "/");
	
	size_t start = str.find(tag);
	size_t end;
	if(start == std::string::npos)
		return "";
		
	start += strlen(tag);
	end = str.find(endTag, start);
	
	return str.substr(start, end-start).c_str();
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
