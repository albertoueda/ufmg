#include "parser.h"

perfhash_t keep_tag;       // HTML elements keepd
perfhash_t keep_attribute; // HTML attrs keepd
perfhash_t discard_content;    // Elements whose content is discarded
perfhash_t extensions_ignore;  // Extensions to ignore
perfhash_t extensions_log;     // Extensions to log, but not to download
perfhash_t extensions_stat;    // Extensions to count, but neither log nor download

map <string,bool> is_link;            // Elements that define a link
map <string,bool> is_link_with_content; // Attributes that define a link with the caption being the content
map <string,string> link_attribute;     // Attributes that define a link
map <string,string> link_caption;       // Attributes that define the caption for a link
map <string,docid_t> count_extensions_stat;   // For gathering statistics

//
// Name: parser_process
//
// Descriptions:
//   Parses a document
//
// Input:
//   doc - the document to be parsed
//   inbuf - text of the document
// 
// Return:
//   new content_length of the document
//

off64_t parser_process( doc_t *doc, char *inbuf, char *outbuf) {
	assert( doc != NULL );
	assert( doc->docid != 0 );
	assert( doc->raw_content_length > 0 );

	assert( inbuf != NULL );
	assert( outbuf != NULL );

	off64_t content_length = 0;

	// Select the parser
	switch( doc->mime_type ) {

		case MIME_TEXT_HTML:
			// Html documents
			return parser_process_html( doc, inbuf, outbuf );
			break;

		case MIME_ROBOTS_TXT:
			// Robots exclusion protocol
			return parser_process_robotstxt( doc, inbuf, outbuf );
			break;

		case MIME_ROBOTS_RDF:
			// Robots site summary
			return parser_process_robotsrdf( doc, inbuf, outbuf );
			break;

		case MIME_REDIRECT:
			inbuf[doc->raw_content_length] = '\0';
			parser_analyze_link( doc, inbuf, "", (double)0, (em_tag_t)0 );
			return 0;
			break;

		case MIME_TEXT_PLAIN:

			// Include a null at the end
			content_length = doc->raw_content_length;

			// Make room at the end for a null character
			// because we might need to append one
			if( content_length >= MAX_DOC_LEN ) {
				content_length = MAX_DOC_LEN - 1;
			}

			// Copy buffer
			memcpy( outbuf, inbuf, content_length );

			// See if the buffer includes a null at the end
			// (inside the buffer, not outside)
			if( outbuf[content_length-1] != '\0' ) {
				outbuf[content_length] = '\0';
				content_length ++;
			}
			return content_length;
			break;

		default:

			// Copy buffer, unchanged
			memcpy( outbuf, inbuf, doc->raw_content_length );
			return doc->raw_content_length;
			break;
	}
}

//
// Name: parser_process_html
//
// Description:
//   Parse an html document
//
// Input:
//   doc - the document to be parsed
//   inbuf - text of the document
// 
// Return:
//   new content_length of the document
//

// First: define some macros
#define IS_START() (inbuf[inpos]=='<')
#define IS_END() (inbuf[inpos]=='>')
#define IS_SPACE() (inbuf[inpos]==' '||inbuf[inpos]=='	'||inbuf[inpos]=='\n'||inbuf[inpos]=='\r')
#define IS_EOF()	(inpos > size)
#define IS_QUOTE() (inbuf[inpos]=='\'' || inbuf[inpos]=='\"')
#define IS_EQ() (inbuf[inpos]=='=')
#define IS_SLASH() (inbuf[inpos] == '/')
#define IS_NEWLINE() (inbuf[inpos] == '\r' || inbuf[inpos] == '\n')

