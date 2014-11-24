
bin_PROGRAMS = \
	wire-info-shell \
	wire-info-extract \
	wire-info-analysis \
	wire-info-import

# Sources

wire_info_shell_SOURCES = shell.cc shell.h
wire_info_extract_SOURCES	= extract.cc extract.h
wire_info_analysis_SOURCES	= analysis.cc analysis.h lang.cc lang.h
wire_info_import_SOURCES	= import.cc import.h

# Includes

INCLUDES =  -I$(top_srcdir)/lib -I$(top_srcdir)/universalchardet -I$(top_srcdir)/wirudiko -I.
LDADD = -L$(top_srcdir)/lib -L$(top_srcdir)/universalchardet -L$(top_srcdir)/wirudiko -lwire -lunivchardet -lwirudiko
