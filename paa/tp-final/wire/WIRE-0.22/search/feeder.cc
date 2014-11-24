
#include "feeder.h"

// 
// Name: main
//
// Description:
//   Main program for the feeder


int main( int argc, char **argv ) {

	// Init
	wire_start("search-idx-feeder" );
	
	docid_t opt_from	= 0;
	docid_t opt_to		= 0;
	feeder_format_t opt_format	= FEEDER_FORMAT_UNDEFINED;

	while(1) {
		int option_index = 0;

		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"from", 1, 0, 0},
			{"to", 1, 0, 0},
			{"format", 1, 0, 0 },
			{0, 0, 0, 0}
		};

		char c = getopt_long (argc, argv, "h",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 0:
				if( !strcmp( long_options[option_index].name,
							"from" ) ) {
					opt_from = atol( optarg );
					if( opt_from == 0 ) {
						feeder_usage();
					}
				} else if( !strcmp( long_options[option_index].name,
							"to" ) ) {
					opt_to = atol( optarg );
					if( opt_to == 0 ) {
						feeder_usage();
					}
				} else if( !strcmp( long_options[option_index].name,
							"format" ) ) {
					if( !strcmp( optarg, "swish-e" ) ) {
						opt_format = FEEDER_FORMAT_SWISHE;
					} else if( !strcmp( optarg, "lucene" ) ) {
						opt_format = FEEDER_FORMAT_LUCENE;
					} else {
						die( "Unrecognized format, only 'swish-e' and 'lucene' are currently valid" );
					}
					
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					feeder_usage();
				}
				break;
			case 'h':
				feeder_usage();
				break;
			default:
				feeder_usage();
		}
	}

	if( opt_format == FEEDER_FORMAT_UNDEFINED ) {
		feeder_usage();
		die( "You must specify a file format using '--format'" );
	}

	// Open url index
	cerr << "Opening urlindex ... ";
	urlidx = urlidx_open( COLLECTION_URL , true);
	assert( urlidx != NULL );

	// Open metaindex
	cerr << "metaindex ...";
	metaidx = metaidx_open( COLLECTION_METADATA, true );
	assert( metaidx != NULL );

	// Open storage
	cerr << "storage ... ";
	storage = storage_open( COLLECTION_TEXT, true );
	assert( storage != NULL );

	cerr << "done." << endl;

	// Default options
	docid_t count_doc = calculate_scores();
	docid_t start		= 1;
	docid_t finish		= count_doc;

	// Limits
	if( opt_from > 0 ) {
		start = opt_from;
	}
	if( opt_to > 0 && opt_to < count_doc ) {
		finish	= opt_to;
	}

	docid_t ndocs_ok	= 0;
	char *buffer	= (char *)malloc(sizeof(char)*MAX_DOC_LEN);
	assert( buffer != NULL );
	
	for(docid_t i=start; i<finish; i++){

	// THERE IS SOME BUG IN SWISH-E THAT CAUSE A SEGMENTATION FAULT
	// WHILE INDEXING SOME HTML FILES WATCH SWISH-E LOGS TO CHECK IF
	// THERE IS A SUCH ERROR AND SKIP THAT FILE MANUALLY USING THE
	// NEXT LINE
	// if(i== <BAD FILE IDX>) continue;

		if((i % 50)==0) {
			cerr << " " << i << "/" << finish <<" (" << ndocs_ok << " ok)" << endl;
		}

		if(cscore[i]==-1) {
			cerr << "_";
		} else {
			if(feed_program(i, buffer, pagerank[i], wlrank[i], cscore[i], opt_format)) {
				ndocs_ok++;
			}
		}
		
	}
	cerr << "done." << endl;

	// End
	free( buffer );

	wire_stop(0);
}

//
// Name: calculate_scores
//
// Description: 
//   Extracts and calculates scores for feeding them to swish-e
//

docid_t calculate_scores() {
	docid_t count_doc	= metaidx->count_doc;

	//order = (docid_t *)malloc(sizeof(docid_t)*count_doc);
	pagerank = (score_t *)malloc(sizeof(score_t)*(count_doc+1));
	wlrank = (score_t *)malloc(sizeof(score_t)*(count_doc+1));
	cscore = (score_t *)malloc(sizeof(score_t)*(count_doc+1));
	
	metaidx_status_t rc;
	doc_t doc;

	docid_t ndocs_div_50= count_doc / 50;
	cerr << "Extracting scores |--------------------------------------------------|" << endl;
	cerr << "                  ";

	// Iterate through documents in the collection
	for(doc.docid=1; doc.docid<=count_doc; doc.docid++){

		// Report
		if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
				cerr << ".";
		}

		rc = metaidx_doc_retrieve( metaidx, &(doc) );
	   //order[doc.docid-1]=doc.docid;
	   if( rc == METAIDX_OK ) {
		   if(HTTP_IS_OK(doc.http_status)) {
			   pagerank[doc.docid]=(score_t)(100000000 * doc.pagerank);
			   wlrank[doc.docid]=(score_t)(100000000 * doc.wlrank);
			   cscore[doc.docid]=(score_t)(doc.future_score >=0? doc.future_score : 0);
		   }
		   else{
			   pagerank[doc.docid]=-1;
			   wlrank[doc.docid]=-1;
			   cscore[doc.docid]=-1;
		   }
	   }
	}
	cerr << "done." << endl;

