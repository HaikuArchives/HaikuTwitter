/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTStorage.h"

status_t
HTStorage::saveTweet(HTTweet *theTweet)
{
	status_t status;
	
	/*Prepare the path*/
	BPath path;
	status = getTweetPath(path);
	if (status < B_OK)
		return status;
	/*Prepare the directory*/
	BDirectory tweetsDir(path.Path());
	tweetsDir.CreateDirectory(path.Path(), NULL); //Assume this is successfull
	
	/*Prepare the file*/
	if(strlen(theTweet->getId()) > 4)
		path.Append(theTweet->getId());
	else
		return B_OK;

	/*Prepare the content*/
	string fileContent = string(theTweet->getScreenName());
	fileContent.append(":\n");
	fileContent.append(theTweet->getText());
	fileContent.append("\n\nContent from Twitter");

	/*Write the file*/
	BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	status = file.WriteAt(0, fileContent.c_str(), fileContent.length());
	if (status < B_OK)
		return status;
	
	/*Open a node for the file*/
	BNode * node = new BNode(path.Path());
	status = node->InitCheck();
	if (status < B_OK)
		return status;
		
	//Write attributes
	status = node->WriteAttr(HAIKUTWITTER_ATTR_SCREENNAME, B_STRING_TYPE, 0, theTweet->getScreenName().c_str(), theTweet->getScreenName().length());
	if(status < B_OK)
		return status;
	status = node->WriteAttr(HAIKUTWITTER_ATTR_FULLNAME, B_STRING_TYPE, 0, theTweet->getFullName().c_str(), theTweet->getFullName().length());
	if(status < B_OK)
		return status;
	status = node->WriteAttr(HAIKUTWITTER_ATTR_TEXT, B_STRING_TYPE, 0, theTweet->getText().c_str(), theTweet->getText().length());
	if(status < B_OK)
		return status;
	status = node->WriteAttr(HAIKUTWITTER_ATTR_IMAGEURL, B_STRING_TYPE, 0, theTweet->getProfileImageUrl().c_str(), theTweet->getProfileImageUrl().length());
	if(status < B_OK)
		return status;
	status = node->WriteAttr(HAIKUTWITTER_ATTR_SOURCE, B_STRING_TYPE, 0, theTweet->getSourceName().c_str(), theTweet->getSourceName().length());
	if(status < B_OK)
		return status;
	status = node->WriteAttr(HAIKUTWITTER_ATTR_ID, B_STRING_TYPE, 0, theTweet->getId(), strlen(theTweet->getId()));
	if(status < B_OK)
		return status;
	int32 creationTime = theTweet->getUnixTime();
	status = node->WriteAttr(HAIKUTWITTER_ATTR_WHEN, B_TIME_TYPE, 0, &creationTime, sizeof(int32));
	if(status < B_OK)
		return status;
	
	status = node->Sync();
	if(status < B_OK)
		return status;
	
	BNodeInfo nodeInfo(node);
	status = nodeInfo.SetType("text/x-Tweet");
	
	delete node;
	return status;
}

