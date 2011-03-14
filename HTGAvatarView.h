/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include <View.h>
#include <TranslationUtils.h>
#include <Roster.h>
#include <Bitmap.h>

#include "HTGTweetItem.h"
#include "HTTweet.h"

#ifndef HTG_AVATARVIEW_H
#define HTG_AVATARVIEW_H

class HTGAvatarView : public BView {
public:
							HTGAvatarView();
							HTGAvatarView(twitCurl* twitObj, BRect frame, uint32 resizingMode = B_FOLLOW_LEFT_RIGHT);
	virtual					~HTGAvatarView();

			void			SetAvatarTweet(HTTweet*);

	virtual void			Draw(BRect updateRect);
	virtual void			MessageReceived(BMessage *msg);
	virtual void			AttachedToWindow();

private:
			BRect			_AvatarBounds();
			BButton*		fPostButton;
			BStringView*	fCounterView;
			
			void			_UpdateCounter();
			void			postTweet();
			std::string		urlEncode(const char*);
			
			
			HTGTextView*	fMessage;
			HTTweet*		avatarTweet;
			bool			displayAvatar;
			twitCurl*		twitObj;
};
#endif
