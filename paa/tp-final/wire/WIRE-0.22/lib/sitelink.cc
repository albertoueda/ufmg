
#include "sitelink.h"

// 
// Name: sitelink_create
//
// Description:
//   Generates a sitelink structure
//   - Read all the links from the page link structure
//   - Collapse links to inter-site links
//   - Write to disk
//
// Input:
//   linkidx - structure of links between pages
//   metaidx - structure of metadata
// 
// Output:
//   file containing link structure between sites is created
//   metadata of sites is updated to reflect internal and external links
//

void sitelink_create( const char *dirname, linkidx_t *linkidx, metaidx_t *metaidx ) {
	assert( dirname != NULL );
	assert( strlen(dirname) > 0 );
	assert( linkidx != NULL );
	assert( metaidx != NULL );

	docid_t ndocs		= metaidx->count_doc;
	assert( ndocs > 0 );
	siteid_t nsites		= metaidx->count_site;
	assert( nsites > 0 );

	// Create sitelinkidx structure
	sitelinkidx_t		*sitelinkidx	= (sitelinkidx_t *)malloc( sizeof(sitelinkidx_t) );
	assert( sitelinkidx != NULL );
	sitelinkidx->links	= (sitelink_t **)malloc(sizeof(sitelink_t *) * (nsites + 1));
	assert( sitelinkidx->links != NULL );

	// Init vars of the sitelinkidx
	strcpy( sitelinkidx->dirname, dirname );
	sitelinkidx->count_site	= nsites;
	sitelinkidx->storage	= NULL;

	// Check which sites are is_active
	siteid_t nactive	= 0;
	bool *is_active = (bool *)malloc(sizeof(bool)*(nsites+1));
	assert( is_active != NULL );
	for( siteid_t siteid = 1; siteid <= nsites; siteid++ ) {

		site_t site;
		site.siteid = siteid;

		// Load the site metadata
		metaidx_site_retrieve( metaidx, &(site) );

		// Check if it has documents
		if( site.count_doc_ok > 0 ) {
			is_active[siteid]	= true;
			nactive ++;
		} else {
			is_active[siteid]	= false;
		}

		// Blank the position in the sitelink list
		sitelinkidx->links[siteid]	= NULL;
	}

	// See if this is empty
	if( nactive == 0 ) {
		cerr << "** Warning! Couldn't found any is_active site." << endl;
		cerr << "Either 1) the crawler has not collected yet documents from any site -or- 2) you have not run the analysis program on site statistics" << endl;
		sitelink_save( sitelinkidx );
		return;
	}

	// Read the siteids of docids
	cerr << "Reading siteids  |--------------------------------------------------|" << endl;
	cerr << "                  ";

	docid_t ndocs_div_50 = ndocs/50;
	siteid_t *siteid	 = (siteid_t *)malloc(sizeof(siteid_t)*(ndocs+1));
	assert( siteid != NULL );

	bool *is_ignored	 = (bool *)malloc(sizeof(bool)*(ndocs+1));
	assert( is_ignored != NULL );

	doc_t doc;
	for( doc.docid = 1; doc.docid <= ndocs; doc.docid ++ ) {
		if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
			cerr << ".";
		}
		metaidx_doc_retrieve( metaidx, &(doc) );
		siteid[doc.docid]	= doc.siteid;

		// Mark ignored documents
		if( doc.status == STATUS_DOC_IGNORED ) {
			is_ignored[doc.docid] = true;
		} else {
			is_ignored[doc.docid] = false;
		}
	}
	cerr << endl;

	// Get space for storing data
	docid_t *internal_links = (docid_t *)malloc(sizeof(docid_t) * (nsites + 1));

	assert( internal_links != NULL );
	for( siteid_t siteid = 1; siteid <= nsites; siteid++ ) {
		internal_links[siteid]	= 0;
	}

	// Start to read the page links
	cerr << "Reading links    |--------------------------------------------------|" << endl;
	cerr << "                  ";

	out_link_t dest[SITELINK_MAX_OUTDEGREE+1];
	uint ndest;

	linkidx_prepare_sequential_read( linkidx );
	// Iterate through all the documents
    for( docid_t docid=1; docid<=metaidx->count_doc; docid++ ) {

		// Read the links
		linkidx_sequential_read( linkidx, dest, &(ndest) );

		// Show progress bar
        if( ndocs_div_50 > 0 && docid % ndocs_div_50 == 0 ) {
            cerr << ".";
		}

		// Skip ignored documents (this must be done AFTER reading their links!)
		if( is_ignored[docid] ) {
			continue;
		}

		siteid_t siteid_src	= siteid[docid];

		// Skip inactive sites. This must be done AFTER the links
		// are read, because the read is sequential.
		if( ! is_active[siteid_src] ) {
			continue;
		}

		// Check for dests, and add the link
		for( uint j=0; j<ndest; j++ ) {
			siteid_t siteid_dest = siteid[dest[j].dest];

			// Skip dest. sites that are inactive, with 0 pages gathered ok
			if( ! is_active[siteid_dest] ) {
				continue;
			}

			// Skip links to ignored documents
			if( is_ignored[dest[j].dest] ) {
				continue;
			}

			// If the site is not the same
			if( siteid_dest != siteid_src ) {
				// Add the link
				sitelink_t *ptr	= sitelinkidx->links[siteid_src];

				// Check if this link have been seen
				bool seen = false;
				while( ptr != NULL ) {
					if( ptr->siteid == siteid_dest ) {
						ptr->weight ++;
						seen = true;
						break;
					}
					ptr = ptr->next;
				}

				if( !seen ) {
					// Create
					sitelink_t *newlink	= (sitelink_t *)malloc(sizeof(sitelink_t));
					assert( newlink != NULL );

					newlink->siteid	= siteid_dest;
					newlink->weight	= 1;

					// Insert at the beginning
					newlink->next	= sitelinkidx->links[siteid_src];
					sitelinkidx->links[siteid_src]	= newlink;

				}
			} else {
				internal_links[siteid_src] ++;
			}
		}

	}

	cerr << endl;

	// Save link structure
	sitelink_save( sitelinkidx );

	// Save number of internal links of the site
	site_t site;
	for( site.siteid = 1; site.siteid <= nsites; site.siteid ++ ) {
		metaidx_site_retrieve( metaidx, &(site) );
		site.internal_links	= internal_links[site.siteid];
		metaidx_site_store( metaidx, &(site) );
	}
	// Free
	free( is_ignored );
	free( is_active );
	free( internal_links );
}

