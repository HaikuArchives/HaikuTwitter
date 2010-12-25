/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 
 
#include "HTRestParser.h"

HTRestParser::HTRestParser()
	: BHandler()
{
	
}

HTRestParser::~HTRestParser()
{
	
}

void
HTRestParser::parseTimeline(const BString& xmlData, BMessage& reply, const BMessenger& target)
{
	
}

BString
HTRestParser::loadSampleData()
{
	BPath path;
	
	if (_getLocalPath(path) < B_OK) {
		std::cout << "Finding sample data: Failed..." << std::endl;
		return BString("");
	}
		
	BFile file(path.Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK) {
		std::cout << "Loading sample data: Failed..." << std::endl;
		return BString("");
	}
	
	char buffer[120000] = "";
	
	file.ReadAt(0, &buffer, 120000);
	
	return BString(buffer);
}

status_t
HTRestParser::_getLocalPath(BPath& path)
{
	status_t status = find_directory(B_USER_DIRECTORY, &path);
	if (status < B_OK)
		return status;
	
	path.Append("sampleData.txt");
	return B_OK;
}
