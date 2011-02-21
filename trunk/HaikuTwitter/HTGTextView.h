/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <Message.h>
#include <TextView.h>

#ifndef HTG_TEXTVIEW_H
#define HTG_TEXTVIEW_H

class HTGTextView : public BTextView {
public:
	HTGTextView(BRect frame, const char *name, BRect textRect, uint32 resizingMode, uint32 flags);
	HTGTextView(BRect frame, const char *name, BRect textRect, const BFont* font, const rgb_color* color, uint32 resizingMode, uint32 flags);
	
	virtual void KeyDown(const char* bytes, int32 numBytes);
};
#endif
