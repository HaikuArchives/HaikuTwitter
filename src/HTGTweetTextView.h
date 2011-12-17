/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <Message.h>
#include <TextView.h>
#include <PopUpMenu.h>
#include <List.h>
#include <string>
#include <Looper.h>
#include <Entry.h>
#include <Roster.h>

#include <iostream>

#include "HTGMainWindow.h"
#include "HTGTweetMenuItem.h"

#include <Cursor.h>
#include <Region.h>
#include <Font.h>
#include <Window.h>
#include <ObjectList.h>
#include "HyperTextActions.h"

#ifndef HTG_TWEETTEXTVIEW_H
#define HTG_TWEETTEXTVIEW_H

const int32 GO_TO_URL = 'GURL';
const int32 GO_RETWEET = 'GRTW';
const int32 GO_QUOTE = 'GQUO';
const int32 GO_REPLY = 'GRPL';
const struct rgb_color kLinkBlue = {80, 80, 200, 255 };

class HTGTweetTextView;

class HyperTextAction {
public:
								HyperTextAction();
	virtual						~HyperTextAction();

	virtual	void				MouseOver(HTGTweetTextView* view, BPoint where,
									BMessage* message);
	virtual	void				Clicked(HTGTweetTextView* view, BPoint where,
									BMessage* message);
};

class HTGTweetTextView : public BTextView {
public:
								HTGTweetTextView(BRect frame, const char *name,
									BRect textRect, uint32 resizingMode, uint32 flags);
								HTGTweetTextView(BRect frame, const char *name,
									BRect textRect, const BFont* font, const rgb_color* color,
									uint32 resizingMode, uint32 flags);
	virtual	void				MessageReceived(BMessage *msg);
			void				setTweetId(const char* tweetId);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
			void				AddHyperTextAction(int32 startOffset,
									int32 endOffset, HyperTextAction* action);

	virtual	bool				CanEndLine(int32 offset);
			void				MakeHyperText();

			BList*				getUrls();
			BList*				urls;
		
								~HTGTweetTextView();
		
private:
			std::string			tweetId;
			
			struct				ActionInfo;
			class				ActionInfoList;
			HyperTextAction*	_ActionAt(const BPoint& where) const;
			ActionInfoList*		fActionInfos;
	
			thread_id			currentThread;

			void				openUrl(const char *);
			bool				isValidScreenNameChar(const char &);
			void				sendRetweetMsgToParent();
			void				sendQuoteMsgToParent();
			void				sendReplyMsgToParent();
			
			BList*				getScreenNames();
			BList*				getTags();
};

class HTGTweetTextView::ActionInfoList
	: public BObjectList<HTGTweetTextView::ActionInfo> {
public:
	ActionInfoList(int32 itemsPerBlock = 20, bool owning = false)
		: BObjectList<HTGTweetTextView::ActionInfo>(itemsPerBlock, owning)
	{
	}
};
#endif
