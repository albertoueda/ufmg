
#include "linkidx.h"

//
// Name: linkidx_open
//
// Description:
//   opens or creates a link index in a directory
//
// Input:
//   dirname - name of the directory to be created
//
// Return:
//   the structure
//

linkidx_t *linkidx_open( const char *dirname, bool readonly ) {
	// Request memory
	linkidx_t *tmp = (linkidx_t *)malloc(sizeof(linkidx_t));
	assert( tmp != NULL );

	// Set parameters
	assert( strlen(dirname) < MAX_STR_LEN );
	strcpy( tmp->dirname, dirname );
	tmp->readonly		= readonly;

    // Check if there is main file
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s/%s", dirname, STORAGE_FILENAME_MAIN );
	FILE *file_main = fopen64( filename, "r" );
	if( file_main == NULL ) {
		cerr << "(linkidx_create) ";

		if( tmp->readonly ) {
			die( "Can't create linkidx in readonly mode" );
		}

		// Create, but do not check duplicates
		tmp->storage = storage_create( dirname, false );
	} else {
		tmp->storage = storage_open( dirname, tmp->readonly );
	}
	assert( tmp->storage != NULL );

	// Returns
	return tmp;
}


//
// Name: linkidx_remove
//
// Description:
//   Removes files in a link index
//
// Input:
//   dirname - the directory
//   

void linkidx_remove( const char *dirname ) {
	storage_remove( dirname );
}

//
// Name: linkidx_close
//
// Description:
//   Closes the index
//
// Input:
//   linkidx - the structure to be closed
//

void linkidx_close( linkidx_t *linkidx ) {
	assert( linkidx != NULL );
	assert( linkidx->storage != NULL );

	// Close storage
	storage_close( linkidx->storage );

	// Free
	free( linkidx );
}

//
// Name: linkidx_store
//
// Description:
//   Stores the links of a document
//
// Input:
//   linkidx - the structure
//   src - source of the link
//   dest[] - destinations of the link
//   outdegree - number of destinations
//
// Return:
//   status
//

linkidx_status_t linkidx_store( linkidx_t *linkidx, docid_t src_docid, out_link_t dest[], uint out_degree ) {
	assert( ! linkidx->readonly );
	// We don't check the count to be less than LINK_MAX_OUTDEGREE,
	// as in some cases we will not be bound to this limit.
	// However, to use the link analysis functions for pagerank and
	// the link dumper, it is necessary to respect the limit.

	// Check the links before storing
	for( uint i=0; i<out_degree;i++ ) {
		if( dest[i].dest >= CONF_COLLECTION_MAXDOC ) {
			cerr << "[linkidx: destination #" << i << " docid=" << dest[i].dest << " is larger than " << CONF_COLLECTION_MAXDOC << endl;
			return LINKIDX_ERROR;
		}
	}
	// Write
	storage_status_t rc = storage_write( linkidx->storage, src_docid, (char *)(dest), (sizeof(out_link_t)*out_degree), NULL, NULL);
	assert( rc == STORAGE_OK );

	return LINKIDX_OK;
}

//
// Name: linkidx_prepare_sequential_read
//
// Description:
//   Prepares the links file for sequential reading
//
// Input:
//   linkidx - the structure
//

void linkidx_prepare_sequential_read( linkidx_t *linkidx ) {
	storage_prepare_sequential_read( linkidx->storage );
}

// 
// Name: linkidx_retrieve, linkidx_retrieve_using_buffer
// 
// Description:
//   Retrieves the links of a document
//
// Input:
//   linkidx - the structure
//
// Output:
//   dest[] - destinations of links
//   outdegree - out degree
//
// Return:
//   status
// 


linkidx_status_t linkidx_retrieve_using_buffer( linkidx_t *linkidx, char *buffer, docid_t src_docid, out_link_t dest[], uint *out_degree ) {

	// Read
	off64_t bytes;
	storage_status_t rc = storage_read( linkidx->storage, src_docid, buffer, &(bytes) );

	if( (rc == STORAGE_OK) && (bytes > 0) ) {
		// Get out_degree
		*out_degree = bytes/sizeof(out_link_t);

		memcpy( dest, buffer, bytes );

	} else {

		// No links
		*out_degree = 0;
	}

	// Return
	return LINKIDX_OK;
}

linkidx_status_t linkidx_retrieve( linkidx_t *linkidx, docid_t src_docid, out_link_t dest[], uint *out_degree ) {
	// Buffer
	char buffer[(sizeof(out_link_t) * LINK_MAX_OUTDEGREE) + 1];

	return linkidx_retrieve_using_buffer( linkidx, buffer, src_docid, dest, out_degree );
}


//
// Name: linkidx_show_links
//
// Description: 
//   Shows the links of a document
//

