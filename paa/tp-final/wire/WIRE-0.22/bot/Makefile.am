
bin_PROGRAMS = \
	wire-bot-seeder \
	wire-bot-manager \
	wire-bot-reset \
	wire-bot-harvester \
	wire-bot-gatherer

dist_bin_SCRIPTS = \
	wire-bot-run

# Sources

wire_bot_reset_SOURCES = reset.cc reset.h
wire_bot_seeder_SOURCES = seeder.cc seeder.h
wire_bot_manager_SOURCES = manager.cc manager.h
wire_bot_gatherer_SOURCES = gatherer.cc gatherer.h \
	parser.cc parser.h
wire_bot_harvester_SOURCES = \
	pollvec.cc pollvec.h \
	server.cc server.h \
	activepool.cc activepool.h \
	waitq.cc waitq.h \
	page.cc page.h \
	harvester.cc harvester.h

# Includes

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/universalchardet -I$(top_srcdir)/wirudiko -I. 
LDADD = -L$(top_srcdir)/lib -L$(top_srcdir)/universalchardet -L$(top_srcdir)/wirudiko -lwire -lunivchardet -lwirudiko
