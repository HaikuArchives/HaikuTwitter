/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 
 
#include "HTGListView.h"

HTGListView::HTGListView(BRect frame,
							const char* name,
							list_view_type type,
							uint32 resizingMode,
							uint32 flags)
	: BListView(frame, name, type, resizingMode, flags)
{}				

void
HTGListView::MouseDown(BPoint point)
{
	BListView::MouseDown(point);
	
	int index = CurrentSelection();
	if(index < 0)
		return;
	
	HTGTweetItem* clickedItem = (HTGTweetItem*)ItemAt(index);
	if(clickedItem == NULL)
		return;
	
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	
	if ((buttons & B_PRIMARY_MOUSE_BUTTON) != 0) {
		clickedItem->LeftClicked(point, this, ItemFrame(index));
	}
	
	else if((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		clickedItem->RightClicked(point, this, ItemFrame(index));
	}
}

void
HTGListView::SelectionChanged()
{}
