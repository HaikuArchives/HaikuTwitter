/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include <ListItem.h>
#include <Bitmap.h>
#include <View.h>
#include <TextView.h>
#include <Window.h>

#include "HTTweet.h"

#ifndef HTGTWEET_ITEM_H
#define HTGTWEET_ITEM_H

class HTGTweetItem : public BListItem {
public:
	HTGTweetItem(HTTweet *);
	HTTweet* getTweetPtr();
	virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
	virtual ~HTGTweetItem();
private:
	HTTweet *theTweet;
	BTextView *textView;
};
#endif
