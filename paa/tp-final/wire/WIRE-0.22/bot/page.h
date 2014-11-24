
#ifndef _PAGE_H_INCLUDED_
#define _PAGE_H_INCLUDED_

#include <config.h>

// Local libraries

#include "server.h"

// Functions

void page_create_request( server_t *server, page_t *page, char *useragent, uint max_request_size, char *request, uint *request_size );
void page_save( page_t *page, int http_code );

#endif
