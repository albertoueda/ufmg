
#include "lang.h"

char		EXTRA_WORD_CHARACTERS[MAX_DOC_LEN];
uint		EXTRA_WORD_CHARACTERS_COUNT				= 0;

perfhash_t	lang_hash[LANG_MAX_COUNT];
char		lang_names[LANG_MAX_COUNT][MAX_STR_LEN];
uint		lang_count									= 0;

//
// Name: lang_init
// 
// Description:
//   Inits the language recognition environment.
//

void lang_init() {

	//
	// Load the list of stopwords.
	//

	// We need to make a copy because strtok modifies
	// its first argument
	char lang_stopwords[MAX_STR_LEN];
	strcpy( lang_stopwords, CONF_ANALYSIS_LANG_STOPWORDS );

	// Get the language names, we will just check
	// how many languages and extract the language names
	char *langfile;
	langfile = strtok( lang_stopwords, CONF_LIST_SEPARATOR );
	cout << "Language files: " << endl;
	while( langfile != NULL ) {
		strcpy( lang_names[lang_count], langfile );
		cerr << " - " << lang_count << " " << lang_names[lang_count] << endl;

		langfile = strtok( NULL, CONF_LIST_SEPARATOR );

		lang_count++;
		assert( lang_count < LANG_MAX_COUNT );
	}

	//
	// First, we must check for short or repeated words,
	// to be sure that there are no interferences between
	// the language files.
	//
	map <string,bool> must_exclude;
	map <string,uint> seen_word;

	for( uint i=0; i<lang_count; i++ ) {

		map <string,uint> seen_word_this_file;

		// Read the file with the stopwords
		char stopwords[MAX_DOC_LEN];
		char filename[MAX_STR_LEN];
		sprintf( filename, "%s/%s", CONF_ANALYSIS_LANG_BASEPATH, lang_names[i] );
		read_file( filename, stopwords );

		// Iterate through the file
		char *word	= strtok( stopwords, CONF_LIST_SEPARATOR );
		while( word != NULL ) {

			// Count occurences of this word
			seen_word[word] ++;
			seen_word_this_file[word] ++;

			// Check if the word is too small
			if( strlen(word) == 1 ) {
				must_exclude[word]	= true;
			}

			// Check if the word has occured more than once 
			// in this file; this is to check the consistency
			// of the stopwords files
			if( seen_word_this_file[word] > 1 ) {
				cerr << "Repeated word in file " << filename << ":" << word << endl;
				die( "Correct the language files" );
			}

			// Check if the word has occured more than once
			// in all the files, if it occurs in two different
			// files, then it must be discarded, as it is not
			// useful for identifying language
			if( seen_word[word] > 1 ) {
				must_exclude[word]	= true;
			}

			// Next word
			word	= strtok( NULL, CONF_LIST_SEPARATOR );
		}
	}

	// Report words that will be excluded
	map <string,bool>::iterator must_exclude_it;
	cout << "Excluded words, repeated or too short: " << endl;
	cout << " - ";
	for( must_exclude_it = must_exclude.begin(); must_exclude_it != must_exclude.end(); must_exclude_it++ ) {
		cout << (*must_exclude_it).first << " ";
	}
	cout << endl;

	// 
	// Now we can really load the stop words,
	// excluding the words that are repeated
	//

	cout << "Loading stopwords: " << endl;
	for( uint i=0; i<lang_count; i++ ) {
		// Read the file with the stopwords
		char stopwords_raw[MAX_DOC_LEN];
		char stopwords[MAX_DOC_LEN]	= "";
		char filename[MAX_STR_LEN];
		sprintf( filename, "%s/%s", CONF_ANALYSIS_LANG_BASEPATH, lang_names[i] );
		read_file( filename, stopwords_raw );

		// Copy without repeated words
		char *word	= strtok( stopwords_raw, CONF_LIST_SEPARATOR );
		uint nwords_ok	= 0;
		while( word != NULL ) {
			// Check if the word is excluded
			if( ! must_exclude[word] ) {
				strcat( stopwords, word );
				strcat( stopwords, " " );
				nwords_ok	++;
			}

			// Next word
			word = strtok( NULL, CONF_LIST_SEPARATOR );
		}

		// Load the stopwords of that language
		perfhash_t *hash	= &(lang_hash[i]);
		hash->check_matches = true;
		perfhash_create( hash, stopwords );

		cout << "- " << lang_names[i] << ": " << nwords_ok << " words ok" << endl;

		// Copy the characters seen that are not already
		// alpha numeric. This is necessary because the
		// macro isalnum() only works with one locale at a time,
		// and we want to identify alphanumeric characters
		// in many locales at the same time.
		for( uint i=0; i<strlen(stopwords); i++ ) {
			char	c = stopwords[i];
			if( ! (isalnum(c) || isspace(c)) ) {

				bool seen	= false;
				for( uint j=0; j<EXTRA_WORD_CHARACTERS_COUNT; j++ ) {
					if( c == EXTRA_WORD_CHARACTERS[j] ) {
						seen = true;
					}
				}

				if( ! seen ) {
					EXTRA_WORD_CHARACTERS[EXTRA_WORD_CHARACTERS_COUNT++] = c;
					assert( EXTRA_WORD_CHARACTERS_COUNT < MAX_DOC_LEN );
				}
			}

		}


	}

	EXTRA_WORD_CHARACTERS[EXTRA_WORD_CHARACTERS_COUNT] = '\0';
	cout << "New characters found to be part of words:" << endl;
	cout << "- " << EXTRA_WORD_CHARACTERS << endl;
	cout << "* This list can contain strange characters, but they all" << endl;
	cout << "  must be letters in some language." << endl;

}

