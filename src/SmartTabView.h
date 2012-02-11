/*
 * Copyright 2007-2009, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 *	Authors:
 *		Stefano Ceccherini (burton666@libero.it)
 *	Modified:
 *		Martin Hebnes Pedersen
 */
#ifndef SMART_TAB_VIEW_H
#define SMART_TAB_VIEW_H


#include <TabView.h>
#include "AnimationHelper.h"

class HTGTimeLineWindow;
class HTGTimeLineView;
class HTGErrorHandling;
class BPopUpMenu;
class BScrollView;

status_t SavedSearchDestroy(void *data); //Thread function to destroy search on twitter

class SmartTabView : public BTabView {
public:
								SmartTabView(BRect frame, const char* name,
									button_width width = B_WIDTH_AS_USUAL,
									uint32 resizingMode = B_FOLLOW_ALL,
									uint32 flags = B_FULL_UPDATE_ON_RESIZE
										| B_WILL_DRAW | B_NAVIGABLE_JUMP
										| B_FRAME_EVENTS | B_NAVIGABLE);
								SmartTabView(const char* name,
									button_width width = B_WIDTH_AS_USUAL,
									uint32 flags = B_FULL_UPDATE_ON_RESIZE
										| B_WILL_DRAW | B_NAVIGABLE_JUMP
										| B_FRAME_EVENTS | B_NAVIGABLE
										| B_SUPPORTS_LAYOUT);
	virtual						~SmartTabView();

			void				SetInsets(float left, float top, float right,
									float bottom);

	virtual void				MouseDown(BPoint where);

	virtual void				AttachedToWindow();
	virtual void				AllAttached();

	virtual void				MessageReceived(BMessage* message);

	virtual	void				Select(int32 tab);

	virtual	void				RemoveAndDeleteTab(int32 index);

	virtual	void				AddTab(BView* target, bool select = false, BTab* tab = NULL);
	virtual BTab*				RemoveTab(int32 index);

	virtual BRect				DrawTabs();

			void				SetScrollView(BScrollView* scrollView);
			
private:
			int32				_ClickedTabIndex(const BPoint& point);
			void				_OpenAsReplicant(HTGTimeLineView* theTimeline);
			bool				SystemIsGccFourHybrid();

private:
			BRect				fInsets;
			BScrollView*		fScrollView;
};

#endif // SMART_TAB_VIEW_H