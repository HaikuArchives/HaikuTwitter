/*
 * Copyright 2008, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Modified by Martin Hebnes Pedersen, 2011
 * Distributed under the terms of the MIT license.
 */
#ifndef HYPER_TEXT_ACTIONS_H
#define HYPER_TEXT_ACTIONS_H

#include <String.h>
#include <Message.h>

#include "HTGTweetTextView.h"


class URLAction : public HyperTextAction {
public:
								URLAction(const BString& url);
	virtual						~URLAction();

	virtual	void				Clicked(HTGTweetTextView* view, BPoint where,
									BMessage* message);
private:
			BString				fURL;
};

class MessageAction : public HyperTextAction {
public:
								MessageAction(const BMessage& msg);
	virtual						~MessageAction();
	
	virtual	void				Clicked(HTGTweetTextView* view, BPoint where,
									BMessage* message);
private:
			BMessage*			fMsg;
};


class OpenFileAction : public HyperTextAction {
public:
								OpenFileAction(const BString& file);
	virtual						~OpenFileAction();

	virtual	void				Clicked(HTGTweetTextView* view, BPoint where,
									BMessage* message);
private:
			BString				fFile;
};


#endif	// HYPER_TEXT_ACTIONS_H
