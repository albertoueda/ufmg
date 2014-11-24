
bin_PROGRAMS = \
	wire-test-parser \
	wire-test-adns \
	wire-test-perfhash \
	wire-test-multipatt \
	wire-test-url \
	wire-test-urlidx

# Sources

wire_test_parser_SOURCES = \
	test_parser.cc test_parser.h \
	../bot/parser.cc ../bot/parser.h
wire_test_adns_SOURCES = \
	testadns.cc
wire_test_perfhash_SOURCES = \
	test_perfect_hash.cc
wire_test_multipatt_SOURCES = \
	test_multipatt.cc
wire_test_url_SOURCES = \
	test_url.cc
wire_test_urlidx_SOURCES = \
	test_urlidx.cc

# Includes

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/bot -I$(top_srcdir)/universalchardet -I$(top_srcdir)/wirudiko
LDADD = -L$(top_srcdir)/lib -L$(top_srcdir)/universalchardet -L$(top_srcdir)/wirudiko -lwire -lunivchardet -lwirudiko