off64_t parser_process_html( doc_t *doc, char *inbuf, char *outbuf) {
	// Posible links
	char href[MAX_STR_LEN];
	char caption[MAX_STR_LEN];

	// Clean strings
	href[0]						= '\0';
	caption[0]					= '\0';

	// Position and status of the parser
	off64_t inpos					= 0;
	off64_t outpos				= 0;
	off64_t start_text			= 0;
	status_t status				= STATUS_NORMAL;
	bool waiting_link_content	= false;
	bool follow_links			= true;
	off64_t size					= doc->raw_content_length;
	int caption_position		= 0;

	// Verify
	assert( size > 0 );
	assert( doc != NULL );
	assert( inbuf != NULL );
	assert( outbuf != NULL );

	int_stack_t *tag_stack = (int_stack_t *)malloc(sizeof(int_stack_t));
	int_stack_init(tag_stack);

	// Clean the input buffer, leave only what is going to be saved
	for( off64_t in=0; in<size; in++ ) {
		if( inbuf[in] == '\f' || inbuf[in] == '\n' || inbuf[in] == '\r' || inbuf[in] == '\t' || inbuf[in] == '\v' ) {
			inbuf[in] = ' ';
		}
	}

	// Main loop
	while(!IS_EOF()) {

		// ------- EVENTS-ORIENTED PARSER  ----------
		// This is a pseudo-sax parser
		// Events parser
		event_t event = EVENT_UNDEF;
		tag_t	tag;
		tag.attnum = 0;

		// Generate an EVENT_EOF
		if( IS_EOF() ) {
			event = EVENT_EOF;

		// Generate an EVENT_START_TAG or EVENT_END_TAG
		} else if( IS_START() ) {
			event = EVENT_START_TAG;

			// Read the tag name
			inpos++;
			int i = 0;

			while( !IS_SPACE() && !IS_END() && !IS_EOF() ) {
				if( IS_SLASH() ) {
					if( i==0 ) {
						event = EVENT_END_TAG;
						inpos++;
					} else {
						event = EVENT_EMPTY_TAG;
						while( !IS_END() && !IS_EOF() ) {
							inpos++;
						}
					}
				} else  {
					if( i < MAX_STR_LEN - 1 ) {
						tag.name[i++] = tolower(inbuf[inpos]);
					}
					inpos++;
				}
			} // end while reading tag name
			tag.name[i] = '\0';

			if( IS_SPACE() ) {

				// Read until too many attributes
				while( !IS_END() && !IS_EOF() && ( tag.attnum < MAX_HTML_ATTRIBUTES ) ) {

					// Advance to next non-space
					while(!IS_EOF() && !IS_SLASH() && IS_SPACE() && !IS_END()) {
						inpos++;
					}

					if( IS_EOF() ) {
						event = EVENT_EOF;

					} else if( IS_SLASH() ) {

						// Check if empty tag
						event = EVENT_EMPTY_TAG;
						while(!IS_END() && !IS_EOF()) {
							inpos++;
						}
						if( IS_EOF() ) {
							event = EVENT_EOF;
						}
					} else if( !IS_END() ) {
						// Start copying attribute name
						int i = 0;
						while(!IS_EQ() && !IS_END() && !IS_EOF()) {
							if( i < MAX_STR_LEN-1 ) {
								tag.attributes[tag.attnum].name[i++] = tolower(inbuf[inpos]);
							}
							inpos++;
						}
						tag.attributes[tag.attnum].name[i] = '\0';

						if( IS_EQ() ) {
							// Start copying value
							char quoted = '\0';
							while( IS_EQ() && !IS_QUOTE() && !IS_EOF() && !IS_END() ) {
									inpos++;
							}
							if( IS_QUOTE() ) {
								quoted = inbuf[inpos];
								inpos++;
							}

							int i = 0;
							while( !IS_EOF() &&
								((quoted  && (!(inbuf[inpos] == quoted)) && !IS_END()) ||
								 (!quoted && !IS_SPACE() && !IS_END())) )  {
								if( i < MAX_STR_LEN-1 ) {
									tag.attributes[tag.attnum].value[i++] = inbuf[inpos];	
								}
								inpos++;
									
							}
							tag.attributes[tag.attnum].value[i] = '\0';

							if( IS_QUOTE() ) {
								inpos++;
							}

						} else if( IS_EOF() ) {
							event = EVENT_EOF;	
						}
						tag.attnum++;
					}
				}
				

			} // end while (what goes after tag name)

			if( IS_END() ) {
				inpos++;
			}

		// Generate an EVENT_TEXT
		} else if( !IS_EOF() ) {
			while( IS_SPACE() && !IS_EOF() ) {
				inpos++;
			}
			if( !IS_START() && !IS_EOF() ) {
				event = EVENT_TEXT;
				// In a normal SAX parser, we should copy the text to a 
				// temporary buffer. For efficiency reasons, we don't

			} else if( IS_EOF() ) {
				event = EVENT_EOF;
			} else {
				event = EVENT_UNDEF;
			}
		}

		// ------ EVENT HANDLER ----------
		// Switch depending on type of event

		switch(event) {
			case EVENT_EOF:
			case EVENT_UNDEF:
				break;

			case EVENT_TEXT:
				start_text = inpos;

				// See if waiting for link content
				caption_position = 0;
				if( waiting_link_content ) {
					caption_position = strlen(caption);
					caption[caption_position++] = ' ';
				}

				// Copy the incoming text
				while( !IS_START() && !IS_EOF() ) {

					// Only if it's kept
					if( status == STATUS_NORMAL ) {

						// Copy link content to caption if necessary
						if( waiting_link_content ) {
							if( caption_position < MAX_STR_LEN ) {
								caption[caption_position++] = inbuf[inpos];
							}
						}

						// Copy to output buffer
						outbuf[outpos++] = inbuf[inpos];
					}

					// Swallow multiple spaces
					if( IS_SPACE() ) {
						inpos++;
						while(IS_SPACE() && !IS_EOF()) {
							inpos++;
						}
					} else {
						inpos++;
					}
				}

				// If waiting for link content, close the caption
				if( waiting_link_content ) {
					caption[caption_position] = '\0';
				}
				break;

			case EVENT_START_TAG:
			case EVENT_EMPTY_TAG:

				// Check for we are in the meta tag
				if( !strcmp( tag.name, "meta" ) ) {

					// Scan for name="robots"
					// Scan for http-equiv="refresh"
					bool is_robots	= false;
					bool is_refresh	= false;
					for( int att=0; att<tag.attnum;att++ ) {

						// Check if name="robots"
						if( !strcasecmp(tag.attributes[att].name, "name" ) &&
							!strcasecmp(tag.attributes[att].value, "robots" )) {
							is_robots = true;
						}

						// Check if http-equiv="robots"
						if( !strcasecmp(tag.attributes[att].name, "http-equiv" ) &&
							!strcasecmp(tag.attributes[att].value, "refresh" )) {
							is_refresh = true;
						}
					}

					// If it's <meta name="robots" ...> check for content="..."
					if( is_robots ) {
						char robots[MAX_STR_LEN] = "";

						for( int att=0; att<tag.attnum;att++ ) {

							// Check content="..."
							if( !strcmp(tag.attributes[att].name, "content" ) ) {
								for( uint i=0; i<=strlen(tag.attributes[att].value); i++ ) {
									robots[i] = tolower(tag.attributes[att].value[i]);
								}
							}
						}

						// Check for noindex
						if( strstr( robots, "noindex" ) ) {

							// Report
							cerr << "[noindex]";

							// Empty output buffer
							outbuf[0] = '\0';

							// Zero offset
							return (off64_t)0;
						}

						// Check for nofollow
						if( strstr( robots, "nofollow" ) ) {

							// Report
							cerr << "[index,nofollow]";

							// Don't follow links
							follow_links = false;


						}
							
					// If it's meta http-equiv="refresh" check for content="..."
					} else if( is_refresh ) {
						char refresh_url[MAX_STR_LEN] = "";

						for( int att=0; att<tag.attnum;att++ ) {

							// Check content="..."
							if( !strcmp(tag.attributes[att].name, "content" ) ) {
								strcpy( refresh_url, tag.attributes[att].value );
							}
						}

						if( strlen( refresh_url ) > 0 ) {
							// In a meta refresh, content is of the form
							// "5;URL=http://www.example.com/"
							// so we locate the '=' sign and move to the right
							if( char *href = strchr( refresh_url, '=' ) ) {
								if( strlen(href) > 1 ) {
									href++;
									parser_analyze_link(doc, href, "", (double)inpos/(double)size, (em_tag_t)int_stack_peek(tag_stack));
									href[0] = '\0';
								}
							}
						}

					}

				}

				// Verify if the tag must be kept
				if( perfhash_check( &(keep_tag), tag.name) ) {

					// Copy tag name
					int i = 0;
					outbuf[outpos++] = '<';
					while(tag.name[i] != '\0' ) {
						outbuf[outpos++] = tag.name[i++];
					}


					// Copy attributes
					bool tag_is_link	= is_link[tag.name];

					// If this tag is a the start of a link, and we
					// were already waiting for the content of a link,
					// then we must flush what we have up to now to disk,
					// to avoid loosing an href
					if( tag_is_link && waiting_link_content && follow_links ) {
						parser_analyze_link(doc, href, caption, (double)inpos/(double)size, (em_tag_t)int_stack_peek(tag_stack));
						href[0] = '\0';
						caption[0] = '\0';

						waiting_link_content	= false;
					}

					for(int att=0;att<tag.attnum;att++) {

						// Check if the attribute must be kept
						if( perfhash_check( &(keep_attribute), tag.attributes[att].name) ) {
							int i = 0;
							outbuf[outpos++] = ' ';
							while(tag.attributes[att].name[i] != '\0' ) {
								outbuf[outpos++] = tag.attributes[att].name[i++];
							}
							outbuf[outpos++] = '=';
							outbuf[outpos++] = '\"';
							i = 0;
							while(tag.attributes[att].value[i] != '\0' ) {
								outbuf[outpos++] = tag.attributes[att].value[i++];
							}
							outbuf[outpos++] = '\"';

							// Check if this attribute contains a link
							if( tag_is_link ) {
								if( !strcmp(link_attribute[tag.name].c_str(),tag.attributes[att].name) ) {
									strcpy(href,tag.attributes[att].value);
								}
								if( !strcmp(link_caption[tag.name].c_str(),tag.attributes[att].name) ) {
									strcpy(caption,tag.attributes[att].value);
								}
							}
						}
					}
					if( event == EVENT_EMPTY_TAG ) {
						outbuf[outpos++] = '/';
					}
					outbuf[outpos++] = '>';
				} else {
					// It must not be kept, but it's a word boundary
					outbuf[outpos++] = ' ';
				}
				if( perfhash_check( &(discard_content), tag.name) && (event != EVENT_EMPTY_TAG) ) {
					status = STATUS_IGNORE;
				}

				break;

			case EVENT_END_TAG:

				// Verify if the tag must be kept
				if( perfhash_check( &(keep_tag), tag.name ) ) {
					int i = 0;
					outbuf[outpos++] = '<';
					outbuf[outpos++] = '/';
					while(tag.name[i] != '\0' ) {
						outbuf[outpos++] = tag.name[i++];
					}
					outbuf[outpos++] = '>';
				} else {
					// It must not be kept, but it's a word boundary
					outbuf[outpos++] = ' ';
				}
				if( perfhash_check( &(discard_content), tag.name ) ) {
					status = STATUS_NORMAL;
				}
				break;
		}

		// Check for the tag
		update_tag_stack(tag_stack, tag.name, event);

		// ------- LINKS ------
		// Deal with (possible) links

		// Check if the tag has an attribute indicating a link
		if( href[0] != '\0') {

			// If we are in a starting tag
			if( event == EVENT_START_TAG || event == EVENT_EMPTY_TAG ) {

				// We are starting a tag, or we are reading an empty
				// tag.

				// Check if we have to wait for the caption
				// ie: <a href=""> ... </a>
				if( is_link_with_content[tag.name] ) {

					if( event == EVENT_START_TAG ) {

						if( waiting_link_content == true ) {

							// This is a nested link, like
							// <a href="...">......<a href="....">....
							// Nested links, save current link up to here
							if( follow_links ) {
								parser_analyze_link(doc, href, caption, (double)inpos/(double)size, (em_tag_t)int_stack_peek(tag_stack));
							}
							href[0] = '\0';
							caption[0] = '\0';

						}

						// We have to wait for the caption
						waiting_link_content = true;

					} else if( event == EVENT_END_TAG ) {

						// Malformed "<a href=x.html/>blah</a>"
						if( follow_links ) {
							parser_analyze_link(doc, href, "" , (double)inpos/(double)size, (em_tag_t)int_stack_peek(tag_stack));
						}
						href[0] = '\0';
						caption[0] = '\0';

					}
				} else {

					// This is a tag in which the link content is
					// not the caption

					if( waiting_link_content == true ) {

						// This is a nested link, like
						// <a href="...">......<img src="....">....
						// We cannot save the second link now

						waiting_link_content = false;

					}

					// The caption is an attribute, we don't need to wait
					// for the content of this tag
					if( follow_links ) {
						parser_analyze_link(doc, href, caption, (double)inpos/(double)size, (em_tag_t)int_stack_peek(tag_stack));
					}
					href[0] = '\0';
					caption[0] = '\0';
				}

			} else if( event == EVENT_END_TAG ) {


				// On closing tag, check if i was waiting for a link
				if( waiting_link_content ) {

					// I was waiting for a link, save it
					if( follow_links ) {
						parser_analyze_link(doc, href, caption, (double)inpos/(double)size,(em_tag_t)int_stack_peek(tag_stack));
					}
					href[0] = '\0';
					caption[0] = '\0';
					waiting_link_content = false;
				}
			}
		}

	
	} // End While(1)

	free( tag_stack );

	if( outpos < 0 ) {
			// This may happen in the case of a short, anomalous
			// document like this:
			// "<html " without a closing tag.
			outpos = 0;
	}

	// Close output buffer; it might have ended a few chars
	// after the end, it depends if we ended on a tag or not
	while( outpos >= 1 && outbuf[outpos-1] == '\0' ) {
		outpos--;
	}

	assert( outpos < MAX_DOC_LEN );

	// Ensure we have a null at the end
	outbuf[outpos] = '\0';

	return outpos;
}