//
// Name: lang_identify_document
//
// Description:
//   Guess the language in which a document is written
//   
// Input:
//   buffer - the document to be analyzed
//   buffer_length - the length of the document
//   output - file to report results to
//   word_count - number of words found
//
// Return:
//   0 if undefined
//   >0 with a language number if sucesful
//

int lang_identify_document( char *buffer, off64_t buffer_length, FILE *output, uint *word_count ) {
	char word[MAX_STR_LEN];
	bool in_word = false;
	int word_length = 0;
	int language_number;
	bool in_tag = false;

	// Init a score vector
	uint *scores = (uint *)(malloc(sizeof(uint) * lang_count));
	for( uint i=0; i<lang_count; i++ ) {
		scores[i] = 0;
	}
	
	// The structured version is really ugly,
	// this would look much better with GOTOs
	// 
	// We only care about words, and ignore everything
	// witin tags. Notice that this is only for HTML,
	// but could be applied to a plaintext that does not
	// contain the character '<'
	for( off64_t i=0; i<buffer_length; i++ ) {
		char c = buffer[i];

		if( in_tag ) {
			if( c == '>' ) {
				in_tag = false;
			}
		} else {
			if( in_word ) {
				if( lang_is_separator(c) || c == '<' ) {
					word[word_length] = '\0';
					language_number = lang_check_word( word );
					scores[language_number-1] ++;
					(*word_count) ++;
					in_word = false;
					if( c == '<' ) {
						in_tag = true;
					}
				} else {
					if( word_length < MAX_STR_LEN ) {
						word[word_length++] = c;
					}
				}
			} else {
				if( lang_is_separator(c) || c == '<' ) {
					if( c == '<' ) {
						in_tag = true;
					}
				} else {
					word[0] = c;
					word_length = 1;
					in_word = true;
				}
			}
			
		}
	}

	// Last word
	if( in_word && word_length > 0 ) {
		word[word_length] = '\0';
		language_number = lang_check_word( word );
		scores[language_number-1] ++;
		(*word_count) ++;
	}

	// We need to keep the first and second place to compare
	// the difference
	uint first_place_score	= 0;
	uint first_place		= 0;
	uint second_place_score	= 0;
	for( uint i=0; i<lang_count; i++ ) {
		fprintf( output, "%d,", scores[i] );

		if( scores[i] > first_place_score ) {
			second_place_score	= first_place_score;
			
			first_place			= i+1;
			first_place_score	= scores[i];
		} else if( scores[i] > second_place_score ) {
			second_place_score	= scores[i];
		}
	}
	fprintf( output, "%d,", (*word_count) );

	free(scores );

	// Try to infer the language
	int lang	= 0;

	if( ((uint)(*word_count) >= CONF_ANALYSIS_LANG_MIN_WORDS) 
	 && (first_place_score >= CONF_ANALYSIS_LANG_MIN_STOPWORDS)
	 && (first_place_score - second_place_score >= CONF_ANALYSIS_LANG_MIN_DIFFERENCE ) ) {
		lang	= first_place;
	}
	fprintf( output, "%d\n", lang );

	return lang;
}

