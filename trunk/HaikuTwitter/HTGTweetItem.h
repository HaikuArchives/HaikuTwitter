/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include <ListItem.h>
#include <Bitmap.h>
#include <View.h>
#include <Window.h>
#include <Font.h>

#include "HTTweet.h"
#include "HTGTweetTextView.h"

#ifndef HTGTWEET_ITEM_H
#define HTGTWEET_ITEM_H

struct htg_color {
	rgb_color nameColor;
	rgb_color textColor;
	rgb_color timeColor;
	rgb_color sourceColor;
};

//Drawing constants
static BRect kAvatarRect(9, 5, 56, 52);
static BPoint kNamePoint(64, 0);
static BPoint kSourcePoint(0, 2);
static BPoint kTextPoint(64, 3);
static int32 kMargin = 5;

class HTGTweetItem : public BListItem {
public:
	HTGTweetItem(HTTweet* theTweet, bool displayFullName = true);
	HTGTweetItem(BMessage* archive);
	
	HTTweet* getTweetPtr() const;
	BRect AvatarBounds(BRect frame);
	BRect NameBounds(BRect frame, BView* view, float lineHeight, const char* name);
	BRect TimeBounds(BRect frame, BView* view, float lineHeight, const char* time);
	BRect SourceBounds(BRect frame, BView* view, float lineHeight, const char* source);
	virtual void ClearView();
	virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
	virtual void Update(BView *owner, const BFont* font);
	virtual BArchivable* Instantiate(BMessage* archive);
	virtual status_t Archive(BMessage* archive, bool deep = true) const;
	
	static int sortByDateFunc(const void*, const void*);
	
	virtual ~HTGTweetItem();
private:
	htg_color displayColors;
	HTTweet *theTweet;
	HTGTweetTextView *textView;
	HTGTimeLineView *timelineView;
	bool displayFullName;
	bool isReplicant;
	int calculateSize(BRect frame, BView *owner);
	static rgb_color compileColor(uint8 red, uint8 green, uint8 blue);
};
#endif
