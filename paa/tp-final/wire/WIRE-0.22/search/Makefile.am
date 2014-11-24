
bin_PROGRAMS = \
	wire-search-text-split \
	wire-search-feeder \
	wire-search-indexer 

# Sources


wire_search_feeder_SOURCES = feeder.cc feeder.h

wire_search_indexer_SOURCES = indexer.cc feeder.h indexer.h

wire_search_text_split_SOURCES	= split.cc split.h

# Includes

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/universalchardet -I$(top_srcdir)/wirudiko
LDADD = -L$(top_srcdir)/lib -L$(top_srcdir)/universalchardet -L$(top_srcdir)/wirudiko -lwire -lunivchardet -lwirudiko