/*
    cerr << "Sorting array... ";
	qsort(order, doc.docid, sizeof(docid_t), feeder_compare_by_score);
	cerr << "done." << endl;
*/
	return count_doc;
	
}


int feeder_compare_by_score(const void *a,const void *b ) {
	return (wlrank[(*((const docid_t *)a))] <=  wlrank[(*((const docid_t *)b))]);
}

int feed_program(docid_t docid, char *buffer, score_t pagerank, score_t wlrank, score_t cscore, feeder_format_t format) {
    if (format == FEEDER_FORMAT_SWISHE) {
        return feed_swish(docid, buffer, pagerank, wlrank, cscore);
    }
    else if (format == FEEDER_FORMAT_LUCENE) {
        return feed_lucene(docid, buffer, pagerank, wlrank, cscore);
    }
    return -1;
}

int feed_lucene(docid_t docid, char *buf, score_t pagerank, score_t wlrank, score_t cscore){
    assert(docid > 0);
    storage_status_t rc;
	char url[MAX_STR_LEN];
	//char filename[MAX_STR_LEN];
	off64_t size;
	rc = storage_read( storage, docid, buf, &(size) );
	if(rc != STORAGE_OK){
		cerr << "x";
		return 0;
	}
    urlidx_url_by_docid( urlidx, docid, url );

	cout << "Path-Name: " << url << endl;
	cout << "Content-Length: " << strlen(buf)+1 << endl;
        //cout << "Last-Mtime: " << << endl;
	cout << "Document-Type: HTML" << endl;
	cout << "Pagerank: " << pagerank << endl;
    cout << "Wlrank: " << wlrank << endl;
    cout << "Cscore: " << cscore << endl;
	cout << endl;
	cout << buf << endl;
	cerr << ".";
	return 1;
}


int feed_swish(docid_t docid, char *buf, score_t pagerank, score_t wlrank, score_t cscore){
    assert(docid > 0);
    storage_status_t rc;
	char url[MAX_STR_LEN];
	//char filename[MAX_STR_LEN];
	char metas[MAX_STR_LEN];
	off64_t size;
	rc = storage_read( storage, docid, buf, &(size) );
	if(rc != STORAGE_OK){
		cerr << "x";
		return 0;
	}
    urlidx_url_by_docid( urlidx, docid, url );
	sprintf(metas, "<meta name=pagerank content=\"%d\"><meta name=wlrank content=\"%d\"><meta name=cscore content=\"%d\">", (int)pagerank, (int)wlrank, (int)cscore);

	cout << "Path-Name: " << url << endl;
	cout << "Content-Length: " << strlen(metas)+strlen(buf)+1 << endl;
        //cout << "Last-Mtime: " << << endl;
	cout << "Document-Type: HTML" << endl;
	cout << endl;
	cout << metas << buf << endl;
	cerr << ".";
	return 1;
}


void feed_idx(docid_t docid, docid_t count){
    assert(docid > 0);
    storage_status_t rc;
	char buf[MAX_DOC_LEN];
	char url[MAX_STR_LEN];
	char filename[MAX_STR_LEN];
	off64_t size;
	rc = storage_read( storage, docid, buf, &(size) );
	if(rc != STORAGE_OK){
		return;
	}
    urlidx_url_by_docid( urlidx, docid, url );
	sprintf(filename, "%s/%010d_%010d",COLLECTION_INDEX, (int)count, (int)docid);
	//cout << filename << endl;
	int file = creat(filename, S_IRWXU);
	write(file, buf, strlen(buf));
	close(file);
}


//
// Name: cleanup
//
// Description: 
//   Closes files, indexes
//

void cleanup() {
	if( metaidx != NULL ) {
		metaidx_close( metaidx );
		cerr << "[metaidx] ";
	}
	if( urlidx != NULL ) {
		urlidx_close( urlidx );
		cerr << "[urlidx] ";
	}
	if( storage != NULL ) {
		storage_close( storage );
		cerr << "[storage] ";
	}
}

//
// Name: feeder_usage
//
// Description:
//   Prints an usage message, then stops
//

void feeder_usage() {
	cerr << "Usage: program --format FORMAT [OPTION]" << endl;
	cerr << "Extract documents to be indexed, this is invoked by" << endl;
	cerr << "the indexer program, you should not run it directly." << endl;
	cerr << endl;
	cerr << " --format FORMAT  Indexing format, only 'swish-e' and 'lucene' are currently supported" << endl;
	cerr << endl;
	cerr << " --from           First docid to index, default 1" << endl;
	cerr << " --to             Last docid to index, default ndocs" << endl;
	cerr << " --help           This help message" << endl;
	cerr << endl;
	wire_stop(0);
}
