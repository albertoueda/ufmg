#include "config.h"
#include "const.h"
#include "xmlconf.h"
#include "xmlconf-main.h"
#include "utils.h"
#include "perfect_hash.h"
#include "urlidx.h"

void do_urlidx_path_to_absolute_test (int testnum, char *src, char *dst, char *expected){
	char newurl[MAX_STR_LEN];
	newurl[0]='/';
	newurl[1]='\0';
	urlidx_relative_path_to_absolute(src, dst, newurl);
	cerr <<"test "<<testnum<<":	";
	if (strcmp(newurl,expected)!=0){
		cerr <<" [ERROR]"<<endl;
		cerr <<"\t       source:	"<<src<<endl;
		cerr <<"\t          dst:	"<<dst<<endl;
		cerr <<"\tabsolute path:	"<<newurl<<endl;
		cerr <<"\t     expected:	"<<expected<<endl;
	}
	else{
		cerr <<"[OK]"<<endl;
	}
	cerr<<endl;
}

#define MAX_SESSIONID_VARIABLES 255

int main( int argc, char **argv ) {
	wire_start( "testurl" );

	perfhash_t extensions_dynamic;
	extensions_dynamic.check_matches = true;

	// Create a table with variable names for better speed
	int nvars	= 0;

	int cnt         = 0;
	do_urlidx_path_to_absolute_test( cnt++,"a.html", "b.html","b.html");

	do_urlidx_path_to_absolute_test(cnt++, "src/path/a.html", "dst/path/b.html","src/path/dst/path/b.html");

	do_urlidx_path_to_absolute_test(cnt++, "src/path/a.html?arg1=val1", "dst/path/b.html?arg2=val2","src/path/dst/path/b.html?arg2=val2");
	do_urlidx_path_to_absolute_test(cnt++, "src/path/a.html?arg1=val1a/val1b", "dst/path/b.html?arg2=val2","src/path/dst/path/b.html?arg2=val2");

	


	wire_stop(0);
}

void cleanup() {

}