// 
// Name: sitelink_save
//
// Description:
//   Saves the sitelink structure from disk
//
// Input:
//   sitelinkidx - the structure
//
//

void sitelink_save( sitelinkidx_t *sitelinkidx ) {
	assert( sitelinkidx != NULL );
	assert( sitelinkidx->links != NULL );

	siteid_t nsites	= sitelinkidx->count_site;

	// Adjacency list, for saving
	saved_sitelink_t adjacency_list[SITELINK_MAX_OUTDEGREE];

	// Create the storage
	assert( sitelinkidx->storage == NULL );
	sitelinkidx->storage = storage_create( sitelinkidx->dirname, false );
	assert( sitelinkidx->storage != NULL );

	// Report
	cerr << "Saving sitelinks |--------------------------------------------------|" << endl;
	cerr << "                  ";

	site_t site;
	siteid_t nsites_div_50	= nsites / 50;
	for( site.siteid = 1; site.siteid <= nsites; site.siteid ++ ) {

		// Report
		if( nsites_div_50 > 0 && site.siteid % nsites_div_50 == 0 ) {
			cerr << ".";
		}

		uint adjacency_list_length = 0;
		sitelink_t *ptr = sitelinkidx->links[site.siteid];

		// Copy adjacency list to structure, for saving
		while( ptr != NULL ) {
			// Copy data
			adjacency_list[adjacency_list_length].siteid = ptr->siteid;
			adjacency_list[adjacency_list_length].weight = ptr->weight;

			adjacency_list_length ++;

			if( adjacency_list_length == (SITELINK_MAX_OUTDEGREE - 1) ) {
				// Too long
				ptr = NULL;
			} else {
				// Continue
				ptr = ptr->next;
			}
		}

		// Store adjancency list in the storage file
		if( adjacency_list_length > 0 ) {
			storage_write( sitelinkidx->storage,
				site.siteid,
				(char *)(adjacency_list),
				adjacency_list_length * sizeof(saved_sitelink_t),
				NULL, NULL );
		}

	}
	cerr << endl;

	// Close storage
	storage_close( sitelinkidx->storage );
	sitelinkidx->storage = NULL;

	// Save information about the sitelink
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s/%s", sitelinkidx->dirname, SITELINK_FILENAME );
	FILE *out	= fopen64( filename, "w" );
	assert( out != NULL );
	fwrite( sitelinkidx, sizeof(sitelinkidx_t), 1, out );
	fclose( out );

	// Free memory, to prevent saving again
	free( sitelinkidx->links );
	sitelinkidx->links	= NULL;
	free( sitelinkidx );
}

// 
// Name: sitelink_load
//
// Description:
//   Read the sitelink structure from disk
//
// Input:
//   metaidx - structure of metadata
//
// Return:
//   structure of links between sites
//

