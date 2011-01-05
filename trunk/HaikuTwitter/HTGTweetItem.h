/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
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

class HTGTweetItem : public BListItem {
public:
	HTGTweetItem(HTTweet *, bool displayFullName = true);
	HTTweet* getTweetPtr();
	virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
	virtual void Update(BView *owner, const BFont* font);
	virtual ~HTGTweetItem();
private:
	htg_color displayColors;
	HTTweet *theTweet;
	HTGTweetTextView *textView;
	HTGTimeLineView *timelineView;
	bool displayFullName;
	int calculateSize(BView *owner);
	static rgb_color compileColor(uint8 red, uint8 green, uint8 blue);
};
#endif