//
// Name: lang_analyze
//
// Description:
//   Analyzes files to get languages
//  

void lang_analyze() {

	// Init languages
	lang_init();
		
	// Create a single buffer to read all files
	char *buffer = (char *)malloc(sizeof(char) * (MAX_DOC_LEN + 1));

	// Open output file
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s/%s/lang", CONF_COLLECTION_BASE, COLLECTION_ANALYSIS );
	createdir( filename );
	sprintf( filename, "%s/%s/lang/docid_words.csv", CONF_COLLECTION_BASE, COLLECTION_ANALYSIS );

	FILE *output	= fopen64( filename, "w" );
	assert( output != NULL );

	fprintf( output, "docid,depth,in_degree,pagerank,last_visit,last_modified," );
	for( uint langnum=1; langnum<=lang_count; langnum++ ) {
		fprintf( output, "%s,", lang_names[langnum-1] );
	}
	fprintf( output, "word count,language\n" );

	// Iterate
	storage_status_t rc;

	docid_t ndocs			= metaidx->count_doc;
	docid_t ndocs_div_50    = (ndocs / 50);
	cerr << "Analyzing    |--------------------------------------------------|" << endl;
	cerr << "              ";

	int lang	= 0;
	docid_t **lang_depth	= (docid_t **)malloc(sizeof(docid_t *) *
			(CONF_MANAGER_MAXDEPTH_STATIC+2));
	assert( lang_depth != NULL );

	docid_t *count_lang			= (docid_t *)malloc(sizeof(docid_t) * 
			(lang_count + 1 ));
	assert( count_lang != NULL );

	// Remember: langnum 0 is undefined
	for( uint langnum=0; langnum<=lang_count; langnum++ ) {
		count_lang[langnum]	= 0;
	}

	docid_t	cnt_sampled		= 0;
	docid_t cnt_words_ok	= 0;

	// Clean
	for( depth_t depth=1; depth <= CONF_MANAGER_MAXDEPTH_STATIC + 1; depth ++ ) {
		lang_depth[depth]	= (docid_t *)malloc(sizeof(docid_t) * (lang_count+1));
		assert( lang_depth[depth] != NULL );

		// Remember: lang 0 is undefined
		for( uint lang=0; lang<=lang_count; lang++ ) {
			lang_depth[depth][lang]	= 0;
		}
	}

	// Iterate through documents
	map <uint,uint> nsaved_text;
	for( docid_t docid=1; docid<=ndocs; docid++ ) {

		
		if( ndocs_div_50 > 0 && ( docid % ndocs_div_50 ) == 0 ) {
			cerr << ".";
		}

		// Skip some documents
		if( ( docid % CONF_ANALYSIS_LANG_SAMPLE_EVERY ) != 0 ) {
			continue;
		}

		// Read storage
		off64_t buffer_length		= 0;
		rc = storage_read( storage, docid, buffer, &(buffer_length) );

		// storage_read can fail if the document does not exist
		if( rc == STORAGE_OK ) {


			// Read the document
			doc_t doc;
			doc.docid = docid;
			metaidx_status_t mrc = metaidx_doc_retrieve( metaidx, &(doc) );
			assert( doc.docid == docid );
			assert( mrc == METAIDX_OK );

			// Print basic data about the document
			buffer[buffer_length] = '\0';
			fprintf( output, "%lu,", docid );
			fprintf( output, "%d,", doc.depth );
			fprintf( output, "%d,", doc.in_degree );
			fprintf( output, "%e,", doc.pagerank );
			fprintf( output, "%d,", (int)(doc.last_visit) );
			fprintf( output, "%d,", (int)(doc.last_modified) );

			// Analyze
			uint word_count			= 0;
			lang = lang_identify_document( buffer, buffer_length, output, &(word_count) );

			if( lang > 0 ) {
				// Check if we have enough samples of documents
				// in this language. This is useful to check
				// how well/bad is the algorithm behaving.
				if( nsaved_text[lang] < CONF_ANALYSIS_LANG_SAVE_TEXT ) {

					// Save a sample
					nsaved_text[lang] ++;
					char filename[MAX_STR_LEN];

					// Create directory for extracting samples
					sprintf( filename, "%s/lang/sample_%s",
							COLLECTION_ANALYSIS, lang_names[lang-1] );
					createdir( filename );

					// Extract sample
					sprintf( filename, "%s/lang/sample_%s/%d.txt",
							 COLLECTION_ANALYSIS, lang_names[lang-1], nsaved_text[lang] );
					FILE *out	= fopen64( filename, "w" );
					assert( out != NULL );
					fwrite( buffer, buffer_length, 1, out );
					fclose( out );

				}
			}

			// Count at depth
			if( doc.depth > 0 && doc.depth <= CONF_MANAGER_MAXDEPTH_STATIC ) {
				lang_depth[doc.depth][lang]	++;
			}

			count_lang[lang]	++;
			cnt_sampled			++;
			if( word_count >= CONF_ANALYSIS_LANG_MIN_WORDS ) {
				cnt_words_ok ++;
			}
		}
	}
	cerr << " done." << endl;
	fclose( output );
	free(buffer);

	//
	// Summary report
	// 

	sprintf( filename, "%s/lang/summary.csv", COLLECTION_ANALYSIS );

	output	= fopen64( filename, "w" );
	assert( output != NULL );

	fprintf( output, "Documents sampled,%lu\n", cnt_sampled );
	fprintf( output, "Documents with more than %d words,%lu\n", 
			CONF_ANALYSIS_LANG_MIN_WORDS, cnt_words_ok );
	fprintf( output, "Documents that were not identified,%lu\n",
			count_lang[0] );
	fclose( output );

	//
	// Documents per language
	//

	sprintf( filename, "%s/lang/count.csv", COLLECTION_ANALYSIS );
	output	= fopen64( filename, "w" );
	assert( output != NULL );
	fprintf( output, "Document language,Number of documents,Fraction\n" );
	for( uint langnum=1; langnum<=lang_count; langnum++ ) {
		fprintf( output, "%s,%lu,%e\n",
				lang_names[langnum-1], count_lang[langnum],
				(double)(count_lang[langnum]) /
				(double)(cnt_sampled - count_lang[0])
		);
	}
	fclose( output );

	//
	// Documents and language by depth
	// 

	sprintf( filename, "%s/lang/by_depth.csv", COLLECTION_ANALYSIS );

	output	= fopen64( filename, "w" );
	assert( output != NULL );

	fprintf( output, "depth" );
	for( uint langnum=1; langnum<=lang_count; langnum++ ) {
		fprintf( output, ",%s", lang_names[langnum-1] );
		fprintf( output, ",Fraction" );
	}
	fprintf( output, "\n" );

	for( depth_t depth=1; depth <= CONF_MANAGER_MAXDEPTH_STATIC; depth ++ ) {
		fprintf( output, "%d", depth );

		// Get the sum of recognized documents at this depth
		docid_t	cnt_ok	= 0;
		for( uint langnum=1; langnum<=lang_count; langnum++ ) {
			cnt_ok	+= lang_depth[depth][langnum];
		}

		for( uint langnum=1; langnum<=lang_count; langnum++ ) {
			fprintf( output, ",%lu", lang_depth[depth][langnum] );
			fprintf( output, ",%e",
				(double)(lang_depth[depth][langnum])
			  / (double)cnt_ok );
		}
		fprintf( output, "\n" );
	}
	fclose( output );

	// Free memory
	free(lang_depth);
}