sitelinkidx_t *sitelink_load( const char *dirname ) {
	// Get memory for the sitelink structure
	sitelinkidx_t		*sitelinkidx	= (sitelinkidx_t *)malloc( sizeof(sitelinkidx_t) );
	assert( sitelinkidx != NULL );

	// Load the structure's data
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s/%s", dirname, SITELINK_FILENAME );
	FILE *in = fopen64( filename, "r" );
	assert( in != NULL );
	fread( sitelinkidx, sizeof(sitelinkidx_t), 1, in );
	fclose( in );

	// Check nsites
	siteid_t nsites	= sitelinkidx->count_site;
	assert( nsites <= CONF_COLLECTION_MAXSITE );

	// Copy the dirname (it might have changed)
	strcpy( sitelinkidx->dirname, dirname );

	// Load the links
	sitelinkidx->links	= (sitelink_t **)malloc(sizeof(sitelink_t *) * (nsites + 1));
	assert( sitelinkidx->links != NULL );

	// Open the storage file
	sitelinkidx->storage	= storage_open( sitelinkidx->dirname, true );
	assert( sitelinkidx->storage != NULL );

	// Sequential read is to speed up the process of reading
	storage_record_t rec;
	storage_status_t rc;
	uint ndest = 0;
	saved_sitelink_t adjacency_list[SITELINK_MAX_OUTDEGREE];

	storage_prepare_sequential_read( sitelinkidx->storage );
	for( siteid_t siteid = 1; siteid <= nsites; siteid++ ) {

		// Start with a blank adjacency list at this position
		sitelinkidx->links[siteid]	= NULL;

		// Do sequential read
		rc = storage_sequential_read( sitelinkidx->storage,
				&(rec),
				(char *)(adjacency_list) );

		// Check if something was read
		if( rc == STORAGE_OK && rec.size > 0 ) {

			// It was read
			ndest	= rec.size/sizeof(saved_sitelink_t);

			// Add to the adjacency 
			for( uint i=0; i<ndest; i++ ) {
				sitelink_t *item	= (sitelink_t *)malloc(sizeof(sitelink_t));
				item->next			= sitelinkidx->links[siteid];
				item->siteid		= adjacency_list[i].siteid;
				item->weight		= adjacency_list[i].weight;
				sitelinkidx->links[siteid]	= item;
			}
		}

	}

	storage_close( sitelinkidx->storage );

	return sitelinkidx;
}

// 
// Name: sitelink_dump_structure
//
// Description:
//   Shows the site structure to screen
//
// Input:
//   sitelinkidx - the structure
//

void sitelink_dump_structure( sitelinkidx_t *sitelinkidx ) {
	assert( sitelinkidx != NULL );

	siteid_t nsites	= sitelinkidx->count_site;
	for( siteid_t siteid = 1; siteid <= nsites; siteid++ ) {
		sitelink_dump_links( sitelinkidx, siteid );
	}
}

//
// Name: sitelink_dump_links
//
// Description:
//   Shows all the links from a sites
//
// Input:
//   sitelinks - the structure
//   siteid - siteid to inspect
//

void sitelink_dump_links( sitelinkidx_t *sitelinkidx, siteid_t siteid ) {
	assert( sitelinkidx != NULL );
	assert( sitelinkidx->links != NULL );
	assert( siteid > 0 );

	cerr << siteid;
	sitelink_t *ptr = sitelinkidx->links[siteid];
	while( ptr != NULL ) {
		cerr << " " << ptr->siteid << "(" << ptr->weight << ")";
		ptr	= ptr->next;
	}
	cerr << endl;
}

//
// Name: sitelink_dump_links_with_sitename
//
// Description:
//   Shows all the links from a sites
//   includes names of the dest. sites
//
// Input:
//   sitelinkidx - the structure
//   urlidx - index of urls
//   siteid - siteid to inspect
//

void sitelink_dump_links_with_sitename( sitelinkidx_t *sitelinkidx, urlidx_t *urlidx, siteid_t siteid ) {
	assert( sitelinkidx != NULL );
	assert( urlidx != NULL );
	assert( siteid > 0 );
	char sitename[MAX_STR_LEN]	= "";

	sitelink_t *ptr = sitelinkidx->links[siteid];
	cout << siteid << endl;
	uint	nlinks	= 0;

	// Iterate through the linked list
	siteid_t sum_weight	= 0;
	while( ptr != NULL ) {
		urlidx_site_by_siteid( urlidx, ptr->siteid, sitename );
		cout << ptr->siteid << "(" << ptr->weight << ") ";
		cout << sitename << endl;
		sum_weight += ptr->weight;
		ptr = ptr->next;
		nlinks++;
	}

	// Report number of links
	if( nlinks == 0 ) {
		cout << "No links" << endl;
	} else {
		cout << "Total: " << sum_weight <<  " links to " << nlinks << " sites" << endl;
	}
}

//
// Strongly connected components calculations
//

//
// Name: sitelink_scc_clean_vector
//   
// Description:
//   Cleans a vector of integers
//
// Input:
//   nsites - number of sites
//   scc_vector - vector to clean
//

void sitelink_scc_clean_vector( siteid_t nsites, int *scc_vector ) {
	for( siteid_t i=1; i<=nsites; i++ ) {
		scc_vector[i]	= 0;
	}
}

void sitelink_scc_clean_bool_vector( siteid_t nsites, bool *scc_vector ) {
	for( siteid_t i=1; i<=nsites; i++ ) {
		scc_vector[i]	= false;
	}
}

