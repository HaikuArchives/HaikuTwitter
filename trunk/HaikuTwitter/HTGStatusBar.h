/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include <View.h>
#include <ScrollBar.h>

#ifndef HTG_STATUSBAR_H
#define HTG_STATUSBAR_H

class HTGStatusBar : public BView {
public:
							HTGStatusBar(BRect frame);
	virtual					~HTGStatusBar();

	virtual void			Draw(BRect updateRect);
	virtual void			AttachedToWindow();
			void			SetStatus(const char*);
private:
			const char*		status;
};
#endif
