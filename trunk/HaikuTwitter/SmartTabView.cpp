/*
 * Copyright 2007-2009, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 *	Authors:
 *		Stefano Ceccherini (burton666@libero.it)
 */


/*!	The SmartTabView class is a BTabView descendant that hides the tab bar
	as long as there is only a single tab.
	Furthermore, it provides a tab context menu, as well as allowing you to
	close buttons with the middle mouse button.
*/


#include "SmartTabView.h"

#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include <Screen.h>
#include <ScrollView.h>
#include <Window.h>

#include <stdio.h>

#include "HTGTimeLineView.h"


const static uint32 kCloseTab = 'ClTb';
const static uint32 kMakeReplicant = 'MkRp';


SmartTabView::SmartTabView(BRect frame, const char* name, button_width width,
		uint32 resizingMode, uint32 flags)
	:
	BTabView(frame, name, width, resizingMode, flags),
	fInsets(0, 0, 0, 0),
	fScrollView(NULL)
{
	// Resize the container view to fill the complete tab view for single-tab
	// mode. Later, when more than one tab is added, we shrink the container
	// view again.
	frame.OffsetTo(B_ORIGIN);
	ContainerView()->MoveTo(frame.LeftTop());
	ContainerView()->ResizeTo(frame.Width(), frame.Height());
}


SmartTabView::~SmartTabView()
{
}

void
SmartTabView::SetInsets(float left, float top, float right, float bottom)
{
	fInsets.left = left;
	fInsets.top = top;
	fInsets.right = right;
	fInsets.bottom = bottom;
}

bool
SmartTabView::SystemIsGccFourHybrid()
{
	BEntry gccFourHybrid("/boot/system/lib/gcc2/libstdc++.r4.so");
	BEntry gccTwoHybrid("/boot/system/lib/gcc4/libsupc++.so");
	
	return gccFourHybrid.Exists();
}

void
SmartTabView::MouseDown(BPoint point)
{
	bool handled = false;

	if (CountTabs() > 1) {
		int32 tabIndex = _ClickedTabIndex(point);
		int32 buttons;
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);
		if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
			BMessage* closeMessage = new BMessage(kCloseTab);
			BMessage* replicantMessage = new BMessage(kMakeReplicant);
			closeMessage->AddInt32("index", tabIndex);
			replicantMessage->AddInt32("index", tabIndex);

			BPopUpMenu* popUpMenu = new BPopUpMenu("tab menu");
			popUpMenu->AddItem(new BMenuItem("Make replicant...", replicantMessage));
			if (tabIndex >= 2)
				popUpMenu->AddItem(new BMenuItem("Close tab", closeMessage));
			popUpMenu->SetAsyncAutoDestruct(true);
			popUpMenu->SetTargetForItems(BMessenger(this));
			popUpMenu->Go(ConvertToScreen(point+BPoint(1, 1)), true, true, true);

			handled = true;
		} else if ((buttons & B_TERTIARY_MOUSE_BUTTON) != 0) {
			RemoveAndDeleteTab(tabIndex);
			handled = true;
		}
	}

	if (!handled)
		BTabView::MouseDown(point);
}


void
SmartTabView::AttachedToWindow()
{
	BTabView::AttachedToWindow();
}


void
SmartTabView::AllAttached()
{
	BTabView::AllAttached();
}


void
SmartTabView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kCloseTab:
		{
			int32 tabIndex = 0;
			if (message->FindInt32("index", &tabIndex) == B_OK)
				RemoveAndDeleteTab(tabIndex);
			break;
		}
		case kMakeReplicant:
		{
			int32 tabIndex = 0;
			if(!SystemIsGccFourHybrid()) {
				string gcc2Error = ("I'm guessing you're not running a gcc4 hybrid system?\n\n");
				gcc2Error.append("Chances are, you are running a gcc2 hybrid version of Haiku.");
				gcc2Error.append("Replicating a timeline from HaikuTwitter (gcc4) on a gcc2 system ");
				gcc2Error.append("will probably bring your system to it's knees...\n\nIf you on the ");
				gcc2Error.append("other hand are running a pure gcc4 system, ");
				gcc2Error.append("you can go-ahead and ignore this warning:-)");
				HTGErrorHandling::displayError(gcc2Error.c_str());
			}
			if (message->FindInt32("index", &tabIndex) == B_OK) {
				HTGTimeLineView* theTimeline = dynamic_cast<HTGTimeLineView *>(ViewForTab(tabIndex));
				_OpenAsReplicant(theTimeline);
			}
			break;
		}
		default:
			BTabView::MessageReceived(message);
			break;
	}
}

void
SmartTabView::_OpenAsReplicant(HTGTimeLineView* theTimeline)
{
	string key = theTimeline->twitObj->getAccessKey();
	string secret = theTimeline->twitObj->getAccessSecret();
	int refreshTime = 5;
	const int32 TYPE = theTimeline->TYPE;
	const char* requestInfo = theTimeline->Name();
	HTGTimeLineWindow* theWindow = new HTGTimeLineWindow(Window(), key, secret, refreshTime, TYPE, requestInfo);
	theWindow->Show();
}

