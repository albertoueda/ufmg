
#include "manager.h"

//
// Name: main
//
// Description:
//   Manager program, creates the harvests
//

int main( int argc, char **argv ) {
	wire_start( "manager" );

	// Parse options
	bool opt_dont_generate	= false;
	bool opt_cancel			= false;
	bool opt_exclude		= false; char opt_exclude_site[MAX_STR_LEN];
	bool opt_verify_only	= false;

	while(1) {
		int option_index = 0;

		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"dont-generate", 0, 0, 0},
			{"verify-only", 0, 0, 0},
			{"cancel", 0, 0, 0},
			{"exclude", 1, 0, 0},
			{0, 0, 0, 0}
		};

		char c = getopt_long (argc, argv, "hcv",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 0:
				if( !strcmp( long_options[option_index].name,
							"dont-generate" ) ) {
					opt_dont_generate = true;
				} else if( !strcmp( long_options[option_index].name,
							"cancel" ) ) {
					opt_cancel = true;
				} else if( !strcmp( long_options[option_index].name,
							"verify-only" ) ) {
					opt_verify_only = true;
				} else if( !strcmp( long_options[option_index].name,
							"exclude" ) ) {
					opt_exclude = true;
					strcpy( opt_exclude_site, optarg );
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					manager_usage();
				}
				break;
			case 'c':
				opt_cancel = true;
				break;
			case 'v':
				opt_verify_only = true;
				break;
			case 'h':
				manager_usage();
				break;
			default:
				manager_usage();
		}
	}

	// Open all indexes
	cerr << "Open indexes ... ";
	manager_open_indexes();
	cerr << "done." << endl;

	// Get ndocs
	ndocs = metaidx->count_doc;
	if( ndocs == 0 ) {
		die( "Metaindex empty, execute seeder first" );
	}
	nsites = metaidx->count_site;
	assert( nsites > 0 );


	// Verify only
	if( opt_verify_only ) {
		cerr << "Verifying documents only" << endl;
		bool everything_ok = true;

		docid_t ndocs_div_50 = ndocs / 50;
		cerr << "Marking docs   |--------------------------------------------------|" << endl;
		cerr << "               ";
		docid_t docid;
		doc_t doc;
		for( docid=1; docid<=ndocs; docid++ ) {

			doc.docid = docid;

			if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
				cerr << ".";
			}

			// Read the document
			metaidx_doc_retrieve( metaidx, &(doc) );
			if( doc.docid != docid ) {
				cerr << "DocID " << docid << " is damaged, appears as " << doc.docid << endl;
				everything_ok = false;
			}

		}
		if( everything_ok ) {
			wire_stop(0);
		} else {
			die( "There were errors found during the verification" );
		}
	}

	// Add exclusion
	if( opt_exclude == true ) {
		cerr << "Excluding all URLs from Website " << opt_exclude_site << endl;
		urlidx_status_t rc;
		siteid_t siteid;
		
		rc = urlidx_resolve_site( urlidx, opt_exclude_site, NULL );
		if( rc == URLIDX_NOT_FOUND ) {
			cerr << "Site named '" << opt_exclude_site << "' was not found" << endl;
			die( "Site not found" );
		}

		rc = urlidx_resolve_site( urlidx, opt_exclude_site, &(siteid) );

		cerr << "Siteid is " << siteid << endl;

		docid_t ndocs_div_50 = ndocs / 50;
		cerr << "Marking docs   |--------------------------------------------------|" << endl;
		cerr << "               ";
		docid_t docid;
		doc_t doc;
		for( docid=1; docid<=ndocs; docid++ ) {

			doc.docid = docid;

			if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
				cerr << ".";
			}

			// Read the document
			metaidx_doc_retrieve( metaidx, &(doc) );
			assert( doc.docid == docid );
			

			// If the document is from this Website, mark as excluded
			if( doc.siteid == siteid ) {
				doc.status = STATUS_DOC_EXCLUSION;
				metaidx_doc_store( metaidx, &(doc) );
			}
		}
		wire_stop(0);
	}

	// Check if requested to cancel
	if( opt_cancel ) {
		harvest_t *harvester = NULL;
		for( int i=1; harvest_exists( COLLECTION_HARVEST, i ); i++ ) {

			// Open in readonly mode
			harvester = harvest_open( COLLECTION_HARVEST, i, true );
			cerr << "Checking harvester #" << harvester->id << " ... ";
			if( harvester->status != STATUS_HARVEST_SEEDED ) {

				// Close and remove
				harvest_close( harvester );
				harvest_remove( COLLECTION_HARVEST, i );
				cerr << " canceled, removed." << endl;
			} else {
				// Just close
				harvest_close( harvester );
				cerr << " ok." << endl;
			}
		}

		// Sites are marked with the harvesterid that has pages from
		// it, to avoid having two harvester in the same site.
		siteid_t nsites_div_50 = metaidx->count_site / 50;
		site_t site;
		cerr << "Marking sites   |--------------------------------------------------|" << endl;
		cerr << "                 ";
		for( site.siteid=1; site.siteid <= metaidx->count_site; site.siteid++ ) {

			// Report
			if( nsites_div_50 > 0 && site.siteid % nsites_div_50 == 0 ) {
				cerr << ".";
			}

			metaidx_site_retrieve( metaidx, &(site) );
			site.harvest_id = 0;
			metaidx_site_store( metaidx, &(site) );
		}
		cerr << " ok" << endl;

		// Assigned documents are marked as new
		docid_t ndocs_div_50 = metaidx->count_doc / 50;
		doc_t doc;
		cerr << "Marking docs    |--------------------------------------------------|" << endl;
		cerr << "                 ";
		for( doc.docid=1; doc.docid <= metaidx->count_doc; doc.docid++ ) {

			// Report
			if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
				cerr << ".";
			}

			metaidx_doc_retrieve( metaidx, &(doc) );
			if( doc.status == STATUS_DOC_ASSIGNED ) {
				// I am marking the document as new
				// It might have been already crawled and it was here because it was going
				// to be re-crawled, but we don't know it.
				doc.status = STATUS_DOC_NEW;
				metaidx_doc_store( metaidx, &(doc) );
			}

		}
		cerr << " ok" << endl;

		// OK
		wire_stop(0);
	}

	// Calculate link scores only if needed
	if( CONF_MANAGER_SCORE_PAGERANK_WEIGHT > 0
	 || CONF_MANAGER_SCORE_WLSCORE_WEIGHT > 0 
	 || CONF_MANAGER_SCORE_HITS_HUB_WEIGHT > 0
	 || CONF_MANAGER_SCORE_HITS_AUTHORITY_WEIGHT > 0 ) {

		bool calc_pagerank	= false;
		bool calc_wlrank	= false;
		bool calc_hits		= false;
		if( CONF_MANAGER_SCORE_PAGERANK_WEIGHT > 0 ) {
			calc_pagerank	= true;
		}
		if( CONF_MANAGER_SCORE_WLSCORE_WEIGHT > 0 ) {
			calc_wlrank		= true;
		}
		if( CONF_MANAGER_SCORE_HITS_HUB_WEIGHT > 0
		 || CONF_MANAGER_SCORE_HITS_AUTHORITY_WEIGHT > 0 ) {
			calc_hits		= true;
		}

		cerr << "* Performing link analysis on documents" << endl;

		linkidx_link_analysis( linkidx, metaidx,
				calc_pagerank, calc_wlrank, calc_hits,
				true );
	}

	// Calculate link site scores if needed
	if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0
	 || CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 ) {
		cerr << "* Generating statistics about sites" << endl;
		metaidx_analysis_site_statistics( metaidx );

	}

	if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0 ) {

		sitelinkidx_t	*sitelinkidx	= NULL;

		cerr << "* Generating site links structure" << endl;
		sitelink_create( COLLECTION_SITELINK, linkidx, metaidx );
		sitelinkidx = sitelink_load( COLLECTION_SITELINK );

		cerr << "* Calculating linearized siterank" << endl;
		sitelink_analysis_siterank( sitelinkidx, metaidx, true, true, true );

		free( sitelinkidx );
	}

	// Calculate scores
	manager_calculate_scores();

	// Check if scores_only
	if( opt_dont_generate ) {
		cerr << "Future and current scores calculated. Stop." << endl;
		wire_stop(0);
	}

	// Sort documents by predicted scores
	cerr << "Ordering documents by priority ... ";
	manager_order_documents();
	cerr << "done." << endl;

	// Copy priority=-1 -> order[] = 0;
	cerr << "Freeing some memory ... ";
	for( docid_t i=1; i<=ndocs; i++ ) {
		if( priority[order[i-1] - 1] == (priority_t)(-1) ) {
			order[i-1] = 0;
		}
	}
	free( priority );
	cerr << "done." << endl;
	priority = NULL;

	// Show number of batches and document in each batch
	cerr << "Batch size: " << CONF_MANAGER_BATCH_SIZE << endl;
	cerr << "Creating " << CONF_MANAGER_BATCH_COUNT << " batchs" << endl;

	// Create the batches
	for( uint harvest_count = 1; harvest_count <= CONF_MANAGER_BATCH_COUNT; harvest_count++) {

		// Create harvester
		cerr << "- Create harvester ... ";
		harvest_t *harvest = harvest_create( COLLECTION_HARVEST );
		cerr << harvest->id << " done." << endl;

		// Set parameters for the harvester
		harvest->status = STATUS_HARVEST_ASSIGNED;
		harvest->creationtime = time(NULL);

		// Create the harvest list
		manager_create_harvest_list( harvest );

		if( harvest->count == 0 ) {
			cerr << "Failed while creating harvest list ... ";
			int failed_id = harvest->id;

			harvest_close( harvest );
			harvest_remove( COLLECTION_HARVEST, failed_id );
			cerr << "removed." << endl;
			break;
		} else if( harvest->count > CONF_MANAGER_BATCH_SIZE ) {
			cerr << "Error!: too many documents" << endl;
			int failed_id = harvest->id;

			harvest_close( harvest );
			harvest_remove( COLLECTION_HARVEST, failed_id );
			cerr << "removed." << endl;
			break;
		}

		// Dump
		harvest_dump_status( harvest );

		// Log
		syslog( LOG_NOTICE, "manager created harvest #%d", harvest->id); 

		// Close
		assert( ! harvest->readonly );
		harvest_close( harvest );
	}

	// Stop
	wire_stop(0);
}

