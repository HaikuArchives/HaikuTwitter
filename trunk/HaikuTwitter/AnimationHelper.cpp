/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 


#include "AnimationHelper.h"

const int32 AnimationHelper::REFRESH_RATE = 160;

void AnimationHelper::resizeWidthAnimated(BWindow* theWindow, const float pixels, const int32 ms) {
	float frames = ms*REFRESH_RATE/1000;
	BScreen currentScreen(theWindow);
	float limit = currentScreen.Frame().Width();
	
	float pixelsPerFrame = pixels/frames;
	for(int n = 0; n < frames; n++) {
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
}