//
// Name: parser_init
//
// Description:
//   Initializes the auxiliary vars of the parsers
//

void parser_init() {
	cerr << "Initializing mappings: discard_content, ";

	// Can be set to false for better speed, but less reliability
	// while discarding tags
	bool double_check_kept_markup = false;

	/* We don't want to loose text accidentaly */
	discard_content.check_matches = true;
	perfhash_create( &(discard_content), CONF_GATHERER_DISCARD_CONTENT );

	cerr << "keep tags, ";
	keep_tag.check_matches = double_check_kept_markup;
	perfhash_create( &(keep_tag), CONF_GATHERER_KEEP_TAG );

	cerr << "keep attributes, ";
	keep_attribute.check_matches = double_check_kept_markup;
	perfhash_create( &(keep_attribute), CONF_GATHERER_KEEP_ATTRIBUTE );

	cerr << "extensions to ignore, ";
	extensions_ignore.check_matches = true;
	perfhash_create( &(extensions_ignore), CONF_SEEDER_LINK_EXTENSIONS_IGNORE );

	cerr << "extensions to log, ";
	extensions_log.check_matches = true;
	perfhash_create( &(extensions_log), CONF_SEEDER_LINK_EXTENSIONS_LOG );

	cerr << "extensions to stat, ";
	extensions_stat.check_matches = true;
	perfhash_create( &(extensions_stat), CONF_SEEDER_LINK_EXTENSIONS_STAT );

	cerr << "link attributes, ";
	parser_init_link_attributes(CONF_GATHERER_LINK_ATTRIBUTE);
	cerr << "done." << endl;

}