void
SmartTabView::Select(int32 index)
{
	BTabView::Select(index);
	BView *view = ViewForTab(index);
	if (view != NULL) {
		view->MoveTo(fInsets.LeftTop());
		view->ResizeTo(ContainerView()->Bounds().Width()
				- fInsets.left - fInsets.right,
			ContainerView()->Bounds().Height() - fInsets.top - fInsets.bottom);
	}
}


void
SmartTabView::RemoveAndDeleteTab(int32 index)
{
	// Select another tab
	if (index == Selection()) {
		if (index > 0)
			Select(0); //Modified by Martin
		else if (index < CountTabs())
			Select(index + 1);
	}
	BTab *theTab = RemoveTab(index);
	
	thread_id theThread = spawn_thread(SavedSearchDestroy, "DestroySearchThread", 10, theTab);
	resume_thread(theThread);
}

status_t
SavedSearchDestroy(void *data) 
{
	BTab *theTab = (BTab *)data;
	HTGTimeLineView *theView = (HTGTimeLineView *)theTab->View();
	
	theView->savedSearchDestoySelf();
	
	delete theTab;
	return B_OK;
}
void
SmartTabView::AddTab(BView* target, bool select, BTab* tab)
{
	if (target == NULL)
		return;

	BTabView::AddTab(target, tab);

	if (CountTabs() == 1) {
		// Call select on the tab, since
		// we're resizing the contained view
		// inside that function
		Select(0);
	} else if (CountTabs() == 2) {
		// Need to resize the view, since we're switching from "normal"
		// to tabbed mode
		ContainerView()->ResizeBy(0, -TabHeight());
		ContainerView()->MoveBy(0, TabHeight());

		// Make sure the content size stays the same, but take special care
		// of full screen mode
		BScreen screen(Window());
		if (Window()->DecoratorFrame().Height() + 2 * TabHeight()
				< screen.Frame().Height()) {
			if (Window()->Frame().bottom + TabHeight()
				> screen.Frame().bottom - 5) {
				Window()->MoveBy(0, -TabHeight());
			}

			Window()->ResizeBy(0, TabHeight());
		}

		// Adapt scroll bar if there is one
		if (fScrollView != NULL) {
			BScrollBar* bar = fScrollView->ScrollBar(B_VERTICAL);
			if (bar != NULL) {
				bar->ResizeBy(0, -1);
				bar->MoveBy(0, 1);
			}
		}
	}

	Invalidate(TabFrame(CountTabs() - 1).InsetByCopy(-2, -2));
	
	if(select)
		Select(CountTabs()-1);
	
	/*Resize window if too small*/
	int sizeDiff = (TabFrame(CountTabs() -1).right) - Window()->Frame().Width();
	if(sizeDiff > 0 && (TabFrame(CountTabs() -1).right < 914)) {
		AnimationHelper::resizeWidthAnimated(Window(), sizeDiff, 200); //No animation here
	}
	
	((HTGTimeLineView*)target)->updateTimeLine();
}

BTab*
SmartTabView::RemoveTab(int32 index)
{
	/*Resize window if it's too big*/
	int sizeDiff = (TabFrame(CountTabs() -2).right) - Window()->Frame().Width();
	if((TabFrame(CountTabs() -2).right) < 315)
			sizeDiff = 315-Window()->Frame().Width();
	if(sizeDiff < 0) {
		AnimationHelper::resizeWidthAnimated(Window(), sizeDiff, 200);
	}
	
	if (CountTabs() == 2) {
		// Hide the tab bar again by resizing the container view

		// Make sure the content size stays the same, but take special care
		// of full screen mode
		BScreen screen(Window());
		if (Window()->DecoratorFrame().Height() + 2 * TabHeight()
				< screen.Frame().Height()) {
			if (Window()->Frame().bottom
				> screen.Frame().bottom - 5 - TabHeight()) {
				Window()->MoveBy(0, TabHeight());
			}
			Window()->ResizeBy(0, -TabHeight());
		}

		// Adapt scroll bar if there is one
		if (fScrollView != NULL) {
			BScrollBar* bar = fScrollView->ScrollBar(B_VERTICAL);
			if (bar != NULL) {
				bar->ResizeBy(0, 1);
				bar->MoveBy(0, -1);
			}
		}

		ContainerView()->MoveBy(0, -TabHeight());
		ContainerView()->ResizeBy(0, TabHeight());
	}

	return BTabView::RemoveTab(index);
}


BRect
SmartTabView::DrawTabs()
{
	if (CountTabs() > 1)
		return BTabView::DrawTabs();

	return BRect();
}


/*!	If you have a vertical scroll view that overlaps with the menu bar, it will
	be resized automatically when the tabs are hidden/shown.
*/
void
SmartTabView::SetScrollView(BScrollView* scrollView)
{
	fScrollView = scrollView;
}


int32
SmartTabView::_ClickedTabIndex(const BPoint& point)
{
	if (point.y <= TabHeight()) {
		for (int32 i = 0; i < CountTabs(); i++) {
			if (TabFrame(i).Contains(point))
				return i;
		}
	}

	return -1;
}
