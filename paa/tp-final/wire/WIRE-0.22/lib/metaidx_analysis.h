
#ifndef _METAIDX_ANALYSIS_H_INCLUDED_
#define _METAIDX_ANALYSIS_H_INCLUDED_

#include <config.h>

// System libraries

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>


#include <math.h>
#include <errno.h>
#include <queue>
#include <string>
#include <map>

using namespace std;

// Local libraries

#include "const.h"
#include "http_codes.h"
#include "xmlconf.h"
#include "die.h"
#include "utils.h"
#include "metaidx.h"
#include "sitelink.h"

// Macros

#define ROUND_DOUBLE(x)          ( rint(x * (double)100000000) / (double)100000000 )

// Functions

void metaidx_analysis_site_statistics( metaidx_t *metaidx );
void metaidx_analysis_doc_statistics( metaidx_t *metaidx, doc_status_t status, uint static_dynamic );

void metaidx_doc_calculate_scores( docid_t ndocs, siteid_t nsites, doc_t *doc, siterank_t siterank, docid_t queuesize );
freshness_t metaidx_doc_freshness( doc_t *doc );


#endif