//
// Name: parser_save_extensions_stats
//
// Description:
//   Saves the extensions that we need statistics about
//

void parser_save_extensions_stats( FILE *links_stat ) {
	map<string,docid_t>::iterator extensions_it;
	for( extensions_it = count_extensions_stat.begin(); extensions_it != count_extensions_stat.end();                extensions_it++ ) {
		fprintf( links_stat, "%lu %s\n", (*extensions_it).second, ((*extensions_it).first).c_str() );
	}
}

//
// Name: parser_init_link_attributes
//
// Description:
//   Initialize several mapings regarding attributes that are links
//   in certain html elements.
//
// Input:
//   str - string containing configuration for linkattributes
// 

void parser_init_link_attributes(char *str) {
	if( str==NULL ) {
		return;
	}

	char *s = strtok(str,CONF_LIST_SEPARATOR);
	while(s != NULL) {
		char tagname[MAX_STR_LEN];
		char attname[MAX_STR_LEN];
		char caption[MAX_STR_LEN];

		uint i;
		uint j;

		// Copy the tag name
		j = 0;
		for(i=0; s[i]!='/'; i++,j++) {
			assert(s[i]!='\0');
			tagname[j] = s[i];	
		}
		tagname[j] = '\0';
		i++;

		// Copy the link attribute name
		j = 0;
		for(;s[i]!='/';i++,j++) {
			assert(s[i]!='\0');
			attname[j] = s[i];
		}
		attname[j] = '\0';
		i++;

		// Copy the caption attribute
		j = 0;
		for(;s[i]!='\0' && s[i]!='+';i++,j++) {
			caption[j] = s[i];
		}
		caption[j] = '\0';


		is_link[tagname] = true;
		link_attribute[tagname] = attname;
		link_caption[tagname] = caption;

		// Check that the tag is preserved
		if( ! perfhash_check( &(keep_tag), tagname) ) {
			cerr << endl;
			cerr << "Error! failed to found " << tagname << " in tags: " << CONF_GATHERER_KEEP_TAG << endl;
			die( "Tag for link discovery is not preserved" );
		}

		// Check that the attributes are preserved
		if( ! perfhash_check( &(keep_attribute), attname) ) {
			cerr << endl;
			cerr << "Error! failed to found " << attname << " in atributes: " << CONF_GATHERER_KEEP_ATTRIBUTE << endl;
			die( "Attribute for link discovery is not preserved" );
		}
		if( ! perfhash_check( &(keep_attribute), caption) ) {
			cerr << endl;
			cerr << "Error! failed to found " << caption << " in atributes: " << CONF_GATHERER_KEEP_ATTRIBUTE << endl;
			die( "Attribute for link discovery is not preserved" );
		}

		if( s[i] == '+' ) {
			is_link_with_content[tagname] = true;
		}
		s = strtok(NULL,CONF_LIST_SEPARATOR);
	}
	return;
}