//
// Name: manager_create_harvest_list
//
// Description:
//   Creates a list of documents to be harvested, and assign
//   that list to the harvester
//
// Input:
//   harvest - Harvester to assign documents
//

void manager_create_harvest_list( harvest_t *harvest ) {
	doc_t doc;
	site_t site;
	char path[MAX_STR_LEN];
	char sitename[MAX_STR_LEN];
	harvest->count = 0;
	metaidx_status_t metaidx_status;

	manager_site_status_t *manager_site_status = NULL;
	manager_site_status = (manager_site_status_t *)malloc( sizeof(manager_site_status_t) * (nsites + 1) );
	assert( manager_site_status != NULL );

	// I want to be sure this is reasonably sized and also completely sure
	// that it fits in a uint. 100 million pages in the same batch from the
	// same site would be completely wrong
	assert( CONF_MANAGER_BATCH_SAMESITE < 100000000 );
	uint *site_count = NULL;
	site_count = (uint *)malloc( sizeof(uint) * (nsites + 1) );
	assert( site_count != NULL );

	depth_t max_depth = CONF_MANAGER_MAXDEPTH_DYNAMIC > CONF_MANAGER_MAXDEPTH_STATIC ? CONF_MANAGER_MAXDEPTH_DYNAMIC + 1 : CONF_MANAGER_MAXDEPTH_STATIC + 1;

	uint *depth_count = (uint *)malloc(sizeof(uint)*(max_depth+1));
	assert( depth_count != NULL );
	for( depth_t depth = 0; depth <= max_depth; depth++ ) {
		depth_count[depth] = 0;
	}

	siteid_t saturated_sites	= 0;
	siteid_t count_sites		= 0;
	siteid_t count_erroneous_sites	= 0;

	// Clear variables
	for( siteid_t siteid=1; siteid<=nsites; siteid++ ) {
		site_count[siteid] = 0;
		manager_site_status[siteid] = MANAGER_SITE_STATUS_AVAILABLE;
	}

	// Iterate through sites to check for robots.txt and robots.rdf
	cerr << "Scheduling special files such as robots.txt" << endl;
	cerr << "Checking sites |--------------------------------------------------|" << endl;
	cerr << "                ";
	docid_t nsites_div_50 = nsites / 50;

	time_t	now	= time(NULL);
	for( siteid_t siteid=1; siteid<=nsites; siteid++ ) {
		site.siteid	= siteid;

		// Report
		if( nsites_div_50 > 0 && siteid % nsites_div_50 == 0 ) {
			cerr << ".";
		}

		// Check if too many, maybe even checking special files
		// we already have enough pages for a batch
		if( harvest->count >= CONF_MANAGER_BATCH_SIZE ) {
			break;
		}

		// Retrieve the site
		metaidx_status = metaidx_site_retrieve( metaidx, &(site) );
		assert( metaidx_status == METAIDX_OK );

		// Get period length
		time_t period_robots_txt = site.has_valid_robots_txt
			?	CONF_MANAGER_MINPERIOD_ROBOTS_TXT_VALID 
			:	CONF_MANAGER_MINPERIOD_ROBOTS_TXT_NOT_VALID;

		time_t period_robots_rdf = site.has_valid_robots_rdf
			?	CONF_MANAGER_MINPERIOD_ROBOTS_RDF_VALID 
			:	CONF_MANAGER_MINPERIOD_ROBOTS_RDF_NOT_VALID;

		// Check if period has passed for robots.txt
		if( site.docid_robots_txt					> 0 &&
			(now - site.last_checked_robots_txt)	> period_robots_txt ) {

			// Get document for robots.txt
			doc.docid		= site.docid_robots_txt;
			metaidx_status	= metaidx_doc_retrieve( metaidx, &(doc) );
			assert( metaidx_status == METAIDX_OK );

			// Check if it has been assigned
			if( doc.status != STATUS_DOC_ASSIGNED ) {

				// Add to the queue
				doc.status = STATUS_DOC_ASSIGNED;
				metaidx_doc_store( metaidx, &(doc) );

				// Retrieve the url
				urlidx_path_by_docid( urlidx, doc.docid, path );
				assert( path != NULL );

				// Assign the document
				harvest_append_doc( harvest, &(doc), path );

				// Count
				site_count[doc.siteid] ++;
				depth_count[doc.depth] ++;

				// Check if its necessary to store the site also
				if( manager_site_status[siteid] != MANAGER_SITE_STATUS_CHECK_SPECIAL ) {

					// Read the sitename
					urlidx_site_by_siteid( urlidx, site.siteid, sitename );

					// Give information to the harvest
					harvest_append_site( harvest, &(site), sitename );

					// Count
					count_sites ++;
				}

				// Mark as special site
				manager_site_status[siteid]	= MANAGER_SITE_STATUS_CHECK_SPECIAL;

			}

		}

		// Check if period has passed for robots.rdf
		if( site.docid_robots_rdf					> 0 &&
			(now - site.last_checked_robots_rdf)	> period_robots_rdf ) {

			// Get document for robots.rdf
			doc.docid		= site.docid_robots_rdf;
			metaidx_status	= metaidx_doc_retrieve( metaidx, &(doc) );
			assert( metaidx_status == METAIDX_OK );

			// Check if it has been assigned
			if( doc.status != STATUS_DOC_ASSIGNED ) {


				// Add to the queue
				doc.status = STATUS_DOC_ASSIGNED;
				metaidx_doc_store( metaidx, &(doc) );

				// Retrieve the url
				urlidx_path_by_docid( urlidx, doc.docid, path );
				assert( path != NULL );

				// Assign the document
				harvest_append_doc( harvest, &(doc), path );

				// Count
				site_count[doc.siteid] ++;
				depth_count[doc.depth] ++;

				// Check if its necessary to store the site also
				if( manager_site_status[siteid] != MANAGER_SITE_STATUS_CHECK_SPECIAL ) {

					// Read the sitename
					urlidx_site_by_siteid( urlidx, site.siteid, sitename );

					// Give information to the harvest
					harvest_append_site( harvest, &(site), sitename );

					// Count
					count_sites ++;
				}

				// Mark as special site
				manager_site_status[siteid]	= MANAGER_SITE_STATUS_CHECK_SPECIAL;

			}

		}
	}

	cerr << endl;

	docid_t batchsize_div_50 = CONF_MANAGER_BATCH_SIZE / 50;
	cerr << "Adding documents until we have " << CONF_MANAGER_BATCH_SIZE << endl;
	cerr << "Documents |--------------------------------------------------|" << endl;
	cerr << "          ";

	// Iterate until the batch have been asigned
	for( docid_t i = 1; i<=ndocs; i++ ) {

		// Note that order starts from 0
		doc.docid = order[i-1];

		// Skip documents with 'nil' priority
		if( doc.docid == 0 ) {
			// Skip
			continue;
		}

		// Recover siteid from cache
		doc.siteid = docid_to_siteid[doc.docid];

		// Check if too many from the same site
		if( site_count[doc.siteid] >= CONF_MANAGER_BATCH_SAMESITE ) {
			// Skip, too many from the same site
			continue;
		}

		// Check if too many errors, or we are checking special files, or we already
		// know the site belongs to other harvester
		if( manager_site_status[doc.siteid]	== MANAGER_SITE_STATUS_TOO_MANY_ERRORS
		 || manager_site_status[doc.siteid] == MANAGER_SITE_STATUS_CHECK_SPECIAL
		 || manager_site_status[doc.siteid] == MANAGER_SITE_STATUS_ASSIGNED_TO_OTHER_HARVEST ) {
			// Skip, site is busy
			order[i-1] = 0;
			continue;
		}

		// Retrieve the actual metadata from the document
		metaidx_status = metaidx_doc_retrieve( metaidx, &(doc) );
		assert( metaidx_status == METAIDX_OK );

		// Check if already assigned
		if( doc.status == STATUS_DOC_ASSIGNED ) {
			// Skip assigned documents
			continue;
		}

		// Check if this is the first doc of this site
		if( site_count[doc.siteid] == 0 ) {

			// Retrieve the site
			site.siteid = doc.siteid;
			metaidx_status = metaidx_site_retrieve( metaidx, &(site) );

			// If it is assigned to other site
			if( site.harvest_id > 0 &&
				site.harvest_id != harvest->id ) {
				// Skip: assigned to other harvester

				manager_site_status[doc.siteid] = MANAGER_SITE_STATUS_ASSIGNED_TO_OTHER_HARVEST;
				order[i-1] = 0;
				continue;

			} else if( site.count_error > CONF_MAX_ERRORS_DIFFERENT_BATCH ) {

				// Skip: too many errors
				count_erroneous_sites	++;

				// Mark as erroneous site
				manager_site_status[doc.siteid]	= MANAGER_SITE_STATUS_TOO_MANY_ERRORS;

				order[i-1] = 0;
				continue;

			} else if( site.docid_robots_txt == doc.docid 
					|| site.docid_robots_rdf == doc.docid ) {

				// Do not assign special docs here
				order[i-1] = 0;
				continue;

			} else {
				// Mark as assigned to this harvester,
				// obviously the site must be unassigned first
				assert( site.harvest_id == 0 );
				site.harvest_id = harvest->id;
				metaidx_site_store( metaidx, &(site) );
			}
		}

		// Mark the document as assigned
		order[i-1] = 0;
		doc.status = STATUS_DOC_ASSIGNED;
		metaidx_doc_store( metaidx, &(doc) );

		// Retrieve the url
		urlidx_path_by_docid( urlidx, doc.docid, path );
		assert( path != NULL );

		// Assign the document
		harvest_append_doc( harvest, &(doc), path );

		// Count
		site_count[doc.siteid] ++;
		depth_count[doc.depth] ++;

		if( site_count[doc.siteid] == CONF_MANAGER_BATCH_SAMESITE ) {
			saturated_sites ++;
		}

		// Check if its necessary to store the site also for the harvester
		if( manager_site_status[doc.siteid]	!= MANAGER_SITE_STATUS_STORED ) {

			// Read the sitename
			urlidx_site_by_siteid( urlidx, site.siteid, sitename );

			// Give information to the harvest
			harvest_append_site( harvest, &(site), sitename );

			// Mark as stored
			manager_site_status[doc.siteid]	= MANAGER_SITE_STATUS_STORED;

			// Count
			count_sites ++;
		}

		// Check if done
		if( harvest->count >= CONF_MANAGER_BATCH_SIZE ) {
			break;
		}

		// Report
		if( batchsize_div_50 > 0 && harvest->count % batchsize_div_50 == 0 ) {
			cerr << ".";
		}
	}
	cerr << " " << harvest->count << " done." << endl;

	// Report
	cerr << endl;
	cerr << "Documents excluded from sites with too many errors : " << count_erroneous_sites << endl;
	cerr << endl;
	cerr << "   Number of sites assigned           : " << count_sites << endl;
	cerr << "   Sites assigned with maximum docs   : " << saturated_sites << " with " << CONF_MANAGER_BATCH_SAMESITE << " docs each " << endl;

	cerr << endl << "   Number of assigned documents per depth:" << endl;
	for( depth_t depth = 0; depth <= max_depth; depth++ ) {
		if( depth_count[depth] > 0 ) {
			cerr << "      " << depth << " " << depth_count[depth] << endl;
		}
	}

	// Check how many documents were assigned
	if( harvest->count == 0 )
	{
		cerr << "[No documents available to assign] ";
	}
	else if( harvest->count < CONF_MANAGER_BATCH_SIZE )
	{
		cerr << "[Only " << harvest->count << " pages] ";
	}

	free( manager_site_status );
	free( site_count );
	free( depth_count );
}
	
