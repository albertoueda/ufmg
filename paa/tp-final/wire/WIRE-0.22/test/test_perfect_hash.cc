
#include "perfect_hash.h"
#include "perfect_hash.cc"
#include <stdio.h>
#include <assert.h>
#define MAX_STR_LEN 1024

int main( int argc, char **argv ) {
	perfhash_t perfhash, discard_content, keep_tag, extensions_log, extensions_ignore, keep_attribute;


	// Can be set to false for better speed, but less reliability
	// while discarding tags

	/* We don't want to loose text accidentaly */
	cerr << "Initializing mappings: discard_content, ";
	bool double_check_kept_markup = true;
	discard_content.check_matches = true;
	perfhash_create( &(discard_content), "style script object embed" );

	cerr << "keep tags, ";
	keep_tag.check_matches = double_check_kept_markup;
	perfhash_create( &(keep_tag), "p table tr td th i em strong b a h1 h2 h3 h4 h5 h6 blockquote meta title head body frame link img area embed" );


	cerr << "keep attributes, ";
	keep_attribute.check_matches = double_check_kept_markup;
	perfhash_create( &(keep_attribute), "http-equiv src alt description keywords name content rel href title" );

	cerr << "extensions to ignore, ";
	extensions_ignore.check_matches = true;
	perfhash_create( &(extensions_ignore), "changes zip lha lhz arj rar tar gz tgz shar clt crt com ocx" );

	cerr << "extensions to log, ";
	extensions_log.check_matches = true;
	perfhash_create( &(extensions_log), "gif tif tiff jpg jpeg jpe png cgm bmp mpeg mov avi pcf wav wmv wma mid mp3 au doc ps pdf rtf ppt xml swf fla deb rpm xls" );
	//DD
	assert( perfhash_check( &(extensions_log), "gif" ) );
	assert( perfhash_check( &(extensions_log), "rtf" ) );
	assert( perfhash_check( &(extensions_log), "swf" ) );

	perfhash.check_matches = true;

    printf( "Testing\n" );
	perfhash_create( &(perfhash), "zip lha lhz arj rar tar gz shar gif jpg jpeg png cgm bmp ppt doc rtf xls pdf ps mpeg mov avi exe com ocx dll wav mid mp3 clt crt" );

	perfhash_dump( &(perfhash) );
	assert(  perfhash_check( &(perfhash), "avi" )  );
	assert( perfhash_check( &(perfhash), "exe" ) );
	assert( perfhash_check( &(perfhash), "com" ) );
	assert( perfhash_check( &(perfhash), "ocx" ) );
	assert( perfhash_check( &(perfhash), "dll" ) );
	assert( !perfhash_check( &(perfhash), "html" ) );
	assert( !perfhash_check( &(perfhash), "htm" ) );
	assert( !perfhash_check( &(perfhash), "asp" ) );
	assert( !perfhash_check( &(perfhash), "cgi" ) );
	assert( !perfhash_check( &(perfhash), "pl" ) );
	assert( !perfhash_check( &(perfhash), "yogi" ) );
	assert( !perfhash_check( &(perfhash), "cabul" ) );
	assert( !perfhash_check( &(perfhash), "toro" ) );
	assert( !perfhash_check( &(perfhash), "ligre" ) );
	assert( !perfhash_check( &(perfhash), "asdj" ) );
	perfhash_destroy( &(perfhash) );
	
	perfhash_create( &(perfhash), "p table tr td th i em strong b a h1 h2 h3 h4 h5 h6 blockquote meta title head body frame" );
	perfhash_dump( &(perfhash) );

	assert( perfhash_check( &(perfhash), "p" ) );
	assert( perfhash_check( &(perfhash), "table" ) );
	assert( perfhash_check( &(perfhash), "tr" ) );
	assert( perfhash_check( &(perfhash), "td" ) );
	assert( perfhash_check( &(perfhash), "th" ) );
	assert( perfhash_check( &(perfhash), "blockquote" ) );
	assert( perfhash_check( &(perfhash), "title" ) );
	assert( !perfhash_check( &(perfhash), "html" ) );
	assert( !perfhash_check( &(perfhash), "htm" ) );
	assert( !perfhash_check( &(perfhash), "asp" ) );
	assert( !perfhash_check( &(perfhash), "frames" ) );
	assert( !perfhash_check( &(perfhash), "script" ) );
	assert( !perfhash_check( &(perfhash), "yogi" ) );
	assert( !perfhash_check( &(perfhash), "cabul" ) );
	assert( !perfhash_check( &(perfhash), "toro" ) );
	assert( !perfhash_check( &(perfhash), "ligre" ) );
	assert( !perfhash_check( &(perfhash), "asdj" ) );
	perfhash_destroy( &(perfhash) );

	perfhash_create( &(perfhash), "gif tif tiff jpg jpeg jpe png cgm bmp mpeg mov avi pcf wav wmv wma mid mp3 au doc ps pdf rtf ppt xml swf fla deb rpm xls" );
	perfhash_dump( &(perfhash) );

	assert( perfhash_check( &(perfhash), "gif" ) );
	assert( perfhash_check( &(perfhash), "pdf" ) );
	assert( perfhash_check( &(perfhash), "xls" ) );
	assert( ! perfhash_check( &(perfhash), "xxx" ) );

	perfhash_destroy( &(perfhash) );
	


}