//
// Name: parser_analyze_link
//
// Description:
//   Analyze a link
//
// Input:
//   doc - source document
//   href - destination of link
//   caption - text of link
//

void parser_analyze_link(doc_t *doc, char *href, char *caption, double rel_pos, em_tag_t tag) {
	assert( doc != NULL );
	assert( href != NULL );

	// Ignore some common cases

	if( href[0] == '#' ) {
		// Local reference to the same page, ignore
		return;

	} else if( strchr(href,':') != NULL ) {
		// URL with protocol

		// If it's javascript, ignore
		if( ! strncasecmp( href, "javascript:", 11 ) ) {
			return;
		}

		// If it's mailto, ignore
		if( ! strncasecmp( href, "mailto:", 7 ) ) {
			return;
		}

	} else if( strchr(href,'@') != NULL ) {
		// Malformed e-mail URL, ignore
		return;

	}

	// Check the href (no spaces), cut at first hash mark
	for( uint i=0; i<=strlen(href); i++ )
	{
		if( isspace( href[i] ) )
		{
			// Has spaces or newlines, ignore
			return;
		}
		if( href[i] == '#' ) {
			href[i] = '\0';
		}
	}

	// Put the caption in one line
	for( uint i=0; i<=strlen(caption); i++ )
	{
		if( isspace( caption[i] ) )
		{
			caption[i] = ' ';
		}
	}

	// Look for the type of extension
	char extension[MAX_STR_LEN];
	extension[0] = '\0';
	extension_type_t extension_type = parser_check_extension( href, extension );

	if( extension_type == EXTENSION_NORMAL ) {
		if( strcmp( caption, LINK_CAPTION_FORBIDDEN ) != 0 ) {
			// We skip normal links that, by change, have the same caption that we use for marking
			// forbidden links in robots.txt files
			fprintf( links_download, "%lu %d %d %s %s\n", doc->docid, (int)(100.0*(1 - rel_pos)), tag, href, caption );
		}

	} else if( extension_type == EXTENSION_LOG ) {
		fprintf( links_log, "%lu %d %d %s %s\n", doc->docid, (int)(100.0*(1 - rel_pos)), tag, href, caption );

	} else if( extension_type == EXTENSION_STAT ) {
		// We count these items only if they are local links, because
		// as we use this for web characterization, we want to count
		// the number of say, images, INSIDE the collection
		if( (strlen(extension) > 0) && (strchr(href,':') == NULL) ) {
			count_extensions_stat[extension] ++;
		}
		
	} else {
		// Ignore
	}

	// Blank both parts of the links, because for the next link
	// we don't know what are we going to get first (the href or the caption)

	return;
}

