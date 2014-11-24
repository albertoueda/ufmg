
#ifdef XMLCONF_EXTERNAL_DECLARATIONS
extern char CONF_COLLECTION_BASE[MAX_STR_LEN];
extern unsigned long  CONF_COLLECTION_MAXDOC;
extern unsigned long  CONF_COLLECTION_MAXSITE;
extern uint  CONF_MANAGER_BATCH_COUNT;
extern unsigned long  CONF_MANAGER_BATCH_SIZE;
extern uint  CONF_MANAGER_BATCH_SAMESITE;
extern uint	CONF_MANAGER_MAXDEPTH_STATIC;
extern uint	CONF_MANAGER_MAXDEPTH_DYNAMIC;
extern uint CONF_MAX_ERRORS_DIFFERENT_BATCH;
extern uint CONF_MANAGER_MAX_ERRORS_SAME_BATCH;
extern uint		CONF_MANAGER_SCORE_PAGERANK_WEIGHT;
extern double		CONF_MANAGER_SCORE_PAGERANK_DAMPENING;
extern double	CONF_MANAGER_SCORE_PAGERANK_MAX_ERROR;
extern uint		CONF_MANAGER_SCORE_WLSCORE_WEIGHT;
extern uint	CONF_MANAGER_SCORE_HITS_HUB_WEIGHT;
extern uint CONF_MANAGER_SCORE_HITS_AUTHORITY_WEIGHT;
extern double	CONF_MANAGER_SCORE_HITS_MAX_ERROR;
extern uint CONF_MANAGER_SCORE_SITERANK_WEIGHT;
extern double	CONF_MANAGER_SCORE_SITERANK_MAX_ERROR;
extern uint CONF_MANAGER_SCORE_STATIC_WEIGHT;
extern uint CONF_MANAGER_SCORE_DEPTH_WEIGHT;
extern uint	CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT;
extern uint CONF_MANAGER_SCORE_RANDOM_WEIGHT;
extern uint CONF_MANAGER_SCORE_AGE_EXPONENT;
extern uint CONF_MANAGER_SCORE_BOOSTFRONTPAGE;
extern ulong	CONF_MANAGER_MINPERIOD_UNSUCCESSFUL;
extern ulong	CONF_MANAGER_MINPERIOD_SUCCESSFUL_BUT_NOT_OK;
extern ulong	CONF_MANAGER_MINPERIOD_SUCCESSFUL_AND_OK;
extern ulong CONF_MANAGER_MINPERIOD_ROBOTS_TXT_VALID;
extern ulong CONF_MANAGER_MINPERIOD_ROBOTS_TXT_NOT_VALID;
extern ulong CONF_MANAGER_MINPERIOD_ROBOTS_RDF_VALID;
extern ulong CONF_MANAGER_MINPERIOD_ROBOTS_RDF_NOT_VALID;
extern char CONF_GATHERER_DESTINATION[MAX_STR_LEN];
extern char CONF_GATHERER_DISCARD_CONTENT[MAX_STR_LEN];
extern char CONF_GATHERER_KEEP_TAG[MAX_STR_LEN];
extern char CONF_GATHERER_KEEP_ATTRIBUTE[MAX_STR_LEN];
extern char CONF_GATHERER_LINK_ATTRIBUTE[MAX_STR_LEN];
extern uint  CONF_GATHERER_CHANGETODYNAMIC;
extern uint CONF_GATHERER_REMOVESOURCE;
extern ulong CONF_GATHERER_MAXSTOREDSIZE;
extern uint  CONF_GATHERER_SAVEHASHONLY;
extern uint  CONF_GATHERER_CONVERTTOUTF8;
extern char CONF_GATHERER_DEFAULTCHARSET[MAX_STR_LEN];
extern uint CONF_GATHERER_USESKETCHES;
extern char CONF_SEEDER_SESSIONIDS[MAX_STR_LEN];
extern char CONF_SEEDER_REJECTPATTERNS[MAX_STR_LEN];
extern uint CONF_SEEDER_ADDHOMEPAGE;
extern uint CONF_SEEDER_ADD_ROBOTSTXT;
extern uint CONF_SEEDER_ADD_ROBOTSRDF;
extern ulong CONF_SEEDER_MAX_URLS_PER_SITE;
extern char CONF_SEEDER_LINK_ACCEPT_PROTOCOL[MAX_STR_LEN];
extern char CONF_SEEDER_LINK_ACCEPT_DOMAIN_SUFFIXES[MAX_STR_LEN];
extern char	CONF_SEEDER_LINK_EXTENSIONS_IGNORE[MAX_STR_LEN];
extern char	CONF_SEEDER_LINK_EXTENSIONS_LOG[MAX_STR_LEN];
extern char	CONF_SEEDER_LINK_EXTENSIONS_STAT[MAX_STR_LEN];
extern char CONF_SEEDER_LINK_DYNAMIC_EXTENSION[MAX_STR_LEN];
extern char  CONF_HARVESTER_BLOCKED_IP[MAX_STR_LEN];
extern char  CONF_HARVESTER_USER_AGENT_COMMENT[MAX_STR_LEN];
extern uint  CONF_HARVESTER_NTHREADS_START;
extern uint  CONF_HARVESTER_NTHREADS_SOFTMIN;
extern uint  CONF_HARVESTER_NTHREADS_SOFTMINTIME;
extern uint  CONF_HARVESTER_NTHREADS_HARDMIN;
extern uint  CONF_HARVESTER_TIMEOUT_BYTES_PER_SECOND;
extern uint  CONF_CONNECTION_TIMEOUT;
extern uint  CONF_HARVESTER_TIMEOUT_READWRITE;
extern uint  CONF_HARVESTER_TIMEOUT_POLL;
extern ulong  CONF_MAX_FILE_SIZE;
extern uint  CONF_HARVESTER_DNSMAX;
extern uint  CONF_HARVESTER_DNSTIMEOUT;
extern uint  CONF_HARVESTER_WAIT_COUNTBIG;
extern uint  CONF_HARVESTER_WAIT_BIG;
extern uint  CONF_HARVESTER_WAIT_NORMAL;
extern char CONF_HARVESTER_ACCEPTMIME[MAX_STR_LEN];
extern uint  CONF_HARVESTER_LOGLEVEL;
extern uint  CONF_HARVESTER_DNSEXPIRE;
extern char	CONF_HARVESTER_RESOLVCONF[MAX_STR_LEN];
extern char CONF_ANALYSIS_LANG_BASEPATH[MAX_STR_LEN];
extern char CONF_ANALYSIS_LANG_STOPWORDS[MAX_STR_LEN];
extern uint CONF_ANALYSIS_LANG_SAMPLE_EVERY;
extern uint CONF_ANALYSIS_LANG_SAVE_TEXT;
extern uint CONF_ANALYSIS_LANG_MIN_WORDS;
extern uint CONF_ANALYSIS_LANG_MIN_STOPWORDS;
extern uint CONF_ANALYSIS_LANG_MIN_DIFFERENCE;
extern uint CONF_ANALYSIS_DOC_SAMPLE_EVERY;
extern uint CONF_INDEX_DESCRIPTIONSIZE;
extern uint	CONF_INDEX_MINWORDLENGTH;
extern uint	CONF_INDEX_MAXWORDLENGTH;
extern char	CONF_INDEX_STOPWORDSFILE[MAX_STR_LEN];