//
// Name: sitelink_reverse_structure
//
// Description:
//   Reverses an adjacency list
//
// Input:
//   nsites - number of sites
//   sitelinks - adjacency list
//
// Returns:
//   reversed adjacency list
//
sitelink_t **sitelink_reverse_structure( siteid_t nsites, sitelink_t **sitelinks ) {
	// Allocate memory
	sitelink_t **reverse_sitelinks	= (sitelink_t **)malloc(sizeof(sitelink_t *) * (nsites + 1));
	assert( reverse_sitelinks != NULL );
	for( siteid_t siteid = 1; siteid <= nsites; siteid++ ) {
		reverse_sitelinks[siteid]	= NULL;
	}

	// This reverses the adjacency list
	// the new list cannot be saved to disk: it may have more than
	// the maximum number of outgoing links
	sitelink_t *ptr;
	for( siteid_t siteid = 1; siteid <= nsites; siteid++ ) {
		ptr = sitelinks[siteid];

		while( ptr != NULL ) {
			sitelink_t *newlink	= (sitelink_t *)malloc(sizeof(sitelink_t));
			newlink->siteid	= siteid;
			newlink->next	= reverse_sitelinks[ptr->siteid];
			reverse_sitelinks[ptr->siteid]	= newlink;
			
			ptr 	= ptr->next;
		}
	}

	return reverse_sitelinks;
}

//
// Name: sitelink_scc_dfs_compute_finish
//
// Description:
//   Used by the SCC algorithm
//

void sitelink_scc_dfs_compute_finish( siteid_t nsites, sitelink_t **sitelinks, siteid_t v, bool *scc_vec_mark, int *scc_vec_finish, int *scc_finish_time ) {
	assert( v >= 1 );
	assert( v <= nsites );

    siteid_t w;

    scc_vec_mark[v] = true;

	sitelink_t *ptr	= sitelinks[v];

	while( ptr != NULL ) {
		w	= ptr->siteid;

        if( scc_vec_mark[w] == false ) {
            sitelink_scc_dfs_compute_finish(nsites, sitelinks, w, scc_vec_mark, scc_vec_finish, scc_finish_time);
        }

		ptr = ptr->next;
    }

	// Mark finish time
    (*scc_finish_time) = (*scc_finish_time)+1;
    scc_vec_finish[v] = (*scc_finish_time);
}

//
// Name: sitelink_scc_dfs_decreasing
//
// Description:
//   Used by the SCC algorithm
//

void sitelink_scc_dfs_decreasing( siteid_t v, sitelink_t **reverse_sitelinks, bool *scc_vec_mark, int *scc_vec_component, int *scc_count ) {
	sitelink_t *ptr;
	siteid_t w;

	scc_vec_mark[v]			= true;
	scc_vec_component[v]	= (*scc_count);
	
	ptr	= reverse_sitelinks[v];
	while( ptr != NULL ) {
		w	= ptr->siteid;
		if( scc_vec_mark[w] == false ) {
			sitelink_scc_dfs_decreasing( w, reverse_sitelinks, scc_vec_mark, scc_vec_component, scc_count );
		}
		ptr = ptr->next;
	}
}

//
// Name: sitelink_mark_reachable
//
// Description:
//   Marks all the sites that are reachable from a site in the graph.
//   It receives an old_component_name and a new_component_name
//   Recursively mark veritces in the graph, only of those that
//   match the old_component_name
//
// Input:
//   sitelinks - the graph structure
//   siteid - the source vertex
//   components - the array of component names
//   old_component_name - the component name that must match
//   new_component_name - the component name to assign to reachable vertices
//   recursive - if true, mark recursively (recursion only in marked nodes!)
//
// Output:
//   components - contains marked elements with the new component name
//

void sitelink_mark_reachable( sitelink_t **sitelinks, siteid_t siteid, component_t *components, component_t old_component_name, component_t new_component_name, bool recursive ) {
	sitelink_t	*ptr;
	ptr	= sitelinks[siteid];
	while( ptr != NULL ) {
		if( components[ptr->siteid] == old_component_name ) {
			components[ptr->siteid] = new_component_name;

			// The recursion only happens if we're marking nodes
			// If we always do recursion, we can be trapped into cycles
			if( recursive ) {
				sitelink_mark_reachable( sitelinks, ptr->siteid, components, old_component_name, new_component_name, recursive );
			}
		}
		ptr = ptr->next;
	}
}

//
// Name: sitelink_components
//
// Description:
//   Generates the list of strongly connected components
//   Calculates the component for each site
//
// Input:
//   nsites - the number of sites
//   sitelink - the structure with the links
//