//
// Name: parser_process_robotstxt
//
// Description:
//   Process a robots.txt file. These files usually are not very
//   strict, so this parser is not stricts and errs in the side
//   of caution, disallowing this robot to the specified directories.
//   This parser doesn't support excluding of a particular file,
//   because that should be done via a "noindex" tag.
//
// Input:
//   doc - the document object
//   inbuf - the contents of the robots.txt file
//
// Return:
//   the number of exclusions found
//

off64_t parser_process_robotstxt( doc_t *doc, char *inbuf, char *outbuf) {
	off64_t size = doc->raw_content_length;

	uint exclusions = 0;

	char line[MAX_STR_LEN];

	// This variable is true if the robots.txt file refers
	// to this user-agent, or to all user agents.
	bool useragent_match = false;

	for( off64_t inpos = 0; inpos<doc->raw_content_length; inpos++ ) {

		// Copy a single line
		off64_t linepos = 0;
		while( inpos < size
				&& (inbuf[inpos] != '\n' && inbuf[inpos] != '\r' ) )
		{
			// Do not copy long lines
			if( linepos < MAX_STR_LEN - 10 ) {
				line[linepos++] = inbuf[inpos];
			}
			inpos++;
		}
		line[linepos++] = '\n';
		line[linepos]   = '\0';

		// Skip the trailing newlines
		while( inpos < size
				&& (inbuf[inpos] == '\n' || inbuf[inpos] == '\r' ) )
		{
			inpos++;
		}
		inpos--;

		// Skip comments
		if( line[0] == '#' ) {
			continue;
		}

		// Look for user-agent tags
		if( !strncasecmp( line, "user-agent", strlen("user-agent"))
		  ||!strncasecmp( line, "useragent",  strlen("useragent"))) {

			// It only needs to have the name of this bot,
			// or an asterisk.
			if( strchr( line, '*') != NULL )
			{
				useragent_match = true;
			} else {
				char line_cpy[MAX_STR_LEN];
				char package_cpy[MAX_STR_LEN];

				assert( strlen(line) < MAX_STR_LEN );
				strcpy( line_cpy, line );
				assert( strlen(PACKAGE) < MAX_STR_LEN );
				strcpy( package_cpy, PACKAGE );

				str_tolower( line_cpy );
				str_tolower( package_cpy );

				if( strstr( line_cpy, package_cpy ) != NULL ) {
					useragent_match = true;
				} else {
					useragent_match = false;

				}


			}
		}

		// Look for disallow tags
		else if( !strncasecmp( line, "disallow", strlen("disallow") ))
		{
			uint i = 0;
			// Skip "disallow"
			while( line[i] != ':' && line[i] != '\n' )
			{
				i++;
			}
			// Skip ' : '
			while( (line[i] == ':' || isspace( line[i]))
				&& line[i] != '\n' )
			{
				i++;
			}
			char dirname[MAX_STR_LEN];
			uint j = 0;

			// Copy the directory name
			while( !isspace( line[i] ) && line[i] != '\n' )
			{
				dirname[j++] = line[i++];
			}
			dirname[j] = '\0';

			// Save the disallow instruction only if:
			// - We match the useragent, or useragent is '*'
			// - It is an absolute path
			if( useragent_match && dirname[0] == '/' ) {
				fprintf( links_download, "%lu 0 0 %s %s\n",
						doc->docid, dirname, LINK_CAPTION_FORBIDDEN );
				exclusions++;
			}

		}
	}

	// Return the number of exclusions found (NOT the number of bytes, note that
	// the output buffer does not contain valid data)
	return (off64_t)exclusions;
}