void linkidx_show_links( linkidx_t *linkidx, docid_t src_docid ) {
	// Report
	cout << "Links for docid " << src_docid << " : ";

	// Get links
	out_link_t dest[LINK_MAX_OUTDEGREE];
	uint out_degree;

	linkidx_retrieve( linkidx, src_docid, dest, &(out_degree) );

	// Show
	if( out_degree == 0 ) {
			cout << "none." << endl;
	} else {
		    cout << endl << "DESTID\tREL_POS\tTAG\tANCHOR_LENGTH" << endl;
			for( uint i=0; i<out_degree; i++ ) {
					cout << dest[i].dest << "\t" << (int)dest[i].rel_pos << "\t" << (int)dest[i].tag << "\t" << (int)dest[i].anchor_length << endl;
			}
	}
}

//
// Name: linkidx_dump_links
//
// Description: 
//   Dumps the links of a document, for analysis (in 1 line)
//

void linkidx_dump_links( linkidx_t *linkidx, docid_t src_docid ) {
	// Report
	cout << src_docid;

	// Get links
	out_link_t dest[LINK_MAX_OUTDEGREE];
	uint out_degree;

	linkidx_retrieve( linkidx, src_docid, dest, &(out_degree) );

	// Show
	if( out_degree > 0 ) {
		for( uint i=0; i<out_degree; i++ ) {
			cout << " " << dest[i].dest;
		}
	}
	cout << endl;
}

//
// Name: linkidx_dump_links_checking
//
// Description: 
//   Dumps the links of a document, for analysis (in 1 line)
//   receives an input with the docs it must show
//
// Input:
//   linkidx - the structure
//   docid - the source docid for the links
//   ok - a char array containing NULL for the documents that should not be shown
//

void linkidx_dump_links_checking( linkidx_t *linkidx, docid_t src_docid, char *ok ) {
	assert( ok[src_docid] != '\0' );

        // Report
        cout << src_docid;

        // Get links
        out_link_t dest[LINK_MAX_OUTDEGREE];
        uint out_degree;

        linkidx_retrieve( linkidx, src_docid, dest, &(out_degree) );

        // Show
        if( out_degree > 0 ) {
                for( uint i=0; i<out_degree; i++ ) {
                        if( ok[dest[i].dest] != '\0' ) {
                                cout << " " << dest[i].dest;
                        }
                }
        }
        cout << endl;
}

//
// Name: linkidx_dump_links_with_urls
//
// Description: 
//   Dumps the links of a document; includes destination urls
//
// Input:
//   linkidx - the link index
//   urlidx - index of urls
//   src_docid - the original docid
//

void linkidx_dump_links_with_url( linkidx_t *linkidx, urlidx_t *urlidx, docid_t src_docid ) {
	assert( linkidx != NULL );
	assert( urlidx != NULL );
	assert( src_docid > 0 );

	char url[MAX_STR_LEN]	= "";
	urlidx_url_by_docid( urlidx, src_docid, url );

	// Report
	cout << "Links from " << src_docid << " " << url << endl;

	// Get links
	out_link_t dest[LINK_MAX_OUTDEGREE];
	uint out_degree;

	linkidx_retrieve( linkidx, src_docid, dest, &(out_degree) );

	// Show
	if( out_degree > 0 ) {
		for( uint i=0; i<out_degree; i++ ) {
			urlidx_url_by_docid( urlidx, dest[i].dest, url );
			cout << "- " << dest[i].dest << " " << url << " " << TAG_STR(dest[i].tag) << endl;
		}
		cout << "Total: " << out_degree << " links" << endl;
	} else {
		cout << "No links" << endl;
	}
	cout << endl;
}
//
// Name: linkidx_dump_status
//
// Description:
//   Dumps the status of the link index
//
// Input:
//   the structure

void linkidx_dump_status( linkidx_t *linkidx ) {
	assert( linkidx != NULL );

	cerr << "Begin status dump for link index" << endl;
	cerr << "directory		" << linkidx->dirname << endl;
	storage_dump_status( linkidx->storage );
	cerr << "End status dump for link index" << endl;
}

//
// Name: linkidx_fix_depths
//
// Description:
//   Calculates the depth
//

