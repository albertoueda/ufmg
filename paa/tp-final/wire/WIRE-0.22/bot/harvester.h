
#ifndef _HARVESTER_H_
#define _HARVESTER_H_

#include <config.h>

// System libraries

#include <getopt.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <adns.h>
#include <sys/utsname.h>


// Local libraries

#include "perfect_hash.h"
#include "xmlconf.h"
#include "const.h"
#include "utils.h"
#include "cleanup.h"
#include "http_codes.h"
#include "harvestidx.h"
#include "metaidx.h"
#include "pollvec.h"
#include "activepool.h"
#include "waitq.h"
#include "server.h"
#include "page.h"

// Functions

void harvester_prepare();
void harvester_init_servers();
void harvester_usage();

off64_t	bytes_in	= 0;
off64_t	bytes_out	= 0;

#endif