//
// Name: lang_check_word
//
// Description:
//   Indicates in which of the hashes of stopwords is a word
//   Notice that it is important that the most common language
//   is listed first.
//
// Input:
//   word - the word to be checked
//
// Return:
//   -1 if the word is not found
//   >0 to indicate the number of the language if it is found
//   

int lang_check_word( char *word ) {
	for( uint i=0; i<lang_count; i++ ) {
		if( perfhash_check( &(lang_hash[i]), word ) ) {
			// Language numbers are positive integer
			return (i + 1);
		}
	}
	return -1;
}

//
// Name: lang_is_separator
//
// Description:
//   Checks if a character is a separator
//   Separators include:
//     - spaces
//     - characters not included on the letters loaded from the
//       stopwords lists
//

bool lang_is_separator( char c ) {
	if( isspace(c) ) {
		return true;
	} else if( c == '&' || c == ';' ) {
		return false;
	}
	
	// Maybe our list includes non-alphabetic characters
	// that should be treated as alphanumeric
	for( uint i=0; i<EXTRA_WORD_CHARACTERS_COUNT; i++ ) {
		if( c == EXTRA_WORD_CHARACTERS[i] ) {
			return false;
		}
	}

	if( isalnum(c) ) {
		return false;
	}

	return true;
}