void linkidx_fix_depths( linkidx_t *linkidx, metaidx_t *metaidx, urlidx_t *urlidx ) {
	assert( linkidx != NULL );
	assert( ! linkidx->readonly );
	depth_t *depth;

	cerr << "Fixing depths" << endl;

	cerr << "- Requesting memory ... ";
	// Depth=-1 => Depth undefined
	depth = (depth_t *)malloc(sizeof(depth_t)*(metaidx->count_doc + 1));
	assert( depth != NULL );
	cerr << "ok" << endl;

	cerr << "Initial depth is set as 1 for home pages." << endl;

	cerr << "- Setting initial depth |--------------------------------------------------|" << endl;
	cerr << "                         ";
	docid_t ndocs_div_50 = ((metaidx->count_doc)/50);
	for( docid_t i=1; i<=metaidx->count_doc; i++ ) {
		if( ndocs_div_50 > 0 && i % ndocs_div_50 == 0 ) {
			cerr << ".";
		}
		if( urlidx_is_homepage( urlidx, i ) ) {
			depth[i] = 1;
		} else {
			depth[i] = DEPTH_UNDEFINED;  // undefined depth
		}
	}
	cerr << "ok" << endl;

	cerr << "- Iterating ... ";
	depth_t current_depth = 1;

	out_link_t dest[LINK_MAX_OUTDEGREE+1];
	uint ndest;
	docid_t nmarked	= 0;
	do {

		assert( current_depth < DEPTH_UNDEFINED );

		// Check on each round if a page was found
		nmarked = 0;

		// Report
		cerr << "depth=" << current_depth << " ";

		for( docid_t i=1; i<=metaidx->count_doc; i++ ) {

			// Check depth
			if( depth[i] == current_depth ) {

				// Retrieve links
				linkidx_retrieve( linkidx, i, dest, &(ndest) );

				// Mark pointed pages as with more depth
				for( uint j=0; j<ndest; j++ ) {
					if( depth[dest[j].dest] == DEPTH_UNDEFINED ) {
						depth[dest[j].dest] = current_depth + 1;
					}
				}

				// Note that I found a page at this depth
				nmarked ++;
			}
		}

		cerr << "(" << nmarked << " pages), ";

		current_depth ++;
	} while( (nmarked > 0) && (current_depth < CONF_MANAGER_MAXDEPTH_STATIC) );
	cerr << "done calculating." << endl;

	cerr << "- Writing   |--------------------------------------------------|" << endl;
	cerr << "             ";
	doc_t doc;
	docid_t count_depth_undefined	= 0;
	for( doc.docid=1; doc.docid<=metaidx->count_doc; doc.docid++ ) {
		if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
			cerr << ".";
		}
		metaidx_doc_retrieve( metaidx, &(doc) );
		if( depth[doc.docid] != DEPTH_UNDEFINED ) {
			doc.depth = depth[doc.docid];
			metaidx_doc_store( metaidx, &(doc) );
		} else {
			count_depth_undefined ++;
		}
	}
	cerr << "done." << endl;

	cerr << "There were " << count_depth_undefined << " docs with undefined depth, I left them untouched" << endl;

	free( depth );

}

// 
// Name: linkidx_sort_by_score
//
// Description:
//   Sorts an array two documents based on their score,
//   score is a global variable that will point to
//   pagerank/wlrank/hub/authority score.
//
//   It will put the highest scores first.
//
//   This is used by quicksort while linearizing.
//
// Input:
//   ndocs - number of documents
//   thescore - score function to sort by, STARTING IN 1
//              i.e. score of document 42 is in score[42]
//  
// Output:
//   order - an array with the docids, STARTING IN 1,
//           i.e. order of document 42 is in order[42]
//

double *score	= NULL;

int linkidx_compare_by_score( const void *a, const void *b ) {
	assert( score != NULL );
	return( score[(*((const docid_t *)a))] >= score[(*((const docid_t *)b))] );
}

void linkidx_order_by_score( docid_t ndocs, docid_t *order, double *thescore ) {
	assert( order != NULL );
	assert( thescore != NULL );

	// Save the score function, it will be used by linkidx_compare_by_score
	score = thescore;
	
	// Initialize the order; it is necessary to start from
	// 0 because quicksort wants it that way
	for( docid_t i=1; i<=ndocs; i++ ) {
		order[i-1] = i;
	}

	// Quicksort
	qsort( order, ndocs, sizeof(docid_t), linkidx_compare_by_score );

	// Shift, so the order starts in 1
	for( docid_t i=ndocs; i>0; i-- ) {
		order[i] = order[i-1];
	}
}


//
// Name: linkidx_link_analysis
//
// Description:
//   Calculates the link-based statistics for all the documents
//
// Input:
//   linkidx - the link index structure
//   metaidx - the metaidx structure to write data
//   pagerank, wlrank, hits - link scores to analyze
//   linearize -
//     true  = set the values of the score uniformily between 0 and 1
//     false = save the actual values
//
//
// Returns:
//   status
//

