/*
 * Copyright 2010 Martin Hebnes Pedersen, martinhpedersen @ "google mail"
 * All rights reserved. Distributed under the terms of the MIT License.
 */ 

#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <Window.h>
#include <Alert.h>
#include <File.h>
#include <Message.h>

#ifndef HTG_ERROR_HANDLING
#define HTG_ERROR_HANDLING

class HTGErrorHandling {
public:
			static void 			displayError(const char *theError);
};
#endif
