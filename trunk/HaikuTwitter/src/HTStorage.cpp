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
	if(strlen(theTweet->getIdString()) > 4)
		path.Append(theTweet->getIdString());
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
	int64 id = theTweet->getId();
	status = node->WriteAttr(HAIKUTWITTER_ATTR_ID, B_INT64_TYPE, 0, &id, sizeof(int64));
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
	int64 id = 0;
	attrSize = node->ReadAttr(HAIKUTWITTER_ATTR_ID, B_INT64_TYPE, 0, &id, sizeof(int64));
	if(attrSize == B_ENTRY_NOT_FOUND)
		return NULL;
	loadedTweet->setId(id);
	
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
HTStorage::cacheBitmap(BMallocIO* bitmapData, std::string& url)
{
	status_t status;
	
	/*Prepare the path*/
	BPath path;
	status = getCachePath(path);
	if(status < B_OK)
		return status;

	/*Prepare the directory*/
	BDirectory cacheDir(path.Path());
	cacheDir.CreateDirectory(path.Path(), NULL); //Assume this is successfull
	
	/*Prepare the file*/
	stringstream out;
	out << FNVHash(url);
	path.Append(out.str().c_str());
		
	/*Write the file*/
	BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE);
	status = file.WriteAt(0, " ", 1);
	
	if(bitmapData == NULL)
		return B_OK;
	
	/*Open a node for the file*/
	BNode * node = new BNode(path.Path());
	status = node->InitCheck();
	if (status < B_OK)
		return status;
	status = node->Lock();
	if(status < B_OK)
		return status;

	//Write attributes
	status = node->WriteAttr(HAIKUTWITTER_CACHE_IMAGE, B_RAW_TYPE, 0, bitmapData->Buffer(), bitmapData->BufferLength());
	if(status < B_OK)
		return status;
	int32 size = bitmapData->BufferLength();
	status = node->WriteAttr(HAIKUTWITTER_CACHE_IMAGE_SIZE, B_INT32_TYPE, 0, &size, sizeof(int32));
	if(status < B_OK)
		return status;
		
	status = node->Sync();
	if(status < B_OK)
		return status;
		
	delete node;
	
	return status;
}

status_t
HTStorage::cleanupBitmapCache()
{
	BPath path;
	if (getCachePath(path) < B_OK)
		return B_ERROR;
		
	BDirectory cacheDir;
	if(cacheDir.SetTo(path.Path()) < B_OK)
		return B_ERROR;
	
	// Delete expired bitmap files
	//TODO: Use BeFS attr. query instead of sequential scan
	BEntry entry;
	struct stat st;
	while(cacheDir.GetNextEntry(&entry) == B_OK) {
		entry.GetStat(&st);
		
		if(st.st_ctime < (time(NULL) - CACHE_EXPIRE_TIME)) {
			char buff[1024];
			entry.GetName(buff);
			std::cout << "Deleting: " << buff << std::endl;
			entry.Remove();
		}
	}
	
	cacheDir.Unset();

	return B_OK;
}

status_t
HTStorage::findBitmap(std::string& url, BMallocIO** mallocIO)
{
	/*Prepare the path*/
	BPath path;
	if (getCachePath(path) < B_OK)
		return B_ERROR;
	stringstream out;
	out << FNVHash(url);
	path.Append(out.str().c_str());
	
	//Load the node
	BNode *node = new BNode(path.Path());
	status_t status = node->InitCheck();
	if(status == B_BUSY)
		return status;
	if(status < B_OK)
		return B_ENTRY_NOT_FOUND;
		
	ssize_t attrSize = -1;

	//Read size attribute
	int32 dataSize = -1;
	attrSize = node->ReadAttr(HAIKUTWITTER_CACHE_IMAGE_SIZE, B_INT32_TYPE, 0, &dataSize, sizeof(int32));
	if(attrSize == B_ENTRY_NOT_FOUND)
		return B_BUSY; //Someone has probably touched the file... and is downloading image
	
	char* buffer = new char[dataSize];
	
	//Read image data attribute
	attrSize = node->ReadAttr(HAIKUTWITTER_CACHE_IMAGE, B_RAW_TYPE, 0, buffer, dataSize);
	if(attrSize == B_ENTRY_NOT_FOUND) {
		return B_ENTRY_NOT_FOUND;
	}

	//File is corrupted
	if(attrSize != dataSize)
		return B_ERROR;
	
	BMallocIO* returnData = new BMallocIO();
	if(returnData->Write(buffer, dataSize) == dataSize) {
		*mallocIO = returnData;
		delete[] buffer;
		return B_OK;
	}
	else {
		delete[] buffer;
		std::cout << "HTStorage::findBitmap(): failed to write buffer to BMallocIO" << std::endl;
		return B_ERROR;
	}
}

status_t
HTStorage::getCachePath(BPath &path)
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status < B_OK)
		return status;
		
	path.Append("HaikuTwitter/");
	return B_OK;
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
		addAttribute(info, HAIKUTWITTER_ATTR_ID, "Id", B_INT64_TYPE);
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

unsigned int
HTStorage::FNVHash(const std::string& str)
{
	const unsigned int fnv_prime = 0x811C9DC5;
	unsigned int hash = 2166136261u;
	for(std::size_t i = 0; i < str.length(); i++) {
		hash ^= str[i];
		hash *= fnv_prime;
	}

	return hash;
}
