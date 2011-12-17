/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "Window.h"
#include "Screen.h"
#include "View.h"

#ifndef ANIMATIONHELPER_H
#define ANIMATIONHELPER_H

class AnimationHelper {
public:
	static	const int32		REFRESH_RATE;
		
	static	void			resizeWidthAnimated(BWindow*, const float pixels, const int32 ms);
};
#endif
