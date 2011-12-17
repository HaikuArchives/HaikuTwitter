/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 
 
#include <ListView.h>

#include "HTGTweetItem.h"

#ifndef HTG_LISTVIEW_H
#define HTG_LISTVIEW_H

class HTGListView : public BListView {
public:
							HTGListView(BRect frame,
										const char* name,
										list_view_type type = B_SINGLE_SELECTION_LIST,
										uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
										uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
	
	virtual	void			MouseDown(BPoint point);
	virtual	void			SelectionChanged();
};
#endif