void sitelink_analysis_components( sitelinkidx_t *sitelinkidx, metaidx_t *metaidx ) {
	assert( sitelinkidx != NULL );
	assert( metaidx != NULL );

	siteid_t nsites		= sitelinkidx->count_site;

	// Check consistency
	assert( nsites == metaidx->count_site );

	// Marks
	bool *scc_vec_mark		= (bool *)malloc(sizeof(bool)*(nsites+1));
	assert( scc_vec_mark != NULL );
	int *scc_vec_finish		= (int *)malloc(sizeof(int)*(nsites+1));
	assert( scc_vec_finish != NULL );
	int *scc_vec_order		= (int *)malloc(sizeof(int)*(nsites+1));
	assert( scc_vec_order != NULL );

	// Read the sites to check which sites are active
	cerr << "Reading list of active sites ... ";
	site_t site;
	bool *active = (bool *)malloc(sizeof(bool)*(nsites+1));
	assert( active != NULL );
	siteid_t nactive = 0;
	for( site.siteid = 1; site.siteid <= nsites; site.siteid++ ) {
		metaidx_site_retrieve( metaidx, &(site) );
		if( site.count_doc_ok > 0 ) {
			active[site.siteid]	= true;
			nactive ++;
		} else {
			active[site.siteid]	= false;
		}
	}
	cerr << nactive << " sites done." << endl;

	// Compute finish times
	cerr << "Computing finish times ... ";
	sitelink_scc_clean_bool_vector( nsites, scc_vec_mark );
	sitelink_scc_clean_vector( nsites, scc_vec_finish );
	cerr << "[cleaned] ";
	sitelink_scc_clean_bool_vector( nsites, scc_vec_mark );
	sitelink_scc_clean_vector( nsites, scc_vec_finish );

	int scc_finish_time	= 0;
	
	for( siteid_t v=1; v<=nsites; v++ ) {
		if( scc_vec_mark[v] == false ) {
			sitelink_scc_dfs_compute_finish( nsites, sitelinkidx->links, v, scc_vec_mark, scc_vec_finish, &(scc_finish_time) );
		}
	}
	cerr << "done." << endl;

	// Reverse adjacency list
	cerr << "Reversing adjacency list ... ";
	sitelink_t **reverse_sitelinks	= sitelink_reverse_structure( nsites, sitelinkidx->links );
	cerr << "done." << endl;

	// Determine the order (decreasing finish)
	for( siteid_t v=1; v<=nsites; v++ ) {
		scc_vec_order[nsites-scc_vec_finish[v]+1] = v;
	}

	free( scc_vec_finish ); scc_vec_finish = NULL;

	// DFS by decreasing finish time
	int *scc_vec_component	= (int *)malloc(sizeof(int)*(nsites+1));
	assert( scc_vec_component != NULL );
	cerr << "Running DFS by decreasing finish time ... ";
	sitelink_scc_clean_bool_vector( nsites, scc_vec_mark );
	sitelink_scc_clean_vector( nsites, scc_vec_component );
	int scc_count	= 0;
	for( siteid_t i=1; i<=nsites; i++ ) {
		// Get the node that is going to be visited now
		siteid_t v	= scc_vec_order[i];

		if( scc_vec_mark[v] == false ) {
			scc_count ++;
			sitelink_scc_dfs_decreasing(v, reverse_sitelinks, scc_vec_mark, scc_vec_component, &(scc_count)); 
		}
	}
	cerr << "done." << endl;

	// The connected components were calculated, locate the largest
	int scc_giant_component = 0;
	int scc_giant_component_size = 0;
	int scc_singletons		= 0;
	int scc_no_outlinks		= 0;
	int scc_no_inlinks		= 0;

	int *scc_size = (int *)malloc(sizeof(int)*(scc_count+1));
	for( int i=0; i<=scc_count; i++ ) {
		scc_size[i]	= 0;
	}
	for( siteid_t siteid=1; siteid<=nsites; siteid++ ) {
		if( ! active[siteid] ) {
			continue;
		}
		scc_size[scc_vec_component[siteid]] ++;
		if( sitelinkidx->links[siteid] == NULL ) {
			scc_no_outlinks ++;
		}
		if( reverse_sitelinks[siteid] == NULL ) {
			scc_no_inlinks ++;
		}
	}

	map<siteid_t,uint> scc_size_histogram;
	for( int i=0; i<=scc_count; i++ ) {
		if( scc_size[i] > scc_giant_component_size ) {
			scc_giant_component			= i;
			scc_giant_component_size	= scc_size[i];

		}
		if( scc_size[i] == 1 ) {
			scc_singletons ++;
		}

		scc_size_histogram[scc_size[i]] ++;
	}

	// Create directory
	createdir( COLLECTION_ANALYSIS );

	// Create dirname for analysis
	char dirname[MAX_STR_LEN];
	sprintf( dirname, "%s/sitelink", COLLECTION_ANALYSIS );
	createdir( dirname );

	// Write general statistics

	char filename[MAX_STR_LEN];

	sprintf( filename, "%s/%s", dirname, "stats.csv" );
	FILE *output = fopen64( filename, "w" );
	assert( output != NULL );

	cerr << "* Writing stats to " << filename << " ... ";

	fprintf( output, "Total number of site names known,%lu\n", nsites );
	fprintf( output, "Sites with at least one page ok,%lu\n", nactive );
	fprintf( output, "Sites without in links (but at least one page ok),%d\n", scc_no_inlinks );
	fprintf( output, "Sites without out links (but at least one page ok),%d\n", scc_no_outlinks );
	fprintf( output, "Size of largest SCC,%d\n", scc_giant_component_size );
	fprintf( output, "SCC-id of largest SCC,%d\n", scc_giant_component );
	fprintf( output, "Number of SCCs with one site only (singletons),%d\n", scc_singletons );

	fclose( output );

	cerr << "done." << endl;

	//
	// Calculate components
	//

	// Mark all the sites in MAIN_NORM
	// All the other sites start as ISLAND
	component_t *component = (component_t *)malloc(sizeof(component_t)*(nsites+1));
	for( siteid_t siteid=1; siteid <= nsites; siteid ++ ) {
		if( scc_vec_component[siteid] == scc_giant_component ) {
			component[siteid]	= COMPONENT_MAIN_NORM;
		} else if( active[siteid] ) {
			component[siteid]	= COMPONENT_ISLAND;
		} else {
			component[siteid]	= COMPONENT_UNDEF;
		}
	}

	// Mark all the sites in OUT: can be reached from IN
	for( siteid_t siteid=1; siteid <= nsites; siteid ++ ) {
		if( component[siteid] == COMPONENT_MAIN_NORM ) {
			sitelink_mark_reachable( sitelinkidx->links, siteid, component, COMPONENT_ISLAND, COMPONENT_OUT, true );
		}
	}

	// Mark all the sites in IN: can reach MAIN_NORM
	for( siteid_t siteid=1; siteid <= nsites; siteid ++ ) {
		if( component[siteid] == COMPONENT_MAIN_NORM ) {
			sitelink_mark_reachable( reverse_sitelinks, siteid, component, COMPONENT_ISLAND, COMPONENT_IN, true );
		}
	}

	// Mark all the sites in MAIN_IN: are directly reached from IN and are MAIN
	// Mark all the sites in TIN: can be reached from IN, but are not in MAIN
	for( siteid_t siteid=1; siteid <= nsites; siteid ++ ) {
		if( component[siteid] == COMPONENT_IN ) {
			sitelink_mark_reachable( sitelinkidx->links, siteid, component, COMPONENT_MAIN_NORM, COMPONENT_MAIN_IN, false );
			sitelink_mark_reachable( sitelinkidx->links, siteid, component, COMPONENT_ISLAND, COMPONENT_TIN, true );
		}
	}

	// Mark all the sites in MAIN_OUT: can directly reach OUT and are MAIN
	// Mark all the sites in MAIN_MAIN: can directly reach OUT and are MAIN_IN
	// Mark all the sites in TOUT: can reach OUT, but are not in MAIN
	for( siteid_t siteid=1; siteid <= nsites; siteid ++ ) {
		if( component[siteid] == COMPONENT_OUT ) {
			sitelink_mark_reachable( reverse_sitelinks, siteid, component, COMPONENT_MAIN_NORM, COMPONENT_MAIN_OUT, false );
			sitelink_mark_reachable( reverse_sitelinks, siteid, component, COMPONENT_ISLAND, COMPONENT_TOUT, true );
			sitelink_mark_reachable( reverse_sitelinks, siteid, component, COMPONENT_TIN, COMPONENT_TUNNEL, true );

			sitelink_mark_reachable( reverse_sitelinks, siteid, component, COMPONENT_MAIN_IN, COMPONENT_MAIN_MAIN, false );
		}
	}

	// 
	// Write statistic
	//
	sprintf( filename, "%s/%s", dirname, "siteid_sccid_component.csv" );
	output = fopen64( filename, "w" );
	assert( output != NULL );
	cerr << "* Siteid and component -> " << filename << " ... ";

	map<component_t,siteid_t> count_component;

	fprintf( output, "Siteid,Strongly connected component id,Component\n" );
	siteid_t nsites_not_undef	= 0;
	for( siteid_t siteid=1; siteid <= nsites; siteid ++ ) {
		fprintf( output, "%lu,%d,%s\n", siteid, scc_vec_component[siteid],COMPONENT_STR(component[siteid]) );
		
		// Write component to metadata
		site.siteid = siteid;
		metaidx_site_retrieve( metaidx, &(site) );
		assert( site.siteid == siteid );
		site.component	= component[siteid];
		metaidx_site_store( metaidx, &(site) );

		// Get component size
		if( scc_vec_component[siteid] != COMPONENT_UNDEF ) {
			nsites_not_undef ++;
		}

		// Count
		count_component[component[siteid]] ++;
	}

	cerr << "done." << endl;
	fclose( output );

	// Size of each strongly-connected-component
	sprintf( filename, "%s/%s", dirname, "scc_sizes.csv" );
	output = fopen64( filename, "w" );
	assert( output != NULL );
	cerr << "* Strongly connected component sizes -> " << filename << " ... ";

	fprintf( output, "SCC size,Number of SCC components,Fraction\n" );

	map<siteid_t,uint>::iterator scc_size_histogram_it;
	for( scc_size_histogram_it = scc_size_histogram.begin(); scc_size_histogram_it != scc_size_histogram.end(); scc_size_histogram_it++ ) {
		// There are some SCC that were created but never used,
		// remember that at the beginning each site starts in its
		// own SCC
		if( (*scc_size_histogram_it).first > 0 ) {
			fprintf( output, "%lu,%d,%e\n", (*scc_size_histogram_it).first, (*scc_size_histogram_it).second, (double)((*scc_size_histogram_it).second) / ((double)scc_count - (double)scc_size_histogram[0]) );
		}
	}
	fclose( output );
	cerr << endl;

	// Get stats
	sprintf( filename, "%s/%s", dirname, "graph_component_sizes.csv" );
	output = fopen64( filename, "w" );
	assert( output != NULL );
	cerr << "* Graph component sizes -> " << filename << " ... ";

	fprintf( output, "Component name,Number of sites,Fraction\n" );
	map<component_t,siteid_t>::iterator component_it;
	for( component_it = count_component.begin(); component_it != count_component.end(); component_it++ ) {
		if( (*component_it).first != COMPONENT_UNDEF ) {
			fprintf( output, "%s,%lu,%e\n", COMPONENT_STR((*component_it).first), (*component_it).second, (double)((*component_it).second) / (double)nsites_not_undef );
		}
	}
	fclose( output );

	cerr << endl;

	free( scc_size );

	free( scc_vec_mark );
	free( scc_vec_order );
	free( scc_vec_component );
}