#define _CONF_EVALUATE  {\
xmlconf_read_string( "collection/base",	CONF_COLLECTION_BASE );\
xmlconf_read_ulong( "collection/maxdoc", &CONF_COLLECTION_MAXDOC );\
xmlconf_read_ulong( "collection/maxsite", &CONF_COLLECTION_MAXSITE );\
xmlconf_read_uint( "manager/batch/count", &CONF_MANAGER_BATCH_COUNT );\
xmlconf_read_ulong( "manager/batch/size", &CONF_MANAGER_BATCH_SIZE );\
xmlconf_read_uint( "manager/batch/samesite", &CONF_MANAGER_BATCH_SAMESITE );\
xmlconf_read_uint( "manager/maxdepth/static", &CONF_MANAGER_MAXDEPTH_STATIC ); \
xmlconf_read_uint( "manager/maxdepth/dynamic", &CONF_MANAGER_MAXDEPTH_DYNAMIC ); \
xmlconf_read_uint( "manager/max-errors/different-batch", &CONF_MAX_ERRORS_DIFFERENT_BATCH ); \
xmlconf_read_uint( "manager/max-errors/same-batch", &CONF_MANAGER_MAX_ERRORS_SAME_BATCH ); \
xmlconf_read_uint( "manager/score/pagerank/weight", &CONF_MANAGER_SCORE_PAGERANK_WEIGHT ); \
xmlconf_read_double( "manager/score/pagerank/dampening", &CONF_MANAGER_SCORE_PAGERANK_DAMPENING ); \
xmlconf_read_double( "manager/score/pagerank/max-error", &CONF_MANAGER_SCORE_PAGERANK_MAX_ERROR ); \
xmlconf_read_uint( "manager/score/wlscore/weight", &CONF_MANAGER_SCORE_WLSCORE_WEIGHT ); \
xmlconf_read_uint( "manager/score/hits/hub/weight", &CONF_MANAGER_SCORE_HITS_HUB_WEIGHT ); \
xmlconf_read_uint( "manager/score/hits/authority/weight", &CONF_MANAGER_SCORE_HITS_AUTHORITY_WEIGHT ); \
xmlconf_read_double( "manager/score/hits/max-error", &CONF_MANAGER_SCORE_HITS_MAX_ERROR ); \
xmlconf_read_uint( "manager/score/siterank/weight", &CONF_MANAGER_SCORE_SITERANK_WEIGHT ); \
xmlconf_read_double( "manager/score/siterank/max-error", &CONF_MANAGER_SCORE_SITERANK_MAX_ERROR ); \
xmlconf_read_uint( "manager/score/static/weight", &CONF_MANAGER_SCORE_STATIC_WEIGHT ); \
xmlconf_read_uint( "manager/score/depth/weight", &CONF_MANAGER_SCORE_DEPTH_WEIGHT ); \
xmlconf_read_uint( "manager/score/queue-size/weight", &CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT ); \
xmlconf_read_uint( "manager/score/random/weight", &CONF_MANAGER_SCORE_RANDOM_WEIGHT ); \
xmlconf_read_uint( "manager/score/age/exponent", &CONF_MANAGER_SCORE_AGE_EXPONENT ); \
xmlconf_read_uint( "manager/score/boostfrontpage", &CONF_MANAGER_SCORE_BOOSTFRONTPAGE ); \
xmlconf_read_ulong( "manager/minperiod/unsuccessful", &CONF_MANAGER_MINPERIOD_UNSUCCESSFUL ); \
xmlconf_read_ulong( "manager/minperiod/successful-but-not-ok", &CONF_MANAGER_MINPERIOD_SUCCESSFUL_BUT_NOT_OK ); \
xmlconf_read_ulong( "manager/minperiod/successful-and-ok", &CONF_MANAGER_MINPERIOD_SUCCESSFUL_AND_OK ); \
xmlconf_read_ulong( "manager/minperiod/robots-txt/when-valid", &CONF_MANAGER_MINPERIOD_ROBOTS_TXT_VALID ); \
xmlconf_read_ulong( "manager/minperiod/robots-txt/when-not-valid", &CONF_MANAGER_MINPERIOD_ROBOTS_TXT_NOT_VALID ); \
xmlconf_read_ulong( "manager/minperiod/robots-rdf/when-valid", &CONF_MANAGER_MINPERIOD_ROBOTS_RDF_VALID ); \
xmlconf_read_ulong( "manager/minperiod/robots-rdf/when-not-valid", &CONF_MANAGER_MINPERIOD_ROBOTS_RDF_NOT_VALID ); \
xmlconf_read_string( "harvester/blocked-ip",   CONF_HARVESTER_BLOCKED_IP );\
xmlconf_read_string( "harvester/user-agent-comment",   CONF_HARVESTER_USER_AGENT_COMMENT );\
xmlconf_read_uint( "harvester/nthreads/start",   &CONF_HARVESTER_NTHREADS_START );\
xmlconf_read_uint( "harvester/nthreads/softmin",   &CONF_HARVESTER_NTHREADS_SOFTMIN );\
xmlconf_read_uint( "harvester/nthreads/softmintime",   &CONF_HARVESTER_NTHREADS_SOFTMINTIME );\
xmlconf_read_uint( "harvester/nthreads/hardmin",   &CONF_HARVESTER_NTHREADS_HARDMIN );\
xmlconf_read_uint( "harvester/timeout/bytes-per-second",   &CONF_HARVESTER_TIMEOUT_BYTES_PER_SECOND );\
xmlconf_read_uint( "harvester/timeout/connection", &CONF_CONNECTION_TIMEOUT );\
xmlconf_read_uint( "harvester/timeout/readwrite", &CONF_HARVESTER_TIMEOUT_READWRITE );\
xmlconf_read_uint( "harvester/timeout/poll", &CONF_HARVESTER_TIMEOUT_POLL );\
xmlconf_read_ulong( "harvester/maxfilesize", &CONF_MAX_FILE_SIZE );\
xmlconf_read_uint( "harvester/dnsmax", &CONF_HARVESTER_DNSMAX );\
xmlconf_read_uint( "harvester/dnstimeout", &CONF_HARVESTER_DNSTIMEOUT );\
xmlconf_read_uint( "harvester/wait/countbig", &CONF_HARVESTER_WAIT_COUNTBIG ) ;\
xmlconf_read_uint( "harvester/wait/big", &CONF_HARVESTER_WAIT_BIG ) ;\
xmlconf_read_uint( "harvester/wait/normal", &CONF_HARVESTER_WAIT_NORMAL ) ;\
xmlconf_read_string( "harvester/acceptmime",	CONF_HARVESTER_ACCEPTMIME );\
xmlconf_read_uint( "harvester/loglevel", &CONF_HARVESTER_LOGLEVEL ); \
xmlconf_read_uint( "harvester/dnsexpire", &CONF_HARVESTER_DNSEXPIRE );\
xmlconf_read_string( "harvester/resolvconf", CONF_HARVESTER_RESOLVCONF ); \
xmlconf_read_string( "gatherer/discard/content",	CONF_GATHERER_DISCARD_CONTENT );\
xmlconf_read_string( "gatherer/keep/tag",CONF_GATHERER_KEEP_TAG );\
xmlconf_read_string( "gatherer/keep/attribute",	CONF_GATHERER_KEEP_ATTRIBUTE );\
xmlconf_read_string( "gatherer/link/attribute",	CONF_GATHERER_LINK_ATTRIBUTE );\
xmlconf_read_uint( "gatherer/changetodynamic", &CONF_GATHERER_CHANGETODYNAMIC ); \
xmlconf_read_uint( "gatherer/removesource", &CONF_GATHERER_REMOVESOURCE ); \
xmlconf_read_ulong( "gatherer/maxstoredsize", &CONF_GATHERER_MAXSTOREDSIZE ); \
xmlconf_read_uint_default( "gatherer/save-hash-only", &CONF_GATHERER_SAVEHASHONLY, 0 ); \
xmlconf_read_uint_default( "gatherer/convert-to-utf8", &CONF_GATHERER_CONVERTTOUTF8, 1 ); \
xmlconf_read_uint_default( "gatherer/use-sketches", &CONF_GATHERER_USESKETCHES, 0); \
xmlconf_read_string( "gatherer/defaultcharset", CONF_GATHERER_DEFAULTCHARSET ); \
xmlconf_read_string( "seeder/sessionids",	CONF_SEEDER_SESSIONIDS );\
xmlconf_read_string( "seeder/reject-patterns",	CONF_SEEDER_REJECTPATTERNS );\
xmlconf_read_uint( "seeder/add-root",	&CONF_SEEDER_ADDHOMEPAGE );\
xmlconf_read_uint( "seeder/add-robots-txt",	&CONF_SEEDER_ADD_ROBOTSTXT );\
xmlconf_read_uint( "seeder/add-robots-rdf",	&CONF_SEEDER_ADD_ROBOTSRDF );\
xmlconf_read_ulong( "seeder/max-urls-per-site",	&CONF_SEEDER_MAX_URLS_PER_SITE );\
xmlconf_read_string( "seeder/accept/protocol",	CONF_SEEDER_LINK_ACCEPT_PROTOCOL );\
xmlconf_read_string( "seeder/accept/domain-suffixes",	CONF_SEEDER_LINK_ACCEPT_DOMAIN_SUFFIXES );\
xmlconf_read_multi_string( "seeder/extensions/ignore/group",	CONF_SEEDER_LINK_EXTENSIONS_IGNORE );\
xmlconf_read_multi_string( "seeder/extensions/log/group",	CONF_SEEDER_LINK_EXTENSIONS_LOG );\
xmlconf_read_multi_string( "seeder/extensions/stat/group",	CONF_SEEDER_LINK_EXTENSIONS_STAT );\
xmlconf_read_multi_string( "seeder/extensions/download/dynamic/group", CONF_SEEDER_LINK_DYNAMIC_EXTENSION );\
xmlconf_read_string( "analysis/lang/basepath", CONF_ANALYSIS_LANG_BASEPATH ); \
xmlconf_read_string( "analysis/lang/stopwords", CONF_ANALYSIS_LANG_STOPWORDS ); \
xmlconf_read_uint( "analysis/lang/sample-every", &CONF_ANALYSIS_LANG_SAMPLE_EVERY ); \
xmlconf_read_uint( "analysis/lang/save-text", &CONF_ANALYSIS_LANG_SAVE_TEXT ); \
xmlconf_read_uint( "analysis/lang/min-words", &CONF_ANALYSIS_LANG_MIN_WORDS ); \
xmlconf_read_uint( "analysis/lang/min-stopwords", &CONF_ANALYSIS_LANG_MIN_STOPWORDS ); \
xmlconf_read_uint( "analysis/lang/min-difference", &CONF_ANALYSIS_LANG_MIN_DIFFERENCE ); \
xmlconf_read_uint( "analysis/doc/sample-every", &CONF_ANALYSIS_DOC_SAMPLE_EVERY );\
xmlconf_read_uint( "index/descriptionsize", &CONF_INDEX_DESCRIPTIONSIZE );\
xmlconf_read_string( "index/stopwordsfile", CONF_INDEX_STOPWORDSFILE ); \
xmlconf_read_uint( "index/minwordlength", &CONF_INDEX_MINWORDLENGTH );\
xmlconf_read_uint( "index/maxwordlength", &CONF_INDEX_MAXWORDLENGTH );\
	}

