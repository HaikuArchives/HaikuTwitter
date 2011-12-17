/*
 * Copyright 2010-2011 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include "HTGErrorHandling.h"

void
HTGErrorHandling::displayError(const char* theError)
{
	BAlert *alert = new BAlert("error", theError, "OK");
	alert->Go();
}
