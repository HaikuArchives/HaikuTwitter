/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTGLogoView.h"

HTGLogoView::HTGLogoView()
	: BView("logo", B_WILL_DRAW)
{
	fLogo = BTranslationUtils::GetBitmap(B_VECTOR_ICON_TYPE, "twitter_icon");
}


HTGLogoView::~HTGLogoView()
{
	delete fLogo;
}

void HTGLogoView::AttachedToWindow()
{
   if ( Parent() )
      SetViewColor(Parent()->ViewColor());
   BView::AttachedToWindow();
}

BSize
HTGLogoView::MinSize()
{
	if (fLogo == NULL)
		return BSize(200, 200);

	return BSize(fLogo->Bounds().Width(), fLogo->Bounds().Height());
}


BSize
HTGLogoView::MaxSize()
{
	if (fLogo == NULL)
		return BSize(300, 300);

	return BSize(B_SIZE_UNLIMITED, fLogo->Bounds().Height());
}


void
HTGLogoView::Draw(BRect updateRect)
{
	if (fLogo != NULL) {
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmap(fLogo,
			BPoint((Bounds().Width() - fLogo->Bounds().Width()) / 2, 0));
		SetDrawingMode(B_OP_OVER);
	}
}