//
// Name: parser_process_robotsrdf
//
// Description:
//   Process a robots.rdf file [experimental]
//
// Input:
//   doc - the document object
//   inbuf - the contents of the robots.txt file
//
// Return:
//

off64_t parser_process_robotsrdf( doc_t *doc, char *inbuf, char *outbuf) {
	uint nfiles	= 0;
	xmlDocPtr           xmldoc		= NULL;
	xmlXPathContextPtr	xmlcontext	= NULL;
	xmlNodePtr			node		= NULL;

	char href[MAX_STR_LEN]						= "";
	struct tm last_modified_struct;
	time_t last_modified;

	// Ensure a consistent buffer on output
	outbuf[0] = '\0';

	// Advance possible whitespace
	inbuf[doc->raw_content_length]	= '\0';
	while( isspace(*inbuf) ) {
		inbuf++;
	}

	// Parse the XML file
	xmldoc	= xmlReadMemory( inbuf, doc->raw_content_length, "file:///dev/null", NULL,
//			XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET );
			0 );

	// Check if the document was parsed ok
	if( xmldoc == NULL ) {

		cerr << "***" << inbuf << "***" << endl;

		// Signal an error condition
		return 0;
	}

	// Create an XPATH context
	xmlcontext	= xmlXPathNewContext( xmldoc );

#define NAMESPACE_RDF	"http://www.w3.org/1999/02/22-rdf-syntax-ns"
#define NAMESPACE_DC	"http://purl.org/dc/elements/1.1/"
#define NAMESPACE_RSS	"http://purl.org/rss/1.0/"
#define RDF_TIME_FORMAT	"%Y-%m-%dT%H:%M"

	// Register namespaces
	xmlXPathRegisterNs(xmlcontext, (const xmlChar *)"rdf",	(const xmlChar *)NAMESPACE_RDF );
	xmlXPathRegisterNs(xmlcontext, (const xmlChar *)"dc",	(const xmlChar *)NAMESPACE_DC );
	xmlXPathRegisterNs(xmlcontext, (const xmlChar *)"rss",	(const xmlChar *)NAMESPACE_RSS );

	// Iterate
	xmlXPathObjectPtr path;
	path = xmlXPathEvalExpression((const xmlChar *)"/rdf:rdf/rss:item" , xmlcontext);

	if( path->nodesetval == NULL || path->nodesetval->nodeNr <= 0 ) {
		xmlFreeDoc( xmldoc );
		xmlXPathFreeContext( xmlcontext );
		return 0;
	}


	xmlNodePtr ptr	= NULL;
	for( int i=0; i<path->nodesetval->nodeNr; i++ ) {
		// Extract node
		node	= path->nodesetval->nodeTab[i];

		// This <item> node needs to have children
		if( node->children == NULL ) {
			continue;
		}

		strcpy( href, "" );
		last_modified	= 0;

		// Check children of this <item> element
		ptr = node->children;
		while( ptr != NULL ) {

			if( !strcmp( (char *)(ptr->name), "link" ) ) {
				// We are on a <link> element, get its content
				if( ptr->children == NULL || ptr->children->type != XML_TEXT_NODE ) {
					continue;
				}
				strcpy( href, (char *)(ptr->children->content) );
			}

			if( !strcmp( (char *)(ptr->name), "modified" ) ) {
				// We are on a <modified> element, get its content
				if( ptr->children == NULL || ptr->children->type != XML_TEXT_NODE ) {
					continue;
				}
				strptime( (char *)(ptr->children->content), RDF_TIME_FORMAT, &(last_modified_struct) );
				last_modified	= mktime( &(last_modified_struct) );
			}
			ptr = ptr->next;
		}

		// Print to output
		if( strlen(href) > 0 ) {
			nfiles ++;

			if( last_modified > 0 ) {
				// If I found a last-modified stamp
				fprintf( links_download, "%lu 0 0 %s %s %d\n",
					doc->docid, href, LINK_CAPTION_LAST_MODIFIED, (int)(last_modified) );
			} else {
				// I didn't found information, but I least I have a normal link
				fprintf( links_download, "%lu 0 0 %s \n",
					doc->docid, href );
			}
		}
	}

	// Free the document
	xmlFreeDoc( xmldoc );
	xmlXPathFreeContext( xmlcontext );
	
	return nfiles;
}

