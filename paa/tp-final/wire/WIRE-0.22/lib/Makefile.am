
lib_LIBRARIES = libwire.a
libwire_a_SOURCES = \
	const.h \
	http_codes.h \
	http_charset.cc http_charset.h \
	utils.cc utils.h\
	cleanup.cc cleanup.h die.h\
	xmlconf.cc xmlconf-main.h xmlconf-vars.h xmlconf.h \
	metaidx.cc metaidx.h \
	metaidx_analysis.cc metaidx_analysis.h \
	storage.cc storage.h \
	urlidx.cc urlidx.h \
	linkidx.cc linkidx.h \
	sitelink.cc sitelink.h \
	perfect_hash.cc perfect_hash.h \
	harvestidx.cc harvestidx.h \
	string_list.cc string_list.h \
	int_stack.cc int_stack.h \
	cgic.h cgic.c \
	connection.h connection.c \
	md5.h md5.cc

INCLUDES = -I$(top_srcdir)/universalchardet -I$(top_srcdir)/wirudiko -I.
LDADD = -L$(top_srcdir)/universalchardet -lunivchardet -lwirudiko
