/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include <View.h>
#include <TranslationUtils.h>
#include <Roster.h>
#include <Bitmap.h>

#ifndef HTG_LOGOVIEW_H
#define HTG_LOGOVIEW_H

class HTGLogoView : public BView {
public:
							HTGLogoView();
	virtual					~HTGLogoView();

	virtual	BSize			MinSize();
	virtual	BSize			MaxSize();

	virtual void			Draw(BRect updateRect);
	virtual void			AttachedToWindow();

private:
			BBitmap*		fLogo;
};
#endif
