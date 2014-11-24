
#ifndef _XMLCONF_H_INCLUDED_
#define _XMLCONF_H_INCLUDED_

#include <config.h>

// System libraries

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>

using namespace std;

// Local libraries

#include "const.h"
#include "die.h"

// Filenames

#define XMLCONF_FILE_PATH		"./wire.conf:/etc/wire.conf:/usr/local/wire/wire.conf"
#define XMLCONF_ENVIRONMENT_VAR	"WIRE_CONF"

// List separators

#define XMLCONF_BASE_XPATH		"config"
#define	CONF_LIST_SEPARATOR	"\r\n,:\t "

// Vars
#define XMLCONF_EXTERNAL_DECLARATIONS
#include "xmlconf-vars.h"
#undef XMLCONF_EXTERNAL_DECLARATIONS

// Configuration readed
extern bool CONF_OK;
extern xmlDocPtr			xmlconf_document;
extern xmlXPathContextPtr	xmlconf_context;

#endif

