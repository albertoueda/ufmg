
lib_LIBRARIES = libwirudiko.a
libwirudiko_a_SOURCES = \
	irudikogenericreader.cc irudikogenericreader.h \
	irudikosimplereader.cc irudikosimplereader.h \
	irudikoenglishreader.cc irudikoenglishreader.h \
	irudikoitalianreader.cc irudikoitalianreader.h \
	irudikosketchgenerator.cc irudikosketchgenerator.h \
	italianstemmer.cc italianstemmer.h \
	englishstemmer.cc englishstemmer.h \
	irudiko.h

INCLUDES = -I$(top_srcdir)/universalchardet -I.
LDADD = -ggdb -L$(top_srcdir)/universalchardet -lunivchardet
