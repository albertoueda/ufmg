#include "config.h"
#include "const.h"
#include "xmlconf.h"
#include "xmlconf-main.h"
#include "utils.h"
#include "perfect_hash.h"
#include "urlidx.h"

void do_url_test( perfhash_t *extensions_dynamic, const char *url, char **varnames, int nvariables ) {
	char newurl[MAX_STR_LEN];
	assert( strlen(url) < (MAX_STR_LEN-3) );

	// Copy (we don't want do accidentally modify a constant)
	strcpy( newurl, url );

	// We will put this character at the end to check for errors
	int origlen = strlen(newurl);
	newurl[origlen+2] = '_';

	// Now we check
	cerr << endl;
	cerr << "<<<OLD<<< " << newurl << endl;
	cerr << "is_dynamic? " << (urlidx_is_dynamic( extensions_dynamic, newurl ) ? "yes" : "no") << endl;

	// Lower case, avoid non-alphanum characters
	char extension[MAX_STR_LEN];
	urlidx_get_lowercase_extension( newurl, extension );
	cerr << "extension: " << extension << endl;

	for( int i=0; i<nvariables; i++ ) {
		urlidx_remove_variable( newurl, varnames[i] );
	}

	urlidx_remove_sessionids_heuristic( newurl );
	urlidx_sanitize_url( newurl );

	cerr << ">>>NEW>>> " << newurl << endl;
	cerr << endl;

	// Check the character past the end
	assert( newurl[origlen+2] == '_' );
}

#define MAX_SESSIONID_VARIABLES 255

int main( int argc, char **argv ) {
	wire_start( "testurl" );

	perfhash_t extensions_dynamic;
	extensions_dynamic.check_matches = true;
    perfhash_create( &(extensions_dynamic), CONF_SEEDER_LINK_DYNAMIC_EXTENSION );

	// Create a table with variable names for better speed
	int nvars	= 0;
	char **varnames	= tokenizeToTable( CONF_SEEDER_SESSIONIDS, &(nvars) );

	
	do_url_test( &(extensions_dynamic), "a.html", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.cgi", varnames, nvars );
	do_url_test( &(extensions_dynamic), "x/z/b.html;jsessionid=0000000", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.html?check=1", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a/b/c.php?storyid=204&amp;page=1&PHPSESSID=5555", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a/b/c.php;PHPSESSID=5555", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a/b/c.php;x=y?z=a.html", varnames, nvars );
	do_url_test( &(extensions_dynamic), "/a/test.html;PHPSESSID=5555?a=9999", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.html;PHPSESSID=?a=9999", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.html?PHPSESSID=X&amp;a=9999", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.html?b=zzzz&PHPSESSID=X&amp;a=9999", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.html;a=1?b=zzzz&amp;PHPSESSID=X&amp;a=9999", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.html;a=1?b=zzzz&amp;PHPSESSID=X&", varnames, nvars );
	do_url_test( &(extensions_dynamic), "PHPSESSID=X", varnames, nvars );
	do_url_test( &(extensions_dynamic), "a.html?PHPSESSID=X&a=000&jsessionid=X", varnames, nvars );
	do_url_test( &(extensions_dynamic), "es_CL/footer_pages/site_map.jhtml;jsessionid=G2PJ3IAIZFEWHFYKJOPCFEY", varnames, nvars );
	do_url_test( &(extensions_dynamic), "tienda/?page=shop/browse&category_id=2506000&ps_session=e32379da6e396b8bc7ef216792a80d42", varnames, nvars );
	do_url_test( &(extensions_dynamic), "session_alive.php", varnames, nvars );
	do_url_test( &(extensions_dynamic), "tienda/tienda.cgi?page=shop&category_id=2506000&session-id=e32379da6e396b8bc7ef216792a80d42", varnames, nvars );
	do_url_test( &(extensions_dynamic), "editorial.asp?session-id=05d103c58feb464db7f09201c261460a", varnames, nvars );
	do_url_test( &(extensions_dynamic), "?session-id=0226b83b44e31c97cd7d12788fa7433a", varnames, nvars );
	do_url_test( &(extensions_dynamic), "forums/index.php?s=269327b02a890d20316f48eca13c4d70&amp;showtopic=5&amp;view=getnewpost", varnames, nvars );
	do_url_test( &(extensions_dynamic), "forums/catalog.php?s=123&amp;showtopic=5&amp;view=getnewpost", varnames, nvars );
	do_url_test( &(extensions_dynamic), "forum/index.php?s=29f9a30263dd7a0453bb13ff71e46590&amp;showtopic=42&amp;view=getnewpost", varnames, nvars );
	do_url_test( &(extensions_dynamic), "forum//index.php?s=29f9a30263dd7a0453bb13ff71e46590&amp;showtopic=42&amp;view=getnewpost", varnames, nvars );
	do_url_test( &(extensions_dynamic), "q_/asp/Subcategory=34/CartID=1282081312201653/Category=10/_q/subcategory.htm", varnames, nvars );
	do_url_test( &(extensions_dynamic), "forums/?sid=1c7853c4453c51fba76deeb985d4e585", varnames, nvars );
	do_url_test( &(extensions_dynamic), "links/index.php?catid=116&amp;sid=1cfa24e9e70f0abb491ca3935d623550", varnames, nvars );
	do_url_test( &(extensions_dynamic), "member.php?s=dd32a854fc1aa29111b01a28d4416386&action=getinfo&userid=2555", varnames, nvars );



	wire_stop(0);
}

void cleanup() {

}

