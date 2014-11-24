
#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

#include <config.h>

// System libraries

#include <regex.h>
#include <map>
#include <iostream>
#include <string>
#include <syslog.h>
#include <cleanup.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

// Local libraries

#include "xmlconf.h"

// Constants

#define MAX_TOKENS	255

// External functions

using namespace std;

extern void xmlconf_load();
extern void cleanup_enable();
extern void cleanup();
extern void cleanup_disable();

// Functions

void get_extension( char *examined, char *extension );
void unescape_url( char *url );
void tokenizeToMap( char *str, map<string,bool> &theMap );
char **tokenizeToTable( char *str, int *ntokens_out );
void tokenizeToRegex(char *str, regex_t *regex );
bool has_nonprintable_characters( char *str );
void timestamp( char *str );
void createdir( const char *dirname );
void replace_all( char *str, const char *pat, const char *rep );
void str_tolower( char *str );
size_t read_file( char *filename, char *buffer );
float timer_delta( struct timeval *time1, struct timeval *time2 );

void wire_start( const char *program_name );
void wire_stop( int code );

#endif