//
// Name: cleanup
//
// Description: 
//   Closes files and clean everything
//

void cleanup() {
	if( metaidx != NULL ) {
		metaidx_close( metaidx );
		cerr << "[metaidx] ";
	}
	if( linkidx != NULL ) {
		linkidx_close( linkidx );
		cerr << "[linkidx] ";
	}
	if( priority != NULL ) {
		free(priority);
	}
	if( order != NULL ) {
		free(order);
	}
	if( docid_to_siteid != NULL ) {
		free(docid_to_siteid);
	}
}

//
// Name: manager_compare_by_priority
//
// Description:
//   Compare by priority function for stl
//
// Input:
//   a,b - docids
// 
// Output:
//   true if priority[a]<=priority[b]
//   

int manager_compare_by_priority( const void *a, const void *b ) {
	assert( priority != NULL );
	return( priority[(*((const docid_t *)a))-1] <= priority[(*((const docid_t *)b))-1] );
}

//
// Name: manager_open_indexes
//
// Description: 
//   Opens all indexes
//

void manager_open_indexes() {
    cerr << "[metaidx] ";
	metaidx = metaidx_open( COLLECTION_METADATA, false );
    assert( metaidx != NULL );

	cerr << "[linkidx] ";
	linkidx = linkidx_open( COLLECTION_LINK, true );
	assert( linkidx != NULL );

	cerr << "[urlidx] ";
	urlidx = urlidx_open( COLLECTION_URL, true );
	assert( urlidx != NULL );

}

