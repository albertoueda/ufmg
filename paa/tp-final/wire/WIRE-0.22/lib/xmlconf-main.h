
#ifndef _XMLCONF_MAIN_H_INCLUDED_
#define _XMLCONF_MAIN_H_INCLUDED_

#include <config.h>

#define XMLCONF_INTERNAL_DECLARATIONS
#include "xmlconf-vars.h"
#undef XMLCONF_INTERNAL_DECLARATIONS

// Variables

bool CONF_OK = false;
xmlDocPtr			xmlconf_document;
xmlXPathContextPtr	xmlconf_context;

// Functions

void xmlconf_load();

#endif

