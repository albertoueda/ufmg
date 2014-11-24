#ifndef _CLEANUP_H_INCLUDED_
#define _CLEANUP_H_INCLUDED_

#include <config.h>

// System libraries

#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>

using namespace std;

// Local libraries
#include "const.h"

// External functions

extern void cleanup();

// Functions

void cleanup_enable();
void cleanup_disable();
void die(const char *reason);

// Low level
void enable_signal_handler( void (*sighandler)(int));
void disable_signal_handler();
void signal_handler( int signum );

#endif