//
// Name: sitelink_siterank
//
// Description:
//   Calculates the siterank for each site.
//   The siterank of a site is an aproximation of the sum of the
//   pageranks for the pages inside the website.
//
// Input:
//   sitelinkidx - the sitelink structure
//   metaidx - the metadata index
//   use_internal_links - if true, the number of internal links
//       is used in the calculus.
//   use_site_size - if true, the site size is used in the calculus
//
//

siterank_t *siterank_cpy	= NULL;
int sitelink_compare_by_siterank( const void *a, const void *b ) {
	return( siterank_cpy[(*((const siteid_t *)a))] >= siterank_cpy[(*((const siteid_t *)b))] );
}



void sitelink_analysis_siterank( sitelinkidx_t *sitelinkidx, metaidx_t *metaidx,
		bool use_internal_links, bool use_site_size, bool linearize ) {

	site_t site;

	siteid_t nsites			= sitelinkidx->count_site;
	assert( nsites == metaidx->count_site );
	siteid_t nsites_active	= 0;
	docid_t  ndocs_total	= 0;


	// Allocates two vectors for the pagerank
	siterank_t *siterank = (siterank_t *)malloc(sizeof(siterank_t)*(nsites+1));
	assert( siterank != NULL );

	siterank_t *siterankTmp = (siterank_t *)malloc(sizeof(siterank_t)*(nsites+1));
	assert( siterankTmp != NULL );

	docid_t *internal_links = (docid_t *)malloc(sizeof(docid_t)*(nsites+1));
	assert( internal_links != NULL );

	docid_t *count_doc_ok = (docid_t *)malloc(sizeof(docid_t)*(nsites+1));
	assert( count_doc_ok != NULL );

	siteid_t *in_degree = (docid_t *)malloc(sizeof(siteid_t)*(nsites+1));
	assert( in_degree != NULL );

	siteid_t *out_degree = (docid_t *)malloc(sizeof(siteid_t)*(nsites+1));
	assert( out_degree != NULL );

	siterank_t *sum_weight = (siterank_t *)malloc(sizeof(siterank_t)*(nsites + 1));
	assert( sum_weight != NULL );

	// Cleans the siterank vector
	// Cleans the in_degree vector
	// Loads the number of internal links for each site
	for( site.siteid=1; site.siteid <= nsites; site.siteid++ ) {
		metaidx_site_retrieve( metaidx, &(site) );

		internal_links[site.siteid]		= site.internal_links;
		count_doc_ok[site.siteid]		= site.count_doc_ok;

		in_degree[site.siteid]			= 0;
		out_degree[site.siteid]			= 0;

		sum_weight[site.siteid]			= 0;

		if( count_doc_ok[site.siteid] > 0 ) {
			nsites_active ++;
			ndocs_total	+= site.count_doc_ok;
		}
	}

	for( site.siteid=1; site.siteid <= nsites; site.siteid ++ ) {
		if( count_doc_ok[site.siteid] > 0 ) {
			siterank[site.siteid] = ((siterank_t)1/(siterank_t)nsites_active);
		} else {
			siterank[site.siteid] = 0;
		}
	}

	// Calculates the in_degree of sites
	sitelink_t *ptr;
	for( site.siteid=1; site.siteid <= nsites; site.siteid++ ) {
		ptr = sitelinkidx->links[site.siteid];
		while( ptr != NULL ) {
			out_degree[site.siteid] ++;
			in_degree[ptr->siteid] ++;
			sum_weight[site.siteid] += ptr->weight;
			ptr = ptr->next;
		}
	}

	double omq	= (double)CONF_MANAGER_SCORE_PAGERANK_DAMPENING;
	double q	= (double)1 - omq;

	uint iteration	= 0;
	while( iteration < SITELINK_MAX_ITERATIONS ) {
		iteration ++;

		cerr << "- Iteration " << iteration << endl;

		// Clear TMP
		for( siteid_t siteid = 1; siteid <= nsites; siteid ++ ) {
			siterankTmp[siteid]	= (siterank_t)0;
		}

		// In this step, we add to a site the siterank of all the sites
		// with links to it, but considering that each site confers
		// only a fraction of its siterank:
		//   1 / (out_degree + internal_links)
		// on each link.
		for( siteid_t siteid = 1; siteid <= nsites; siteid ++ ) {
			ptr = sitelinkidx->links[siteid];
			while( ptr != NULL ) {

				if( use_internal_links ) {
					// Internal links will keep the random surfer in the same site,
					// so the probability of jumping outside is less
					siterankTmp[ptr->siteid] +=
						( siterank[siteid] * (siterank_t)(ptr->weight) )
						/
						( (siterank_t)sum_weight[siteid] + (siterank_t)internal_links[siteid] );
				} else {
					// If we don't use internal links, we just care about
					// the weight of the site
					siterankTmp[ptr->siteid] +=
						( siterank[siteid] * (siterank_t)(ptr->weight) )
						/
						( (siterank_t)sum_weight[siteid] );
				}
				ptr = ptr->next;
			}
		}

		// Now we complete the calculus, we will add the siterank
		// added by the links betweeen all sites (with probability q)
		// and the siterank added by auto-links (with probabilith 1-q=omq)
		siterank_t sum_siteranks = 0;
		for( siteid_t siteid = 1; siteid <= nsites; siteid ++ ) {
			if( count_doc_ok[siteid] > 0 ) {

				siterank_t contribution_random_links	= 0;

				// Check if we must use site size
				// When we use site size, larger sites will 
				// have an advantage.
				if( use_site_size ) {
					// Probability of getting here by a random
					// link, proportional to the number of documents
					// in this website. We will divide the number of documents
					// in this website by the average documents.
					contribution_random_links = 
						((double)count_doc_ok[siteid] /  (double)ndocs_total);
				} else {
					// Probability of getting here by a random
					// link, constant for all sites
					contribution_random_links = 
						(double)1/(double)nsites_active;
				}
				
				//  Calculate
				siterankTmp[siteid] =
					  (q*contribution_random_links)
					+ (omq*siterankTmp[siteid]);

				// If we are using internal links, add the
				// probability of staying in the same node
				if( use_internal_links ) {
					if( internal_links[siteid] + out_degree[siteid] > 0 ) {
						siterankTmp[siteid]	+=
							omq * siterank[siteid] * 
							 ((siterank_t)internal_links[siteid] /
							    ((siterank_t)internal_links[siteid] +
								 (siterank_t)sum_weight[siteid])
							 );
					}
				}

				// Sum, this is to normalize at the end
				sum_siteranks	+= siterankTmp[siteid];
			}
		}

		siterank_t	siterank_delta	= 0;
		siterank_t siterank_aux	= 0;
		for( siteid_t siteid = 1; siteid <= nsites; siteid ++ ) {

			// Calculate change in siterank = new/old value
			siterank_aux	= siterankTmp[siteid]/sum_siteranks;

			// Check variation of siterank
			siterank_delta	+= linkidx_calc_delta( siterank_aux, siterank[siteid] );

			// Normalize to avoid overflow
			siterank[siteid] = siterank_aux;
		}

		siterank_delta	/= nsites_active;
		cerr << "  siterank average error = " << siterank_delta << endl;
		cerr << "  siterank sum values    = " << sum_siteranks << endl;

		if( siterank_delta < CONF_MANAGER_SCORE_SITERANK_MAX_ERROR ) {
			cerr << "OK, target average error reached " << CONF_MANAGER_SCORE_SITERANK_MAX_ERROR << endl;
			break;
		}

		if( sum_siteranks == 0 ) {
			cerr << "Warning: no site has a positive score, maybe the collection process is just starting" << endl;
			break;
		}
	}

	if( iteration == SITELINK_MAX_ITERATIONS ) {
		die( "Maximum limit of iterations reached without the desired convergence" );
	}

	// Linearize
	if( linearize ) {
		siteid_t *order;
		order	= (siteid_t *)malloc(sizeof(siteid_t)*(nsites+1));
		assert( order != NULL );

		cerr << "Linearizing ... ";
		// Generate base order
		for( siteid_t i=1; i<=nsites; i++ ) {
			order[i-1] = i;
		}

		cerr << "ordering, ";
		siterank_cpy	= siterank;
		qsort( order, nsites, sizeof(siteid_t), sitelink_compare_by_siterank );
		siterank_cpy	= NULL;

		// Shift
		for( siteid_t i=nsites; i>0; i-- ) {
			order[i] 	= order[i-1];
		}

		cerr << "assigning, ";

		// While linearizing
		// Avoid giving siterank>0 to websites with siterank=0
		siteid_t nsites_gt_0	= 0;
		for( siteid_t i=1; i<=nsites; i++ ) {
			if( siterank[i] > 0 ) {
				nsites_gt_0 ++;
			}
		}

		double step				= (double)1/(double)nsites_gt_0;
		double current_value	= step;
		for( siteid_t i=1; i<=nsites; i++ ) {
			if( siterank[order[i]] > 0 ) {
				siterank[order[i]]	= current_value;
				current_value		+= step;
			}
		}

		cerr << "done." << endl;
		free(order);

	}


	// Saves date
	cerr << "Saving ... ";
	for( site.siteid=1; site.siteid <= nsites; site.siteid++ ) {
		metaidx_site_retrieve( metaidx, &(site) );
		site.in_degree		= in_degree[site.siteid];
		site.out_degree		= out_degree[site.siteid];
		site.siterank		= siterank[site.siteid];
		metaidx_site_store( metaidx, &(site) );
	}
	cerr << "done." << endl;

	free( sum_weight );

}
