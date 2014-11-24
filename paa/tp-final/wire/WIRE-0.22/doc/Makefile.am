
XSLTPROC=xsltproc
XSLTPROC_FLAGS=@XSLTPROC_FLAGS@
XHTML_STYLESHEET=http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl

htmldir = $(prefix)/share/doc/@PACKAGE@

dist_html_DATA = index.xml index.html sample.conf

if have_xsltproc

index.html: ${top_srcdir}/doc/index.xml
	$(XSLTPROC) $(XSLTPROC_FLAGS) $(XHTML_STYLESHEET) $<  > $@

else

index.html:
	touch $@

endif

install:
	cp sample.conf *.html $(prefix)/share/@PACKAGE@

clean:
	rm -f *.html

SUBDIRS=icn
