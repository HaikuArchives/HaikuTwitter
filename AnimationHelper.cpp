/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "AnimationHelper.h"

const int32 AnimationHelper::REFRESH_RATE = 120;

void
AnimationHelper::resizeWidthAnimated(BWindow* theWindow, const float pixels, const int32 ms)
{
	BScreen currentScreen(theWindow);
	float limit = currentScreen.Frame().Width();
	if(ms < 1 ) {
		theWindow->ResizeBy(pixels, 0);
		if(theWindow->Frame().right > limit)
			theWindow->MoveBy(-pixels, 0);
		theWindow->UpdateIfNeeded();
		return;
	}
	
	int frames = ms*REFRESH_RATE/1000;
	float pixelsPerFrame = (pixels/(float)frames);
	float newWidth = theWindow->Frame().Width() + pixels;
	
	for(int i = 0; i < frames; i++) {
		bool wasLocked = theWindow->IsLocked();
		if(wasLocked)
			theWindow->UnlockLooper(); //For smoother animations
		theWindow->ResizeBy(pixelsPerFrame, 0);
		if(theWindow->Frame().right > limit)
			theWindow->MoveBy(-pixelsPerFrame, 0);
		theWindow->UpdateIfNeeded();
		if(wasLocked)
			theWindow->LockLooper();
		usleep((ms/frames)*1000);
	}
	
	if(newWidth != theWindow->Frame().Width())
		theWindow->ResizeTo(newWidth, theWindow->Frame().Height());
}