HTTweet*
HTStorage::loadTweet(entry_ref* ref)
{
	HTTweet* loadedTweet = new HTTweet();
	
	//Load the node
	BNode *node = new BNode(ref);
	if (node->InitCheck() != B_OK)
		return NULL;
		
	ssize_t attrSize = -1;
	char buffer[255];

	//Read ID attribute
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_ID, B_STRING_TYPE, 0, &buffer, 255);
	if(attrSize == B_ENTRY_NOT_FOUND)
		return NULL;
	buffer[attrSize] = '\0';
	loadedTweet->setId(buffer);
	
	//Read screenname attribute
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_SCREENNAME, B_STRING_TYPE, 0, &buffer, 255);
	if(attrSize == B_ENTRY_NOT_FOUND)
		return NULL;
	buffer[attrSize] = '\0';
	loadedTweet->setScreenName(*new string(buffer));
	
	//Read fullname attribute
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_FULLNAME, B_STRING_TYPE, 0, &buffer, 255);
	if(attrSize != B_ENTRY_NOT_FOUND) {
		buffer[attrSize] = '\0';
		loadedTweet->setFullName(*new string(buffer));
	}
	
	//Read text attribute
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_TEXT, B_STRING_TYPE, 0, &buffer, 255);
	if(attrSize == B_ENTRY_NOT_FOUND)
		return NULL;
	buffer[attrSize] = '\0';
	loadedTweet->setText(*new string(buffer));
	
	//Read imageurl attribute
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_IMAGEURL, B_STRING_TYPE, 0, &buffer, 255);
	if(attrSize == B_ENTRY_NOT_FOUND)
		return NULL;
	buffer[attrSize] = '\0';
	loadedTweet->setProfileImageUrl(*new string(buffer));
	
	//Read source attribute
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_SOURCE, B_STRING_TYPE, 0, &buffer, 255);
	if(attrSize == B_ENTRY_NOT_FOUND)
		return NULL;
	buffer[attrSize] = '\0';
	loadedTweet->setSourceName(*new string(buffer));
	
	//Read when attribute
	time_t creationTime = 0;
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_WHEN, B_TIME_TYPE, 0, &creationTime, sizeof(int32));
	if(attrSize == B_ENTRY_NOT_FOUND)
		return NULL;
	loadedTweet->setDate((size_t)creationTime);
	
	return loadedTweet;
}
status_t
HTStorage::getTweetPath(BPath &path)
{
	status_t status = find_directory(B_USER_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("tweets/");
	return B_OK;
}

void
HTStorage::makeMimeType(bool remakeMIMETypes)
{
	BMimeType mime;
	BMessage info;

	info.MakeEmpty();
	mime.SetTo("text/x-Tweet");
	if (mime.InitCheck() != B_OK) {
		fputs("could not init mime type.\n", stderr);
		return;
	}

	if (!mime.IsInstalled() || remakeMIMETypes) {
		// install the full mime type
		mime.Delete ();
		mime.Install();

		// Set up the list of twitter related attributes that Tracker will
		// let you display in columns for tweets.
		addAttribute(info, HAIKUTWITTER_ATTR_ID, "Id");
		addAttribute(info, HAIKUTWITTER_ATTR_FULLNAME, "Full name", B_STRING_TYPE, true);
		addAttribute(info, HAIKUTWITTER_ATTR_SCREENNAME, "Screen name");
		addAttribute(info, HAIKUTWITTER_ATTR_WHEN, "When", B_TIME_TYPE, true, false, 200);
		addAttribute(info, HAIKUTWITTER_ATTR_TEXT, "Content");
		addAttribute(info, HAIKUTWITTER_ATTR_IMAGEURL, "Profile Image URL", B_STRING_TYPE, false, false, 200);
		addAttribute(info, HAIKUTWITTER_ATTR_SOURCE, "Source");
		mime.SetAttrInfo(&info);

		mime.SetShortDescription("Tweet");
		mime.SetLongDescription("Social networking message (Twitter)");
		mime.SetPreferredApp("application/x-vnd.HaikuTwitter");
	}
	mime.Unset();
}

void
HTStorage::addAttribute(BMessage& msg, const char* name, const char* publicName,
	int32 type, bool viewable, bool editable,
	int32 width)
{
	msg.AddString("attr:name", name);
	msg.AddString("attr:public_name", publicName);
	msg.AddInt32("attr:type", type);
	msg.AddBool("attr:viewable", viewable);
	msg.AddBool("attr:editable", editable);
	msg.AddInt32("attr:width", width);
	msg.AddInt32("attr:alignment", B_ALIGN_LEFT);
}

void
HTStorage::makeIndices()
{
	const char* stringIndices[] = {
		HAIKUTWITTER_ATTR_FULLNAME, HAIKUTWITTER_ATTR_SCREENNAME,
		HAIKUTWITTER_ATTR_TEXT, HAIKUTWITTER_ATTR_SOURCE,
		NULL
	};

	// add tweet indices for all devices capable of querying

	int32 cookie = 0;
	dev_t device;
	while ((device = next_dev(&cookie)) >= B_OK) {
		fs_info info;
		if (fs_stat_dev(device, &info) < 0
			|| (info.flags & B_FS_HAS_QUERY) == 0)
			continue;

		for (int32 i = 0; stringIndices[i]; i++)
			fs_create_index(device, stringIndices[i], B_STRING_TYPE, 0);

		fs_create_index(device, HAIKUTWITTER_ATTR_WHEN, B_INT32_TYPE, 0);
	}
}