//
// Name: manager_calculate_scores
//
// Description:
//   Calculates the present and future score of each document
//

void manager_calculate_scores() {
	// Priority and order
	// Note that in this case the indices start from 0, because
	// we will use the qsort function.
	priority = (priority_t *)malloc(sizeof(priority_t)*(ndocs+1));
	assert( priority != NULL );
	order    = (docid_t *)malloc(sizeof(docid_t)*(ndocs+1));
	assert( order != NULL );
	docid_to_siteid    = (siteid_t *)malloc(sizeof(siteid_t)*(ndocs+1));
	assert( docid_to_siteid != NULL );

	time_t now = time(NULL);
	siteid_t nsites = metaidx->count_site;
	site_t site;

	// Load site data if necessary
	siterank_t *siterank = NULL;
	if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0 ) {
		siterank = (siterank_t *)malloc(sizeof(siterank_t)*(nsites+1));
		assert( siterank != NULL );
	}

	docid_t *queuesize = NULL;
	if( CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 ) {
		queuesize = (docid_t *)malloc(sizeof(docid_t)*(nsites+1));
		assert( queuesize != NULL );
	}

	docid_t max_queue_size = 0;
	for( site.siteid=1; site.siteid<=nsites; site.siteid++ ) {

		// Load the data from the site
		if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0
		 || CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 ) {
			metaidx_site_retrieve( metaidx, &(site) );
		}

		if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0 ) {
			siterank[site.siteid]	= site.siterank;
		}

		if( CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 ) {
			queuesize[site.siteid] = site.count_doc - (site.count_doc_ok - site.count_error);
			if( queuesize[site.siteid] > max_queue_size ) {
				max_queue_size = queuesize[site.siteid];
			}
		}
	}

	// Normalize queuesize
	if( CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 
	 && max_queue_size > 0 ) {
		for( site.siteid=1; site.siteid<=nsites; site.siteid++ ) {
			queuesize[site.siteid]	/= max_queue_size;
		}
	}

	// Iterate documents
    cerr << endl;
	cerr << "Iterating through " << ndocs << " documents in the collection" << endl;
	cerr << "Calculating scores |--------------------------------------------------|" << endl;
	cerr << "                   ";

	docid_t ndocs_div_50 = ndocs / 50;

	// Documents
	doc_t doc;

	docid_t skipped_successful_and_ok		= 0;
	docid_t skipped_successful_but_not_ok	= 0;
	docid_t skipped_unsuccessful			= 0;
	docid_t skipped_exclusion				= 0;
	docid_t skipped_ignored					= 0;
	docid_t skipped_depth					= 0;
	docid_t skipped_other					= 0;

	for( docid_t docid=1; docid<=ndocs; docid++ ) {

		// Report
		if( ndocs_div_50 > 0 && docid % ndocs_div_50 == 0 ) {
			cerr << ".";
		}

		doc.docid = docid;
		metaidx_doc_retrieve( metaidx, &(doc) );
		assert( doc.docid == docid );

		// Save siteid in a cache of siteids
		docid_to_siteid[docid] = doc.siteid;

		// It's not necessary to calculate
		// scores for all documents, some documents will have priority=-1 (min)

		// Skip documents that were succesfully fetched not long ago
		// The condition doc.last_visit is because it could have been
		// reported as modified by robots.rdf file.
		// The harvester NEVER saves a last_modified date in the
		// future, even if the Web server provides a date in the future

		if(  ((uint)(now - doc.last_visit) < CONF_MANAGER_MINPERIOD_SUCCESSFUL_AND_OK)
		  && HTTP_IS_OK( doc.http_status )
		  && doc.last_visit >= doc.last_modified ) {

			priority[doc.docid-1] = (priority_t)(-1);
			skipped_successful_and_ok	++;
			
		} else if(  ((uint)(now - doc.last_visit) < CONF_MANAGER_MINPERIOD_SUCCESSFUL_BUT_NOT_OK)
		  && (!HTTP_IS_OK( doc.http_status ))
		  && (!HTTP_IS_ERROR( doc.http_status ))
		  && (!HTTP_IS_UNDEF( doc.http_status ))
		  && doc.last_visit >= doc.last_modified ) {

			priority[doc.docid-1] = (priority_t)(-1);
			skipped_successful_but_not_ok	++;

		} else if(  ((uint)(now - doc.last_visit) < CONF_MANAGER_MINPERIOD_UNSUCCESSFUL)
		  && HTTP_IS_ERROR( doc.http_status )
		  && (doc.http_status != HTTP_ERROR_SKIPPED )
		  && doc.last_visit >= doc.last_modified ) {

			priority[doc.docid-1] = (priority_t)(-1);
			skipped_unsuccessful	++;

		// Check if this is excluded
		} else if( doc.status == STATUS_DOC_EXCLUSION ) {
			priority[doc.docid-1] = (priority_t)(-1);
			skipped_exclusion ++;

		// Check if this must be ignored
		} else if( doc.status == STATUS_DOC_IGNORED ) {
			priority[doc.docid-1] = (priority_t)(-1);
			skipped_ignored ++;

		// Check if its too deep
		} else if( doc.depth > 
				(doc.is_dynamic
				 ? CONF_MANAGER_MAXDEPTH_DYNAMIC
				 : CONF_MANAGER_MAXDEPTH_STATIC) ) {

			priority[doc.docid-1] = (priority_t)(-1);
			skipped_depth	++;

		} else {

			// Calculate scores
			if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0 && CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 ) {
				metaidx_doc_calculate_scores( ndocs, nsites, &(doc), siterank[doc.siteid], queuesize[doc.siteid] );
			} else if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0 ) {
				metaidx_doc_calculate_scores( ndocs, nsites, &(doc), siterank[doc.siteid], (docid_t)0 );

			} else if( CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 ) {
				metaidx_doc_calculate_scores( ndocs, nsites, &(doc), (siterank_t)0, queuesize[doc.siteid] );
			} else {
				metaidx_doc_calculate_scores( ndocs, nsites, &(doc), (siterank_t)0, (docid_t)0 );
			}

			// Save the scores
			metaidx_doc_store( metaidx, &(doc) );

			priority[doc.docid-1] = doc.future_score - doc.current_score;

			if( priority[doc.docid-1] == (priority_t)(-1) ) {
				skipped_other	++;
			}
		}

	}
	cerr << " ok." << endl;

	// Free memory
	if( CONF_MANAGER_SCORE_SITERANK_WEIGHT > 0 ) {
		free( siterank );
	}
	if( CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT > 0 ) {
		free( queuesize );
	}

	cerr << "Checking sites ... ";
	siteid_t nsites_too_many_errors	= 0;
	siteid_t nsites_data			= 0;
	siteid_t nsites_assigned		= 0;
	for( siteid_t siteid=1; siteid<=nsites; siteid++ ) {
		site.siteid	= siteid;
		metaidx_site_retrieve( metaidx, &(site) );
		assert( site.siteid == siteid );

		if( site.count_error > CONF_MAX_ERRORS_DIFFERENT_BATCH ) {
			nsites_too_many_errors	++;
		}
		if( site.raw_content_length > 0 ) {
			nsites_data ++;
		}
		if( site.harvest_id > 0 ) {
			nsites_assigned ++;
		}
	}
	cerr << "ok." << endl;

	cerr << "- Total number of sites           : " << nsites << endl;
	cerr << "- Sites with too many errors      : " << nsites_too_many_errors << endl;
	cerr << "- Sites that have transfered data : " << nsites_data << endl;
	cerr << "- Sites assigned to other harvest : " << nsites_assigned << endl;
	cerr << endl;

	// Report totals

	cerr << "Pages that are being excluded, because they were visited not long ago:" << endl;
	cerr << "- Successfully downloaded and HTTP OK (code 200)     : " << skipped_successful_and_ok << endl;
	cerr << "- Successfully downloaded but not ok (code 400, 500) : " << skipped_successful_but_not_ok << endl;
	cerr << "- Not successfully downloaded (errors)               : " <<  skipped_unsuccessful << endl;
	cerr << "Pages that are being excluded for other reasons:" << endl;
	cerr << "- Robots exclusion protocol                          : " << skipped_exclusion << endl;
	cerr << "- Too deep                                           : " << skipped_depth << endl;
	cerr << "- Marked as ignored                                  : " << skipped_ignored << endl;
	cerr << "- Other reasons (normally this is zero)              : " << skipped_other << endl;

	docid_t total_excluded = skipped_successful_and_ok + skipped_successful_but_not_ok + skipped_unsuccessful + skipped_exclusion + skipped_ignored + skipped_depth + skipped_other;

	cerr << "Totals:" << endl;
	cerr << "- Excluded from being assigned    : " << total_excluded << endl;
	cerr << "- Included in possible assignment : " << (ndocs - total_excluded) << endl;
	cerr << "- Total pages in the collection   : " << ndocs << endl;
	cerr << "- Total without ignored pages     : " << (ndocs - skipped_ignored) << endl;
	cerr << endl;
}

//
// Name: manager_order_documents
//
// Description:
//   Sort the documents based on priority
//

void manager_order_documents() {
	// Generate default order; the default order will be the
	// newest URLs first; this does not means depth-first as
	// anyways the crawler respects the priority of pages
	for( docid_t i=1; i<=ndocs; i++ ) {
		order[i-1] = i;
	}

	// Quick Sort
	cerr << "[quicksort] ";
	qsort( order, ndocs, sizeof(docid_t), manager_compare_by_priority );
}


//
// Name: manager_usage
//
// Description:
//   Prints an usage message, then stops
//

void manager_usage() {
	cerr << "Usage: program [OPTION]" << endl;
	cerr << "Calculates scores and creates batches of documents"<< endl;
	cerr << "for the harvester" << endl;
	cerr << endl;
	cerr << " --dont-generate   only calculate, don't generate batch" << endl;
	cerr << " --verify-only     verify status of metaidx" << endl;
	cerr << " -c, --cancel      cancel all remaining harvests" << endl;
	cerr << " --exclude SITE    excludes all pages from a Website" << endl;
	cerr << " --help            this help message" << endl;
	cerr << endl;
	wire_stop(0);
}