// 
// Name: update_tag_stack
//
// Description:
//   Updates the tag stack to reflect current parser status
//
// Input:
//   tag_stack - the tag_stack
//   current_tag - the current tag
//   event - type of event (start tag/ end tag)
//

void update_tag_stack(int_stack_t *tag_stack, char *current_tag, event_t event){
	em_tag_t tag	= TAG_UNDEF;

	// TO-DO: this should be implemented with a perfhash to make it faster

	// Check if we are on any special tag
	if(strcmp(current_tag, "h1")==0) {
		tag	= TAG_H1;
	} else if(strcmp(current_tag, "h2")==0) {
		tag	= TAG_H2;
	} else if(strcmp(current_tag, "h3")==0) {
		tag	= TAG_H3;
	} else if(strcmp(current_tag, "h4")==0) {
		tag	= TAG_H4;
	} else if(strcmp(current_tag, "h5")==0) {
		tag	= TAG_H5;
	} else if(strcmp(current_tag, "h6")==0) {
		tag	= TAG_H6;
	} else if(strcmp(current_tag, "b")==0) {
		tag	= TAG_B;
	} else if(strcmp(current_tag, "font")==0) {
		tag = TAG_FONT;
	} else if(strcmp(current_tag, "strong")==0) {
		tag = TAG_STRONG;
	}

	// Only these tags may modify the tag stack
	if( tag != TAG_UNDEF ) {
		if( event == EVENT_START_TAG ) {
			int_stack_push( tag_stack, tag );
		} else {
			int_stack_pop( tag_stack );
		}
	}
}


// 
// Name: seeder_is_rejected_by_extension
//
// Description:
//   Lowercases a extension, then check against
//   extensions_ignore and extensions_log
//
// Input:
//   path - the path and filename to check
//
// Output:
//   extension - the extension, if any
//
// Return:
//   true iff the path must be rejected, based on its extension
//

extension_type_t parser_check_extension( char *path, char *extension ) {
	// Check if there is a path
	if( path == NULL ) {
		return EXTENSION_NORMAL;
	}

	// Check if it has extension
	urlidx_get_lowercase_extension( path, extension );

	if( strlen( extension ) == 0 ) {
		return EXTENSION_NORMAL;
	}

	// Search
	if( perfhash_check( &(extensions_ignore), extension) ) {
		return EXTENSION_IGNORE;
	} else if( perfhash_check( &(extensions_stat), extension ) ) {
		return EXTENSION_STAT;
	} else if( perfhash_check( &(extensions_log), extension ) ) {
		return EXTENSION_LOG;
	}

	return EXTENSION_NORMAL;
}

