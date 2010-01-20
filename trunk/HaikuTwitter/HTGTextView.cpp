/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */

 
#include "HTGTextView.h"

HTGTextView::HTGTextView(BRect frame, const char *name, BRect textRect, uint32 resizingMode, uint32 flags) : BTextView(frame, name, textRect, resizingMode, flags) {
		
}
	
HTGTextView::HTGTextView(BRect frame, const char *name, BRect textRect, const BFont* font, const rgb_color* color, uint32 resizingMode, uint32 flags) : BTextView(frame, name, textRect, font, color, resizingMode, flags) {
		
}

void HTGTextView::KeyDown(const char* bytes, int32 numBytes) {
	BTextView::KeyDown(bytes, numBytes);
	MessageReceived(new BMessage('UPDT'));
}