#endif

#ifdef XMLCONF_INTERNAL_DECLARATIONS
char	CONF_COLLECTION_BASE[MAX_STR_LEN];
unsigned long  CONF_COLLECTION_MAXDOC;
unsigned long  CONF_COLLECTION_MAXSITE;
uint	CONF_MANAGER_BATCH_COUNT;
ulong	CONF_MANAGER_BATCH_SIZE;
uint	CONF_MANAGER_BATCH_SAMESITE;
uint	CONF_MANAGER_MAXDEPTH_DYNAMIC;
uint	CONF_MANAGER_MAXDEPTH_STATIC;
uint	CONF_MAX_ERRORS_DIFFERENT_BATCH;
uint	CONF_MANAGER_MAX_ERRORS_SAME_BATCH;
uint	CONF_MANAGER_SCORE_PAGERANK_WEIGHT;
double	CONF_MANAGER_SCORE_PAGERANK_DAMPENING;
double	CONF_MANAGER_SCORE_PAGERANK_MAX_ERROR;
uint	CONF_MANAGER_SCORE_WLSCORE_WEIGHT;
uint	CONF_MANAGER_SCORE_HITS_HUB_WEIGHT;
uint	CONF_MANAGER_SCORE_HITS_AUTHORITY_WEIGHT;
double	CONF_MANAGER_SCORE_HITS_MAX_ERROR;
uint	CONF_MANAGER_SCORE_SITERANK_WEIGHT;
double	CONF_MANAGER_SCORE_SITERANK_MAX_ERROR;
uint	CONF_MANAGER_SCORE_STATIC_WEIGHT;
uint	CONF_MANAGER_SCORE_DEPTH_WEIGHT;
uint	CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT;
uint	CONF_MANAGER_SCORE_RANDOM_WEIGHT;
uint	CONF_MANAGER_SCORE_AGE_EXPONENT;
uint	CONF_MANAGER_SCORE_BOOSTFRONTPAGE;
ulong	CONF_MANAGER_MINPERIOD_UNSUCCESSFUL;
ulong	CONF_MANAGER_MINPERIOD_SUCCESSFUL_BUT_NOT_OK;
ulong	CONF_MANAGER_MINPERIOD_SUCCESSFUL_AND_OK;
ulong	CONF_MANAGER_MINPERIOD_ROBOTS_TXT_VALID;
ulong	CONF_MANAGER_MINPERIOD_ROBOTS_TXT_NOT_VALID;
ulong	CONF_MANAGER_MINPERIOD_ROBOTS_RDF_VALID;
ulong	CONF_MANAGER_MINPERIOD_ROBOTS_RDF_NOT_VALID;
char	CONF_GATHERER_DISCARD_CONTENT[MAX_STR_LEN];
char	CONF_GATHERER_KEEP_TAG[MAX_STR_LEN];
char	CONF_GATHERER_KEEP_ATTRIBUTE[MAX_STR_LEN];
char	CONF_GATHERER_LINK_ATTRIBUTE[MAX_STR_LEN];
uint	CONF_GATHERER_CHANGETODYNAMIC;
uint	CONF_GATHERER_REMOVESOURCE;
ulong	CONF_GATHERER_MAXSTOREDSIZE;
uint	CONF_GATHERER_SAVEHASHONLY;
uint	CONF_GATHERER_CONVERTTOUTF8;
uint    CONF_GATHERER_USESKETCHES;
char	CONF_GATHERER_DEFAULTCHARSET[MAX_STR_LEN];
char	CONF_SEEDER_SESSIONIDS[MAX_STR_LEN];
char	CONF_SEEDER_REJECTPATTERNS[MAX_STR_LEN];
uint	CONF_SEEDER_ADDHOMEPAGE;
uint	CONF_SEEDER_ADD_ROBOTSTXT;
uint	CONF_SEEDER_ADD_ROBOTSRDF;
ulong	CONF_SEEDER_MAX_URLS_PER_SITE;
char	CONF_SEEDER_LINK_ACCEPT_PROTOCOL[MAX_STR_LEN];
char	CONF_SEEDER_LINK_ACCEPT_DOMAIN_SUFFIXES[MAX_STR_LEN];
char	CONF_SEEDER_LINK_EXTENSIONS_IGNORE[MAX_STR_LEN];
char	CONF_SEEDER_LINK_EXTENSIONS_LOG[MAX_STR_LEN];
char	CONF_SEEDER_LINK_EXTENSIONS_STAT[MAX_STR_LEN];
char	CONF_SEEDER_LINK_DYNAMIC_EXTENSION[MAX_STR_LEN];
char    CONF_HARVESTER_BLOCKED_IP[MAX_STR_LEN];
char    CONF_HARVESTER_USER_AGENT_COMMENT[MAX_STR_LEN];
uint	CONF_HARVESTER_NTHREADS_START;
uint	CONF_HARVESTER_NTHREADS_SOFTMIN;
uint	CONF_HARVESTER_NTHREADS_SOFTMINTIME;
uint	CONF_HARVESTER_NTHREADS_HARDMIN;
uint	CONF_HARVESTER_TIMEOUT_BYTES_PER_SECOND;
uint	CONF_CONNECTION_TIMEOUT;
uint    CONF_HARVESTER_TIMEOUT_READWRITE;
uint    CONF_HARVESTER_TIMEOUT_POLL;
ulong	CONF_MAX_FILE_SIZE;
uint	CONF_HARVESTER_DNSMAX;
uint	CONF_HARVESTER_DNSTIMEOUT;
uint	CONF_HARVESTER_WAIT_COUNTBIG;
uint	CONF_HARVESTER_WAIT_BIG;
uint	CONF_HARVESTER_WAIT_NORMAL;
char	CONF_HARVESTER_ACCEPTMIME[MAX_STR_LEN];
uint	CONF_HARVESTER_LOGLEVEL;
uint	CONF_HARVESTER_DNSEXPIRE;
char	CONF_HARVESTER_RESOLVCONF[MAX_STR_LEN];
char    CONF_ANALYSIS_LANG_BASEPATH[MAX_STR_LEN];
char    CONF_ANALYSIS_LANG_STOPWORDS[MAX_STR_LEN];
uint    CONF_ANALYSIS_LANG_SAMPLE_EVERY;
uint    CONF_ANALYSIS_LANG_SAVE_TEXT;
uint	CONF_ANALYSIS_LANG_MIN_WORDS;
uint	CONF_ANALYSIS_LANG_MIN_STOPWORDS;
uint	CONF_ANALYSIS_LANG_MIN_DIFFERENCE;
uint	CONF_ANALYSIS_DOC_SAMPLE_EVERY;
uint	CONF_INDEX_DESCRIPTIONSIZE;
uint	CONF_INDEX_MINWORDLENGTH;
uint	CONF_INDEX_MAXWORDLENGTH;
char	CONF_INDEX_STOPWORDSFILE[MAX_STR_LEN];

#endif