linkidx_status_t linkidx_link_analysis( linkidx_t *linkidx, metaidx_t *metaidx,
		bool opt_calc_pagerank, bool opt_calc_wlrank, bool opt_calc_hits,
		bool opt_linearize ) {
	assert( linkidx != NULL );
	assert( metaidx != NULL );

	// Check if we were actually requested something
	assert( opt_calc_pagerank || opt_calc_wlrank || opt_calc_hits );

	docid_t ndocs	= metaidx->count_doc;

	// Declare vectors for analisis
	uint *in_degree = (uint *)malloc(sizeof(uint)*(ndocs+1));
	assert( in_degree != NULL );
	uint *out_degree = (uint *)malloc(sizeof(uint)*(ndocs+1));
	assert( out_degree != NULL );

	uint *in_degree_external = (uint *)malloc(sizeof(uint)*(ndocs+1));
	assert( in_degree_external != NULL );
	uint *out_degree_external = (uint *)malloc(sizeof(uint)*(ndocs+1));
	assert( out_degree_external != NULL );

	docid_t *siteid = (docid_t *)malloc(sizeof(docid_t)*(ndocs+1));
	assert( siteid != NULL );

	// Ask memory for pagerank calculation, only if necessary
	pagerank_t *pagerank = NULL;
	pagerank_t *pagerankTmp = NULL;
	if( opt_calc_pagerank ) {
		pagerank = (pagerank_t *)malloc(sizeof(pagerank_t)*(ndocs+1));
		assert( pagerank != NULL );

		pagerankTmp = (pagerank_t *)malloc(sizeof(pagerank_t)*(ndocs+1));
		assert( pagerankTmp != NULL );
	}

	// Ask memory for wlrank, only if necessary
	wlrank_t *wlrank	= NULL;
	wlrank_t *wlrankTmp	= NULL;
	linkweight_t *sum_out_weight = NULL;
	if( opt_calc_wlrank ) {
		sum_out_weight = (linkweight_t *)malloc(sizeof(linkweight_t)*(ndocs+1));
		assert( sum_out_weight != NULL );
		wlrank = (wlrank_t *)malloc(sizeof(wlrank_t)*(ndocs+1));
		assert( wlrank != NULL );
		wlrankTmp = (wlrank_t *)malloc(sizeof(wlrank_t)*(ndocs+1));
		assert( wlrankTmp != NULL );
	}

	// Ask memory for hubs and authorities, only if necessary
	hubrank_t *hub		= NULL;
	hubrank_t *hubTmp	= NULL;
	authrank_t *auth	= NULL;
	authrank_t *authTmp	= NULL;
	if( opt_calc_hits ) {
		hub = (hubrank_t *)malloc(sizeof(hubrank_t)*(ndocs+1));
		assert( hub != NULL );
		hubTmp = (hubrank_t *)malloc(sizeof(hubrank_t)*(ndocs+1));
		assert( hubTmp != NULL );

		auth = (authrank_t *)malloc(sizeof(authrank_t)*(ndocs+1));
		assert( auth != NULL );
		authTmp = (authrank_t *)malloc(sizeof(authrank_t)*(ndocs+1));
		assert( authTmp != NULL );
	}


	cerr << "Link analysis will calculate:"
		<< ( opt_calc_pagerank	? " Pagerank" : "" ) 
		<< ( opt_calc_wlrank ? " Wirescore" : "" ) 
		<< ( opt_calc_hits		? " Hits" : "" )  << endl;

	// Keep a list of ignored documents
	bool *is_ignored;
	is_ignored	= (bool *)malloc(sizeof(bool)*(ndocs+1));
	assert( is_ignored != NULL );
	uint ndocs_active	= 0;

	// Iterate to check for ignored documents
	doc_t doc;
	out_link_t dest[LINK_MAX_OUTDEGREE+1];
	uint ndest;
	docid_t ndocs_div_50 = (ndocs/50);

	cerr << "Preparing |--------------------------------------------------|" << endl;
    cerr << "           ";

	for( docid_t docid = 1; docid <= ndocs; docid ++ ) {
		doc.docid	= docid;
		metaidx_doc_retrieve( metaidx, &(doc) );
		assert( doc.docid == docid );

		if( ndocs_div_50 > 0 && (docid % ndocs_div_50 ) == 0 ) {
			cerr << ".";
		}

		// This is used when we want to mark some documents
		// as ignored in the collection
		if( HTTP_IS_OK( doc.http_status ) || HTTP_IS_REDIRECT( doc.http_status) ) {

			is_ignored[docid]	= false;
			ndocs_active		++;

		} else {
			is_ignored[docid]	= true;
		}

		// We read siteids for applying the heuristic of
		// discarding internal links (inside the same host)
		siteid[docid]			= doc.siteid;
	}

	// Initialize local vars
	// We have to do this loop again
	// because we don't know the number of active
	// documents until we read the entire metadata
	for( docid_t docid = 1; docid <= ndocs; docid ++ ) {

		in_degree[docid] = 0;
		out_degree[docid] = 0;

		if( opt_calc_pagerank ) {
			if( is_ignored[docid] ) {
				pagerank[docid]	= 0;
			} else {
				pagerank[docid] = (pagerank_t)((pagerank_t)LINK_NORMALIZATION/(pagerank_t)ndocs_active);
			}
		}

		if( opt_calc_wlrank ) {
			sum_out_weight[docid] = 0;
			if( is_ignored[docid] ) {
				wlrank[docid] = 0;
			} else {
				wlrank[docid] = (wlrank_t)((wlrank_t)1/(wlrank_t)ndocs_active);
			}
		}

		if( opt_calc_hits ) {
			in_degree_external[docid]	= 0;
			out_degree_external[docid]	= 0;

			if( is_ignored[docid] ) {
				hub[docid]	= 0;
				auth[docid]	= 0;
			} else {
				hub[docid]	= ((hubrank_t)1);
				auth[docid]	= ((authrank_t)1);
			}
		}
	}
	cerr << endl;

	// Iterate to calculate degrees
	cerr << "Calculating degree |--------------------------------------------------|" << endl;
    cerr << "                    ";
	linkidx_prepare_sequential_read( linkidx );
	for( docid_t docid = 1; docid<=ndocs; docid++ ) {

		if( ndocs_div_50 > 0 && (docid % ndocs_div_50 ) == 0 ) {
			cerr << ".";
		}

		// Get links. Always get links BEFORE skipping
		linkidx_sequential_read( linkidx, dest, &(ndest) );

		// See if it's necessary to skip
		if( is_ignored[docid] ) {
			continue;
		}

		// Calculate in degree and out degree
		for(uint j=0; j<ndest;j++ )
		{
			// Avoid self-links or links to ignored documents
			if( (dest[j].dest != docid) && (!is_ignored[dest[j].dest]) ) {
				in_degree[dest[j].dest]	++;
				out_degree[docid]		++;

				// Calculate internal and external links 
				// between documents in different hosts
				if( opt_calc_hits ) {
					if( siteid[dest[j].dest] != siteid[docid] ) {
						in_degree_external[dest[j].dest]	++;
						out_degree_external[docid]			++;
					}
				}

				// Calc weight for normalization
				if( opt_calc_wlrank ) {
					sum_out_weight[docid]+=calculate_link_weight(dest[j]);
				}
			}

		}
	}
	cerr << "done." << endl;

	if( opt_calc_pagerank || opt_calc_hits || opt_calc_wlrank ) {

		// Calculate q and 1-q based on the dampening factor
		double omq	= (double)CONF_MANAGER_SCORE_PAGERANK_DAMPENING;
		double q	= (double)1 - omq;

		cerr << "* Calculating link scores (iterative)" << endl;
		cerr << "  q=" << q << endl;

		// Iterate to calculate pagerank
		bool pagerank_delta_ok	= ( opt_calc_pagerank || opt_calc_wlrank ) ? false : true;
		bool hits_delta_ok		= ( opt_calc_hits ) ? false : true;
		uint iteration=0;

		while( ! (pagerank_delta_ok && hits_delta_ok) ) {

			iteration++;
			cerr << " - Iteration " << iteration << " ";

			// Check iteration
			if( iteration > LINK_SCORE_MAX_ITERATIONS ) {
				die( "Too many iterations without convergence" );
			}

			// Clear TMP
			for( docid_t i = 1; i<=ndocs; i++ ) {
				if( opt_calc_pagerank ) {
					pagerankTmp[i]	= (pagerank_t)0;
				}
				if( opt_calc_wlrank ) {
					wlrankTmp[i]	= (wlrank_t)0;
				}
				if( opt_calc_hits ) {
					authTmp[i]		= (authrank_t)0;
					hubTmp[i]		= (hubrank_t)0;
				}
			}

			// Iteration
			linkidx_prepare_sequential_read( linkidx );
			for( docid_t i=1; i<=ndocs; i++ ) {

				// Skip empty lists
				if( out_degree[i] == 0 || is_ignored[i] ) {
					uint skip	= 1;
					i++;
					while( i<=ndocs && (out_degree[i] == 0 || is_ignored[i]) ) {
						if( ndocs_div_50 > 0 && (i % ndocs_div_50 ) == 0 ) {
							cerr << ".";
						}
						skip++;
						i++;
					}

					// Skip a number of items
					linkidx_sequential_skip( linkidx, skip );

					// See if it is necessary to get out of the cycle
					if( i>ndocs ) {
						continue;
					}
				}

				// Read (sequential)
				linkidx_sequential_read( linkidx, dest, &(ndest) );

				if( ndocs_div_50 > 0 && (i % ndocs_div_50 ) == 0 ) {
					cerr << ".";
				}

				// Check the destinations of this link
				for( uint j=0; j<ndest; j++ ) {

					// If this is not a self-link, and the destination document is not ignored
					if( (dest[j].dest != i) && (! is_ignored[dest[j].dest]) ) {
						assert( out_degree[i] > 0 );

						if( opt_calc_pagerank && (!pagerank_delta_ok) ) {
							pagerankTmp[dest[j].dest]+= pagerank[i]/(pagerank_t)(out_degree[i]);
						}

						if( opt_calc_wlrank && (!pagerank_delta_ok) ) {
							assert( sum_out_weight[i] > 0 );
							wlrankTmp[dest[j].dest]+= wlrank[i]*calculate_link_weight(dest[j])/sum_out_weight[i];
						}

						if( opt_calc_hits && (!hits_delta_ok) ) {
							// Apply Bharat's heuristic: don't count
							// links inside the same host
							if( siteid[dest[j].dest] != siteid[i] ) {
								authTmp[dest[j].dest]	+= hub[i]/(authrank_t)(out_degree_external[i]);
							}
						}
					}
				}
			}

			if( opt_calc_hits && (!hits_delta_ok) ) {

				// Make a second iteration for HITS
				linkidx_prepare_sequential_read( linkidx );

				for( docid_t i=1; i<=ndocs; i++ ) {

					// Skip when possible
					if( out_degree[i] == 0 || is_ignored[i] ) {
						uint skip	= 1;
						i++;
						while( i<=ndocs && (out_degree[i] == 0 || is_ignored[i]) ) {
							if( ndocs_div_50 > 0 && (i % ndocs_div_50 ) == 0 ) {
								cerr << ".";
							}
							skip++;
							i++;
						}

						// Skip a number of items
						linkidx_sequential_skip( linkidx, skip );

						// See if it is necessary to get out of the cycle
						if( i>ndocs ) {
							continue;
						}
					}

					// Read (sequential)
					linkidx_sequential_read( linkidx, dest, &(ndest) );

					if( is_ignored[i] ) {
						// If this document is ignored, continue
						// NOTE that it is important to make the sequential read anyways
						continue;
					}

					// Check the destinations of this link
					for( uint j=0; j<ndest; j++ ) {

						// If this is not a self-link, and the destination document is not ignored
						if( (dest[j].dest != i) && (! is_ignored[dest[j].dest]) ) {

							// Apply Bharat's heuristic: don't count
							// links inside the same host
							if( siteid[dest[j].dest] != siteid[i] ) {
								// Note here that we do the iteration directly, we don't normalize before
								// doing the hub part
								hubTmp[i]				+= authTmp[dest[j].dest]/(hubrank_t)(in_degree_external[dest[j].dest]);
							}
						}
					}
				}
			}

			// Normalize TMP and COPY
			pagerank_t sum_pageranks = 0;
			wlrank_t sum_wlranks = 0;
			hubrank_t sum_hubs = 0;
			authrank_t sum_auths = 0;

			for( docid_t i=1; i<=ndocs; i++ ) {

				if( is_ignored[i] ) {
					continue;
				}

				if( opt_calc_pagerank && (!pagerank_delta_ok)) {
					pagerankTmp[i] = (q/(double)ndocs_active*LINK_NORMALIZATION)
					   			   + ((omq)*pagerankTmp[i]);
					sum_pageranks += pagerankTmp[i];
				}

				if( opt_calc_wlrank && (!pagerank_delta_ok) ) {
					wlrankTmp[i] = (q/(double)ndocs_active*LINK_NORMALIZATION) + (omq*wlrankTmp[i]);
					sum_wlranks += wlrankTmp[i];
				}

				if( opt_calc_hits && (!hits_delta_ok)) {
					sum_auths += authTmp[i];
					sum_hubs += hubTmp[i];
				}
			}

			// Quadratic errors
			double	pagerank_delta		= 0;
			double	wlrank_delta		= 0;
			double	hubrank_delta		= 0;
			double	authrank_delta		= 0;
			pagerank_t	pagerank_aux	= 0;
			wlrank_t	wlrank_aux		= 0;
			hubrank_t	hubrank_aux		= 0;
			authrank_t	authrank_aux	= 0;
			for( docid_t i=1; i<=ndocs; i++ ) {

				if( is_ignored[i] ) {
					continue;
				}

				if( opt_calc_pagerank && (!pagerank_delta_ok)) {
					pagerank_aux = (LINK_NORMALIZATION*pagerankTmp[i])/sum_pageranks;
					pagerank_delta	+= linkidx_calc_delta( pagerank_aux, pagerank[i] );
					pagerank[i]	= pagerank_aux;
				}

				if( opt_calc_wlrank && (!pagerank_delta_ok) ) {
					wlrank_aux		= (LINK_NORMALIZATION*wlrankTmp[i])/sum_wlranks;
					wlrank_delta	+= linkidx_calc_delta( wlrank_aux, wlrank[i] );
					wlrank[i]		= wlrank_aux;
				}

				if( opt_calc_hits && (!hits_delta_ok)) {
					authrank_aux	= (LINK_NORMALIZATION*authTmp[i])/sum_auths;
					authrank_delta	+= linkidx_calc_delta( authrank_aux, auth[i] );
					auth[i]			= authrank_aux;

					hubrank_aux		= (LINK_NORMALIZATION*hubTmp[i])/sum_hubs;
					hubrank_delta	+= linkidx_calc_delta( hubrank_aux, hub[i] );
					hub[i]			= hubrank_aux;
				}
			}
			cerr << " done." << endl;


			// Check delta for pagerank and wlrank
			bool both_ok	= true;
			if( opt_calc_pagerank && (!pagerank_delta_ok) ) {
				pagerank_delta	/= ndocs_active;
				cerr << "    pagerank average error       = " << pagerank_delta << " (target=" << CONF_MANAGER_SCORE_PAGERANK_MAX_ERROR << ")" << endl;
				cerr << "    pagerank sum values          = " << sum_pageranks << endl;
				if( pagerank_delta > CONF_MANAGER_SCORE_PAGERANK_MAX_ERROR ) {
					both_ok	= false;
				}
			}
			if( opt_calc_wlrank && (!pagerank_delta_ok) ) {
				wlrank_delta	/= ndocs_active;
				cerr << "    wlrank average error         = " << wlrank_delta << " (target=" << CONF_MANAGER_SCORE_PAGERANK_MAX_ERROR << ")" << endl;
				cerr << "    wlrank sum values            = " << sum_wlranks << endl;
				if( wlrank_delta > CONF_MANAGER_SCORE_PAGERANK_MAX_ERROR ) {
					both_ok	= false;
				}
			}
			if( both_ok ) {
				pagerank_delta_ok	= true;
			}

			// Check hits
			if( opt_calc_hits && (!hits_delta_ok) ) {
				hubrank_delta	/= ndocs_active;
				cerr << "    hubrank average error        = " << hubrank_delta << " (target=" << CONF_MANAGER_SCORE_HITS_MAX_ERROR << ")" << endl;
				cerr << "    hubrank sum values           = " << sum_hubs << endl;
				authrank_delta	/= ndocs_active;
				cerr << "    authrank average error       = " << authrank_delta << " (target=" << CONF_MANAGER_SCORE_HITS_MAX_ERROR << ")" << endl;
				cerr << "    authrank sum values          = " << sum_auths << endl;

				if( hubrank_delta < CONF_MANAGER_SCORE_HITS_MAX_ERROR 
				 && authrank_delta < CONF_MANAGER_SCORE_HITS_MAX_ERROR ) {
					hits_delta_ok = true;
				}
			}

		}

	}

	if( opt_linearize ) {
		cerr << "Converting to uniformly-distributed values between 0 and 1:" << endl;

		// Space for ordering; I don't need to initialize this before
		// every call, as it is initialized by linkidx_order_by_score
		docid_t *order;
		order	= (docid_t *)malloc(sizeof(docid_t) * (ndocs+1));
		assert( order != NULL );

		// Pagerank
		if( opt_calc_pagerank ) {
			cerr << " - Pagerank ";

			cerr << "ordering, ";
			assert( sizeof(pagerank_t) == sizeof(double) );
			linkidx_order_by_score( ndocs, order, (double *)pagerank );

			cerr << "assigning, ";
			double step				= (double)1/(double)ndocs_active;
			double current_value	= step;
			for( docid_t i=1; i<=ndocs; i++ ) {
				if( is_ignored[order[i]] ) {
					pagerank[order[i]]	= 0;
				} else {
					pagerank[order[i]]	= current_value;
					current_value		+= step;
				}
			}

			cerr << "done." << endl;
		}

		// Pagerank
		if( opt_calc_wlrank ) {
			cerr << " - WLrank ";

			cerr << "ordering, ";
			assert( sizeof(wlrank_t) == sizeof(double) );
			linkidx_order_by_score( ndocs, order, (double *)wlrank );

			cerr << "assigning, ";
			double step				= (double)1/(double)ndocs_active;
			double current_value	= step;
			for( docid_t i=1; i<=ndocs; i++ ) {
				if( is_ignored[order[i]] ) {
					wlrank[order[i]]	= 0;
				} else {
					wlrank[order[i]]	= current_value;
					current_value		+= step;
				}
			}

			cerr << "done." << endl;
		}

		// Hits
		if( opt_calc_hits ) {
			cerr << " - Hits ";

			cerr << "ordering by hub, ";
			assert( sizeof(hubrank_t) == sizeof(double) );
			linkidx_order_by_score( ndocs, order, (double *)hub );

			// For the hits algorithm, there might be many
			// documents with score=0.
			// For pagerank/wlrank, all documents have a positive score,
			// or at least a base score.
			docid_t hub_gt_zero		= 0;
			docid_t auth_gt_zero	= 0;	
			for( docid_t i=1; i<=ndocs; i++ ) {
				if( hub[i] > 0 ) {
					hub_gt_zero ++;
				} 
				if( auth[i] > 0 ) {
					auth_gt_zero ++;
				}
			}

			cerr << "assigning, ";
			double step				= (double)1/(double)hub_gt_zero;
			double current_value	= step;
			if( hub_gt_zero > 0 ) {
				for( docid_t i=1; i<=ndocs; i++ ) {
					if( is_ignored[order[i]] ) {
						hub[order[i]]	= 0;
					} else if( hub[order[i]] > 0 ) {
						hub[order[i]]	= current_value;
						current_value		+= step;
					}
				}
			}
			cerr << "done." << endl;

			cerr << " - Authority ";

			cerr << "ordering by authority, ";
			assert( sizeof(authrank_t) == sizeof(double) );
			linkidx_order_by_score( ndocs, order, (double *)auth );

			cerr << "assigning, ";
			step				= (double)1/(double)auth_gt_zero;
			current_value		= step;
			if( auth_gt_zero > 0 ) {
				for( docid_t i=1; i<=ndocs; i++ ) {
					if( is_ignored[order[i]] ) {
						auth[order[i]]	= 0;
					} else if( auth[order[i]] > 0 ) {
						auth[order[i]]	= current_value;
						current_value		+= step;
					}
				}
			}
			cerr << "done." << endl;
		}

		// Free memory
		free(order);
	}

	cerr << "Writing   |--------------------------------------------------|" << endl;
    cerr << "           ";

	// Write metadata
	for( doc.docid = 1; doc.docid<=ndocs; doc.docid++ ) {

		// Retrieve
		metaidx_doc_retrieve( metaidx, &(doc) );
		assert( doc.docid > 0 );

		if( is_ignored[doc.docid] ) {
			if( opt_calc_pagerank ) {
				assert( pagerank[doc.docid] == 0 );
			}
			if( opt_calc_wlrank ) {
				assert( wlrank[doc.docid] == 0 );
			}
			if( opt_calc_hits ) {
				assert( hub[doc.docid] == 0 );
				assert( auth[doc.docid] == 0 );
			}
		}

		// Copy values
		if( opt_calc_pagerank ) {
			doc.pagerank	= pagerank[doc.docid];
		}

		if( opt_calc_wlrank ) {
			doc.wlrank     = wlrank[doc.docid];
		}

		if( opt_calc_hits ) {
			doc.hubrank     = hub[doc.docid];
			doc.authrank    = auth[doc.docid];
		}

		// Always write in/out degree also
		doc.in_degree	= in_degree[doc.docid];
		doc.out_degree	= out_degree[doc.docid];

		// Store
		metaidx_doc_store( metaidx, &(doc) );
	}
	cerr << "done." << endl;

	// Free
	free(in_degree);
	free(out_degree);

	free(in_degree_external);
	free(out_degree_external);

	if( opt_calc_pagerank ) {
		free(pagerank);
		free(pagerankTmp);
	}

	if( opt_calc_wlrank ) {
		free(sum_out_weight);
		free(wlrank);
		free(wlrankTmp);
	}

	if( opt_calc_hits ) {
		free(hub);
		free(hubTmp);
		free(auth);
		free(authTmp);
	}

	free(is_ignored);
	free(siteid);

	return LINKIDX_OK;

}

linkweight_t calculate_link_weight( out_link_t link){
    linkweight_t base = (linkweight_t)1;
    linkweight_t rel_pos_factor = (linkweight_t)1;
    linkweight_t tag_b_value = (linkweight_t)1;
    linkweight_t tag_h1_value = (linkweight_t)1;
    linkweight_t anchor_length_base = (linkweight_t)1;
    linkweight_t anchor_length_average = (linkweight_t)100;

	
	linkweight_t weight=base;


	// Contribution form the relative position of the link in the src document
	
	weight+=rel_pos_factor * (linkweight_t)((double)link.rel_pos/100.0);

    // Contribution from the tag where the link is embedded
	
	if(link.tag==TAG_B)
		weight+=tag_b_value;
	else if(link.tag==TAG_H1)
		weight+=tag_h1_value;

	// Contribution from the length or the anchor of the link
	
	weight+=anchor_length_base * (linkweight_t)link.anchor_length/anchor_length_average;
	return weight;
}
