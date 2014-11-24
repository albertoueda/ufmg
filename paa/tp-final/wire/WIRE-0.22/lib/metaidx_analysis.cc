#include "metaidx_analysis.h"

//
// Name: metaidx_analysis_site_statistics
//
// Description:
//   Generates statistics about sites:
//    Age of oldest page
//    Age of newest page
//    Various document counts
//    Max depth
//
// Input:
//   metaidx - the metadata structure
//


void metaidx_analysis_site_statistics( metaidx_t *metaidx ) {
	uint *count_doc;
	uint *count_doc_ok;
	uint *count_doc_gathered;
	uint *count_doc_ignored;
	uint *count_doc_new;
	uint *count_doc_assigned;
	uint *count_doc_date_ok;
	uint *count_doc_static;
	uint *count_doc_dynamic;

	uint *max_depth;

	pagerank_t	*sum_pagerank;
	hubrank_t	*sum_hubrank;
	authrank_t	*sum_authrank;

	time_t *age_oldest_page;
	time_t *age_newest_page;
	double *age_average_page;

	// Ask for memory
	siteid_t nsites = metaidx->count_site;

	count_doc = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc != NULL );
	count_doc_gathered = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_gathered != NULL );
	count_doc_ignored = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_ignored != NULL );
	count_doc_new = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_new != NULL );
	count_doc_assigned = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_assigned != NULL );
	count_doc_ok = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_ok != NULL );
	count_doc_date_ok = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_date_ok != NULL );
	count_doc_static = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_static != NULL );
	count_doc_dynamic = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( count_doc_dynamic != NULL );
	max_depth = (uint *)malloc(sizeof(uint)*(nsites+1));
	assert( max_depth != NULL );


	age_oldest_page = (time_t *)malloc(sizeof(time_t)*(nsites+1));
	assert( age_oldest_page != NULL );
	age_newest_page = (time_t *)malloc(sizeof(time_t)*(nsites+1));
	assert( age_newest_page != NULL );
	age_average_page = (double *)malloc(sizeof(double)*(nsites+1));
	assert( age_average_page != NULL );

	sum_pagerank	= (pagerank_t *)malloc(sizeof(pagerank_t)*(nsites + 1));
	assert( sum_pagerank	!= NULL );
	sum_hubrank		= (hubrank_t *)malloc(sizeof(hubrank_t)*(nsites + 1));
	assert( sum_hubrank	!= NULL );
	sum_authrank	= (authrank_t *)malloc(sizeof(authrank_t)*(nsites + 1));
	assert( sum_authrank	!= NULL );
		

	time_t now	= time(NULL);

	for( siteid_t i=1; i<=nsites; i++ ) {
		count_doc[i]			= 0;
		count_doc_ok[i]			= 0;
		count_doc_gathered[i]	= 0;
		count_doc_ignored[i]	= 0;
		count_doc_new[i]		= 0;
		count_doc_assigned[i]	= 0;
		count_doc_date_ok[i]	= 0;
		count_doc_static[i]		= 0;
		count_doc_dynamic[i]	= 0;
		max_depth[i]			= 0;
		age_oldest_page[i]		= 0;		// Very new
		age_newest_page[i]		= now;		// Very old, age=maximum
		age_average_page[i]		= 0;
		sum_pagerank[i]			= 0;
		sum_hubrank[i]			= 0;
		sum_authrank[i]			= 0;
	}

	// Go through the list of docs
	docid_t ndocs = metaidx->count_doc;
	docid_t ndocs_div_50	= ndocs / 50;

	cerr << "Pass 1/2: read docs |--------------------------------------------------|" << endl;
	cerr << "                     ";
	doc_t	doc;
	siteid_t	siteid = 0;
    for( docid_t docid=1; docid<=ndocs; docid++ ) {
		doc.docid	= docid;
		metaidx_doc_retrieve( metaidx, &(doc) );
		assert( doc.docid == docid );

		if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
			cerr << ".";
		}

		// Skip ignored documents
		if( doc.status == STATUS_DOC_IGNORED ) {
			count_doc_ignored[siteid] ++;

		} else {

			siteid	= doc.siteid;
			assert( siteid > 0 && siteid <= nsites );
			count_doc[siteid]	++;

			if( doc.is_dynamic ) {
				count_doc_dynamic[siteid] ++;
			} else {
				count_doc_static[siteid] ++;
			}

			// Count documents by status
			if( doc.status == STATUS_DOC_GATHERED ) {
				count_doc_gathered[siteid] ++;
			} else if( doc.status == STATUS_DOC_ASSIGNED ) {
				count_doc_assigned[siteid] ++;
			} else if( doc.status == STATUS_DOC_NEW ) {
				count_doc_new[siteid] ++;
			} else {
				assert( doc.status == STATUS_DOC_EXCLUSION );
			}

			// Link scores
			sum_pagerank[siteid]	+= doc.pagerank;
			sum_hubrank[siteid]		+= doc.hubrank;
			sum_authrank[siteid]	+= doc.authrank;

			// Only consider OK documents
			if( HTTP_IS_OK( doc.http_status ) ) {
				count_doc_ok[siteid] ++;

				if( doc.depth > max_depth[siteid] ) {
					max_depth[siteid]	= doc.depth;
				}

				// Only consider valid dates, i.e.:
				// they can be at most 1 day in the future,
				// considering date/time
				// 86400 = 60 secs/min x 60 mins/hr x 24 hrs/day

				if( ((doc.last_modified-86400) < doc.last_visit)
						&& doc.last_modified > 0 ) {

					// Fix last modified time
					if( doc.last_modified > doc.last_visit ) {
						doc.last_modified = doc.last_visit;
					}

					uint count	= count_doc_date_ok[siteid];
					count_doc_date_ok[siteid]	++;

					if( count == 0 ) {
						age_average_page[siteid]	= (double)(doc.last_visit - doc.last_modified);
					} else {
						age_average_page[siteid]	= (age_average_page[siteid] * ((double)count/(double)(count+1))) + (age_average_page[siteid] * ((double)1/(double)(count+1)));
					}

					if( age_newest_page[siteid] > (doc.last_visit - doc.last_modified) ) {
						// This is newer
						age_newest_page[siteid] = doc.last_visit - doc.last_modified;
					} 

					if( age_oldest_page[siteid] < (doc.last_visit - doc.last_modified ) ) {
						// This is older
						age_oldest_page[siteid] = doc.last_visit - doc.last_modified;
					}

				}
			}
		}

	}
	cerr << endl;

	// For summary statistics
	map<site_status_t, siteid_t> site_status_map;
	map<docid_t,siteid_t> site_count_doc_map;
	map<uint,siteid_t> site_raw_content_length_mb;
	map<time_t,siteid_t> site_age_oldest_page_months;
	map<time_t,siteid_t> site_age_newest_page_months;
	map<time_t,siteid_t> site_age_average_page_months;

	map<time_t,siteid_t> site_age_oldest_page_years;
	map<time_t,siteid_t> site_age_newest_page_years;
	map<time_t,siteid_t> site_age_average_page_years;

	map<docid_t,siteid_t> site_internal_links;
	map<uint,siteid_t> site_internal_links_by_ndocs;
	map<siteid_t,siteid_t> site_in_degree;
	map<siteid_t,siteid_t> site_out_degree;

	map<depth_t,siteid_t> site_max_depth;

	map<double,docid_t>	site_sum_pagerank;
	map<double,docid_t> site_sum_hubrank;
	map<double,docid_t> site_sum_authrank;
	map<double,docid_t> site_siterank;

	double sum_count_doc				= (double)0;
	double sum_count_doc_static			= (double)0;

	double sum_age_oldest_page_months	= (double)0;
	double sum_age_newest_page_months	= (double)0;
	double sum_age_average_page_months	= (double)0;

	double sum_in_degree				= (double)0;
	double sum_out_degree				= (double)0;
	double sum_internal_links			= (double)0;

	double sum_raw_content_length_mb	= (double)0;
	double sum_max_depth				= (double)0;

	siteid_t nsites_age_valid			= (siteid_t)0;
	siteid_t nsites_ok					= (siteid_t)0;

	cerr << "Pass 2/2: write sites |--------------------------------------------------|" << endl;
	cerr << "                       ";
	siteid_t nsites_div_50 = nsites/50;
	site_t site;
	for( site.siteid=1; site.siteid<=nsites; site.siteid++ ) {

        if( nsites_div_50 > 0 && site.siteid % nsites_div_50 == 0 ) {
            cerr << ".";
		}
		// Read
		metaidx_site_retrieve( metaidx, &(site) );

		// Add statistics
		site.count_doc			= count_doc[site.siteid];
		site.count_doc_ok		= count_doc_ok[site.siteid];
		site.count_doc_gathered	= count_doc_gathered[site.siteid];
		site.count_doc_ignored	= count_doc_ignored[site.siteid];
		site.count_doc_new		= count_doc_new[site.siteid];
		site.count_doc_assigned	= count_doc_assigned[site.siteid];
		site.count_doc_static	= count_doc_static[site.siteid];
		site.count_doc_dynamic	= count_doc_dynamic[site.siteid];
		site.max_depth			= max_depth[site.siteid];

		site.sum_pagerank		= sum_pagerank[site.siteid];
		site.sum_hubrank		= sum_hubrank[site.siteid];
		site.sum_authrank		= sum_authrank[site.siteid];

		if( count_doc_date_ok[site.siteid] > 0 ) {
			site.age_oldest_page	= age_oldest_page[site.siteid];
			site.age_newest_page	= age_newest_page[site.siteid];
			site.age_average_page	= (time_t)(age_average_page[site.siteid]);

			site_age_oldest_page_months[ site.age_oldest_page / 2592000] ++;
			site_age_newest_page_months[ site.age_newest_page / 2592000] ++;
			site_age_average_page_months[ site.age_average_page / 2592000] ++;

			site_age_oldest_page_years[ site.age_oldest_page / 31536000] ++;
			site_age_newest_page_years[ site.age_newest_page / 31536000] ++;
			site_age_average_page_years[ site.age_average_page / 31536000] ++;

			nsites_age_valid			++;

			sum_age_oldest_page_months	+= ( site.age_oldest_page / 2592000 );
			sum_age_newest_page_months	+= ( site.age_newest_page / 2592000 );
			sum_age_average_page_months	+= ( site.age_average_page / 2592000 );

		} else {
			site.age_oldest_page	= 0;
			site.age_newest_page	= 0;
			site.age_average_page	= 0;
		}

		// Write
		metaidx_site_store( metaidx, &(site) );

		// Get site statistics
		site_status_map[site.status] ++;

		if( site.count_doc_ok >= 1 ) {
			nsites_ok		++;

			site_count_doc_map[site.count_doc] ++;
			site_raw_content_length_mb[(uint)rint((off64_t)site.raw_content_length / (off64_t)1048576)] ++;

			site_internal_links[site.internal_links] ++;

			uint internal_links_by_ndocs	= (uint)0;
			if( site.count_doc_ok > 0 ) {
				internal_links_by_ndocs		= (uint)rint( (double)site.internal_links * (double)10 / (double)site.count_doc_ok );
			}
			site_internal_links_by_ndocs[ internal_links_by_ndocs ] ++; 
			site_in_degree[site.in_degree] ++;
			site_out_degree[site.out_degree] ++;
			site_max_depth[site.max_depth] ++;

			// Sum of link scores
			site_sum_pagerank[ ROUND_DOUBLE( site.sum_pagerank ) ] ++;
			site_sum_hubrank[ ROUND_DOUBLE( site.sum_hubrank ) ] ++;
			site_sum_authrank[ ROUND_DOUBLE( site.sum_authrank ) ] ++;
			site_siterank[ ROUND_DOUBLE( site.siterank ) ] ++;

			// Calculate sums for summary stats
			sum_count_doc				+= (double)(site.count_doc);
			sum_count_doc_static		+= (double)(site.count_doc_static);
			sum_in_degree				+= (double)(site.in_degree);
			sum_out_degree				+= (double)(site.out_degree);
			sum_internal_links			+= (double)(site.internal_links);

			sum_raw_content_length_mb	+= (double)((off64_t)site.raw_content_length / (off64_t)1048576);

			sum_max_depth				+= (double)(site.max_depth);
		}
	}

	// Create dated directory for output
	cerr << "* Writing summary statistics" << endl;

	char basename[MAX_STR_LEN];
	strcpy( basename, "site" );

	char filename[MAX_STR_LEN];
	sprintf( filename, "%s/%s", COLLECTION_ANALYSIS, basename );
	createdir( filename );

	cerr << "Output dir    : " << filename << endl;
	FILE *output;

	// Summary
	sprintf( filename, "%s/%s/site_summary.csv", COLLECTION_ANALYSIS, basename );
	cerr << "- Summary info -> " << filename << endl;
	output = fopen64( filename, "w" );
	fprintf( output, "Number of sites ok,%lu\n", nsites_ok );
	fprintf( output, "Number of sites with valid page age,%lu\n", nsites_age_valid );
	fprintf( output, "Average pages per site,%e\n", (sum_count_doc / nsites_ok) );
	fprintf( output, "Average static pages per site,%e\n", (sum_count_doc_static / nsites_ok) );
	fprintf( output, "Average dynamic pages per site,%e\n", ( (sum_count_doc-sum_count_doc_static) / nsites_ok) );
	fprintf( output, "Average of age of oldest page in months,%e\n", (sum_age_oldest_page_months / nsites_age_valid) );
	fprintf( output, "Average of age of newest page in months,%e\n", (sum_age_newest_page_months / nsites_age_valid) );
	fprintf( output, "Average of age of average page in months,%e\n", (sum_age_average_page_months / nsites_age_valid) );
	fprintf( output, "Average in-degree,%e\n", (sum_in_degree / nsites_ok) );
	fprintf( output, "Average out-degree,%e\n", (sum_out_degree / nsites_ok) );
	fprintf( output, "Average internal links,%e\n", (sum_internal_links / nsites_ok) );
	fprintf( output, "Average site size in MB,%e\n", (sum_raw_content_length_mb / nsites_ok) );
	fprintf( output, "Average site max depth,%e\n", (sum_max_depth / nsites_ok ) );
	fclose( output );

	// Site status
	sprintf( filename, "%s/%s/site_status.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Site status,Sites,Fraction\n" );
	cerr << "- Site Status -> " << filename << endl;
	map<site_status_t, siteid_t>::iterator siteit;
	for( siteit = site_status_map.begin(); siteit != site_status_map.end();
			siteit++ ) {
		fprintf( output, "%s,%lu,%e\n", SITE_STATUS_STR((*siteit).first), (*siteit).second, ((double)((*siteit).second) / (double)nsites)  );
	}
	fclose( output );

	// Document count
	sprintf( filename, "%s/%s/site_count_doc.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Number of documents,Sites,Fraction\n" );
	cerr << "- Site doc count -> " << filename << endl;
	map<docid_t, siteid_t>::iterator site_count_doc_it;
	for( site_count_doc_it = site_count_doc_map.begin(); site_count_doc_it != site_count_doc_map.end();
			site_count_doc_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_count_doc_it).first, (*site_count_doc_it).second, ((double)((*site_count_doc_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Raw content length
	sprintf( filename, "%s/%s/site_raw_content_length_mb.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Raw content length in MB,Sites,Fraction\n" );
	cerr << "- Raw content length in MB -> " << filename << endl;
	map<uint, siteid_t>::iterator site_raw_content_length_mb_it;
	for( site_raw_content_length_mb_it = site_raw_content_length_mb.begin(); site_raw_content_length_mb_it != site_raw_content_length_mb.end(); site_raw_content_length_mb_it++ ) {
		fprintf( output, "%d,%lu,%e\n",
			(*site_raw_content_length_mb_it).first,
			(*site_raw_content_length_mb_it).second,
			((double)((*site_raw_content_length_mb_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Age oldest page
	sprintf( filename, "%s/%s/site_age_oldest_page_months.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age oldest page in months,Sites,Fraction\n" );
	cerr << "- Age oldest page months -> " << filename << endl;
	map<time_t, siteid_t>::iterator site_age_oldest_page_months_it;
	for( site_age_oldest_page_months_it = site_age_oldest_page_months.begin(); site_age_oldest_page_months_it != site_age_oldest_page_months.end();
			site_age_oldest_page_months_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_age_oldest_page_months_it).first, (*site_age_oldest_page_months_it).second, ((double)((*site_age_oldest_page_months_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Age newest page
	sprintf( filename, "%s/%s/site_age_newest_page_months.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age newest page in months,Sites,Fraction\n" );
	cerr << "- Age newest page months -> " << filename << endl;
	map<time_t, siteid_t>::iterator site_age_newest_page_months_it;
	for( site_age_newest_page_months_it = site_age_newest_page_months.begin(); site_age_newest_page_months_it != site_age_newest_page_months.end();
			site_age_newest_page_months_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_age_newest_page_months_it).first, (*site_age_newest_page_months_it).second, ((double)((*site_age_newest_page_months_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Age average page
	sprintf( filename, "%s/%s/site_age_average_page_months.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age average page in months,Sites,Fraction\n" );
	cerr << "- Age average page months -> " << filename << endl;
	map<time_t, siteid_t>::iterator site_age_average_page_months_it;
	for( site_age_average_page_months_it = site_age_average_page_months.begin(); site_age_average_page_months_it != site_age_average_page_months.end();
			site_age_average_page_months_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_age_average_page_months_it).first, (*site_age_average_page_months_it).second, ((double)((*site_age_average_page_months_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Age oldest page
	sprintf( filename, "%s/%s/site_age_oldest_page_years.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age oldest page in years,Sites,Fraction\n" );
	cerr << "- Age oldest page years -> " << filename << endl;
	map<time_t, siteid_t>::iterator site_age_oldest_page_years_it;
	for( site_age_oldest_page_years_it = site_age_oldest_page_years.begin(); site_age_oldest_page_years_it != site_age_oldest_page_years.end();
			site_age_oldest_page_years_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_age_oldest_page_years_it).first, (*site_age_oldest_page_years_it).second, ((double)((*site_age_oldest_page_years_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Age newest page
	sprintf( filename, "%s/%s/site_age_newest_page_years.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age newest page in years,Sites,Fraction\n" );
	cerr << "- Age newest page years -> " << filename << endl;
	map<time_t, siteid_t>::iterator site_age_newest_page_years_it;
	for( site_age_newest_page_years_it = site_age_newest_page_years.begin(); site_age_newest_page_years_it != site_age_newest_page_years.end();
			site_age_newest_page_years_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_age_newest_page_years_it).first, (*site_age_newest_page_years_it).second, ((double)((*site_age_newest_page_years_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Age average page
	sprintf( filename, "%s/%s/site_age_average_page_years.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age average page in years,Sites,Fraction\n" );
	cerr << "- Age average page years -> " << filename << endl;
	map<time_t, siteid_t>::iterator site_age_average_page_years_it;
	for( site_age_average_page_years_it = site_age_average_page_years.begin(); site_age_average_page_years_it != site_age_average_page_years.end();
			site_age_average_page_years_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_age_average_page_years_it).first, (*site_age_average_page_years_it).second, ((double)((*site_age_average_page_years_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Internal links
	sprintf( filename, "%s/%s/site_internal_links.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Internal links,Sites,Fraction\n" );
	cerr << "- Internal links -> " << filename << endl;
	map<docid_t, siteid_t>::iterator site_internal_links_it;
	for( site_internal_links_it = site_internal_links.begin(); site_internal_links_it != site_internal_links.end();
			site_internal_links_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_internal_links_it).first, (*site_internal_links_it).second, ((double)((*site_internal_links_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Internal links by ndocs
	sprintf( filename, "%s/%s/site_internal_links_by_ndocs.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Internal links by page,Sites,Fraction\n" );
	cerr << "- Internal links by ndocs-> " << filename << endl;
	map<uint, siteid_t>::iterator site_internal_links_by_ndocs_it;
	for( site_internal_links_by_ndocs_it = site_internal_links_by_ndocs.begin(); site_internal_links_by_ndocs_it != site_internal_links_by_ndocs.end();
			site_internal_links_by_ndocs_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (double)((*site_internal_links_by_ndocs_it).first) / (double)10, (*site_internal_links_by_ndocs_it).second, ((double)((*site_internal_links_by_ndocs_it).second) / (double)nsites)  );
	}
	fclose( output );

	// In degree
	sprintf( filename, "%s/%s/site_in_degree.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Incoming links,Sites,Fraction\n" );
	cerr << "- Incoming links -> " << filename << endl;
	map<siteid_t, siteid_t>::iterator site_in_degree_it;
	for( site_in_degree_it = site_in_degree.begin(); site_in_degree_it != site_in_degree.end();
			site_in_degree_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_in_degree_it).first, (*site_in_degree_it).second, ((double)((*site_in_degree_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Out degree
	sprintf( filename, "%s/%s/site_out_degree.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Outgoing links,Sites,Fraction\n" );
	cerr << "- Outgoing links -> " << filename << endl;
	map<siteid_t, siteid_t>::iterator site_out_degree_it;
	for( site_out_degree_it = site_out_degree.begin(); site_out_degree_it != site_out_degree.end();
			site_out_degree_it++ ) {
		fprintf( output, "%lu,%lu,%e\n", (*site_out_degree_it).first, (*site_out_degree_it).second, ((double)((*site_out_degree_it).second) / (double)nsites)  );
	}
	fclose( output );

	// Max depth
	sprintf( filename, "%s/%s/site_max_depth.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Max depth,Sites,Fraction\n" );
	cerr << "- Max depth -> " << filename << endl;
	map<depth_t, siteid_t>::iterator site_max_depth_it;
	for( site_max_depth_it = site_max_depth.begin(); site_max_depth_it != site_max_depth.end();
			site_max_depth_it++ ) {
		fprintf( output, "%d,%lu,%e\n", (*site_max_depth_it).first, (*site_max_depth_it).second, ((double)((*site_max_depth_it).second) / (double)nsites)  );
	}
	fclose( output );

	// page rank
	sprintf( filename, "%s/%s/sum_pagerank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Sum of pagerank,Documents,Fraction\n" );

	cerr << "- Sum of pagerank > " << filename << endl;
	map<pagerank_t,docid_t>::iterator sum_pagerank_it;
	for( sum_pagerank_it = site_sum_pagerank.begin(); sum_pagerank_it != site_sum_pagerank.end();
			sum_pagerank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*sum_pagerank_it).first, (*sum_pagerank_it).second, (double)((*sum_pagerank_it).second) / (double)ndocs );
	}
	fclose( output );

	// hubrank
	sprintf( filename, "%s/%s/sum_hubrank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Sum of hubrank,Documents,Fraction\n" );

	cerr << "- Sum of hubrank > " << filename << endl;
	map<hubrank_t,docid_t>::iterator sum_hubrank_it;
	for( sum_hubrank_it = site_sum_hubrank.begin(); sum_hubrank_it != site_sum_hubrank.end();
			sum_hubrank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*sum_hubrank_it).first, (*sum_hubrank_it).second, (double)((*sum_hubrank_it).second) / (double)ndocs );
	}
	fclose( output );

	// authrank
	sprintf( filename, "%s/%s/sum_authrank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Sum of authrank,Documents,Fraction\n" );

	cerr << "- Sum of authrank > " << filename << endl;
	map<authrank_t,docid_t>::iterator sum_authrank_it;
	for( sum_authrank_it = site_sum_authrank.begin(); sum_authrank_it != site_sum_authrank.end();
			sum_authrank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*sum_authrank_it).first, (*sum_authrank_it).second, (double)((*sum_authrank_it).second) / (double)ndocs );
	}
	fclose( output );

	// authrank
	sprintf( filename, "%s/%s/siterank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Siterank,Documents,Fraction\n" );

	cerr << "- Siterank > " << filename << endl;
	map<authrank_t,docid_t>::iterator siterank_it;
	for( siterank_it = site_siterank.begin(); siterank_it != site_siterank.end();
			siterank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*siterank_it).first, (*siterank_it).second, (double)((*siterank_it).second) / (double)ndocs );
	}
	fclose( output );

	// Free memory
	// Free memory
	free(count_doc);
	free(count_doc_gathered);
	free(count_doc_ok);
	free(count_doc_date_ok);
	free(count_doc_static);
	free(count_doc_dynamic);
	free(max_depth);
	free(age_oldest_page);
	free(age_newest_page);
	free(age_average_page);

	free(sum_pagerank);
	free(sum_hubrank);
	free(sum_authrank);

	cerr << endl;
}

//
// Name: metaidx_doc_calculate_scores
//
// Description:
//   Calculates all scores
//
// Input:
//   doc - the document object
//   siterank - the ranking of the site
//
// Output:
//   doc.freshness - freshness
//

void metaidx_doc_calculate_scores( docid_t ndocs, siteid_t nsites, doc_t *doc, siterank_t siterank, docid_t queuesize ) {
	// Base score is 0
	doc->current_score = (priority_t)(0);
	doc->future_score = (priority_t)(0);

	// Duplicates have zero score
	if( doc->duplicate_of > (docid_t)0 ) {
		return;
	} else if( doc->status == STATUS_DOC_IGNORED ) {
		return;
	}

	// Calculate Freshness
	doc->freshness = metaidx_doc_freshness( doc );

	// Depth
	depth_t max_depth;
	max_depth = CONF_MANAGER_MAXDEPTH_STATIC;

	priority_t depth_score;
	if( doc->depth > max_depth ) {
		// Pages too deep have zero depth_score
		depth_score = (priority_t)0;
	} else {
		// Score is proportional to depth
		// if depth=1, then score=1,
		// if depth=max_depth, score=1/max_depth
		depth_score = ((priority_t)max_depth - ((priority_t)doc->depth - (priority_t)1)) / (priority_t)max_depth;
	}

	// Scores
	priority_t static_score	= (doc->is_dynamic?0:1);
	priority_t random_score	= ((priority_t)rand()/(priority_t)RAND_MAX);

	doc->future_score = 
			  CONF_MANAGER_SCORE_PAGERANK_WEIGHT  * doc->pagerank
			+ CONF_MANAGER_SCORE_WLSCORE_WEIGHT    * doc->wlrank
			+ CONF_MANAGER_SCORE_HITS_HUB_WEIGHT   * doc->hubrank
			+ CONF_MANAGER_SCORE_HITS_AUTHORITY_WEIGHT  * doc->authrank
			+ CONF_MANAGER_SCORE_SITERANK_WEIGHT  * siterank
			+ CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT  * queuesize
			+ CONF_MANAGER_SCORE_DEPTH_WEIGHT 	* depth_score
			+ CONF_MANAGER_SCORE_STATIC_WEIGHT	* static_score
			+ CONF_MANAGER_SCORE_RANDOM_WEIGHT	* random_score;

	// Boost the score of the front page.
	// This will encourage the crawler to visit the first page of
	// each site often, and will help the discovery of new pages
	if( CONF_MANAGER_SCORE_BOOSTFRONTPAGE &&
	 (doc->depth == (depth_t)1 ) ) {
		doc->future_score = CONF_MANAGER_SCORE_PAGERANK_WEIGHT + CONF_MANAGER_SCORE_WLSCORE_WEIGHT + CONF_MANAGER_SCORE_HITS_HUB_WEIGHT + CONF_MANAGER_SCORE_HITS_AUTHORITY_WEIGHT + CONF_MANAGER_SCORE_SITERANK_WEIGHT + CONF_MANAGER_SCORE_QUEUESIZE_WEIGHT + CONF_MANAGER_SCORE_DEPTH_WEIGHT + CONF_MANAGER_SCORE_STATIC_WEIGHT + CONF_MANAGER_SCORE_RANDOM_WEIGHT;

	}

	// Check freshness of the documents
	if( doc->freshness == (double)0 ) {
		doc->current_score = (priority_t)0;
	} else {
		if( CONF_MANAGER_SCORE_AGE_EXPONENT == 1 ) {
			doc->current_score = doc->future_score * (double)(doc->freshness);
		} else {
			doc->current_score = doc->future_score
				* (priority_t)pow( (double)doc->freshness, (double)CONF_MANAGER_SCORE_AGE_EXPONENT );
		}
	}
}

//
// Name: metaidx_doc_freshness
//
// Description:
//   Gets the freshness of a page as the probability that the
//   local copy equals the remote copy
//   From: Cho & Garcia-Molina: Estimating Frequency of Change 2000
//
// Input:
//   doc - the document object
//
// Output:
//   the freshness (1=updated, 0=old)
//

freshness_t metaidx_doc_freshness( doc_t *doc ) {
	assert( doc != NULL );
	assert( doc->docid > 0 );
	assert( doc->docid < CONF_COLLECTION_MAXDOC );

	// Check simple cases
	if( doc->status == STATUS_DOC_IGNORED ) {
		return 0;

	} else if( doc->number_visits == 0 ) {
		// Unvisited, it's not fresh
		return 0;

	} else if( doc->number_visits > 1 &&
			(doc->number_visits_changed == doc->number_visits) ) {
		// Always change
		return 0;
	} else if( ! HTTP_IS_OK( doc->http_status ) ) {
		// Last visit failed
		return 0;
	} else if( doc->last_modified > doc->last_visit ) {
		// We have last-modified information for this document,
		// provided by another mechanism (e.g.: robots.rdf file)
		return 0;
	}

	// Visited page
	time_t now = time(NULL);
	double lambda;
	
	// Check if we have last modified data
	if( doc->last_modified > 0 ) {

		// Last_modified available, check how many visits
		if( doc->number_visits == 1 ) {

			// Only one visit
			lambda = (double)1 / (double)(doc->last_visit - doc->last_modified);

		} else {

			// More than one visit
			lambda = (((double)doc->number_visits_changed - (double)1) - ((double)doc->number_visits_changed/((double)doc->number_visits * log((double)1 - ((double)doc->number_visits_changed/(double)doc->number_visits)))))/(((double)now - (double)doc->first_visit) * ((double)doc->time_unchanged));

			if( lambda == 0 ) {
				cerr << "Error calculating freshness for document " << doc->docid << endl;
				die( "lambda = 0" );
			}

		}

	} else if( doc->number_visits_changed < doc->number_visits ) {

		// No last_modified data, but there is some data
		// about change (ie.: text analysis)

		lambda = (-((double)doc->number_visits) * log((double)1 - ((double)doc->number_visits_changed / (double)doc->number_visits)))/((double)now - (double)doc->first_visit);

	} else {
		return 0;
	}

	// Return probability
	return exp( -(lambda) * (double)(now - doc->last_visit) );
}

//
// Name: metaidx_analyze_docs
//
// Description:
//   Analyzes and extract statistics about documents, writes
//   them into the analysis directory
//
// Input:
//   metaidx - the metadata structure
//   status - the status (ALL/NEW/GATHERED) of the documents
//   static/dynamic - 0=all, 1=static, 2=dynamic
//
// Output:
//   multiple files are generated in the analysis directory
//

void metaidx_analysis_doc_statistics( metaidx_t *metaidx, doc_status_t status, uint static_dynamic ) {
	assert( metaidx != NULL );
	assert( static_dynamic >= 0 && static_dynamic <= 2 );

	metaidx_status_t rc;
	char static_dynamic_string[MAX_STR_LEN];
	char status_string[MAX_STR_LEN];
	char basename[MAX_STR_LEN];
	FILE *output;

	// Get a string to show the static/dynamic filter type
	strcpy( static_dynamic_string,
			 static_dynamic == 0 ? "all"
		   : static_dynamic == 1 ? "static"
		   : static_dynamic == 2 ? "dynamic"
		   : "(invalid)" );

	// Get a string to show the status filter type
	strcpy( status_string,
			status			== STATUS_DOC_ALL ? "all"
		  : status			== STATUS_DOC_GATHERED ? "gathered"
		  : status			== STATUS_DOC_NEW ? "all"
		  : "(invalid)" );

	// Get directory name
	sprintf( basename, "doc_%s_%s", status_string, static_dynamic_string );
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s/%s", COLLECTION_ANALYSIS, basename );

	// Create output directory
	createdir( COLLECTION_ANALYSIS );
	createdir( filename );

	cerr << endl;
	cerr << "* Generating statistics" << endl;
	cerr << "Status filter : " << status_string << endl;
	cerr << "Type filter   : " << static_dynamic_string << endl;
	cerr << "Output dir    : " << filename << endl;

	// Mappings for getting document information
	map<doc_status_t, docid_t> doc_status_map;
	map<int,docid_t> http_status_map;
	map<depth_t,docid_t> depth_count;

	map<uint,docid_t> in_degree_map;
	map<uint,docid_t> out_degree_map;
	map<uint,docid_t> content_length_kb_map;
	map<uint,docid_t> raw_content_length_kb_map;
	map<int,docid_t> mime_type_map;

	map<double,docid_t> pagerank_map;
	map<double,docid_t> wlrank_map;
	map<double,docid_t> hubrank_map;
	map<double,docid_t> authrank_map;

	map<depth_t,pagerank_t> pagerank_depth;
	map<depth_t,wlrank_t> wlrank_depth;
	map<depth_t,hubrank_t> hubrank_depth;
	map<depth_t,authrank_t> authrank_depth;

	map<time_t,docid_t> age_hours_map;
	map<time_t,docid_t> age_months_map;
	map<time_t,docid_t> age_years_map;

	// Mapping for getting summary information
	docid_t	duplicates_count= 0;
	docid_t dynamic_count	= 0;
	docid_t doc_count		= 0;
	docid_t ndocs			= metaidx->count_doc;

	// Filename for the sample
	sprintf( filename, "%s/%s/doc_sample.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	assert( output != NULL );
	cerr << "Sample of 1/" << CONF_ANALYSIS_DOC_SAMPLE_EVERY << " of the documents -> " << filename << endl;
	metaidx_dump_doc_header( output );

	// Report
	docid_t ndocs_div_50	= (ndocs / 50);
	cerr << "Calculating    |--------------------------------------------------|" << endl;
	cerr << "                ";

	// Iterate through all the documents
	doc_t doc;
	for( docid_t docid=1; docid<=ndocs; docid++ ) {
		doc.docid	= docid;

		if( ndocs_div_50 > 0 && ( docid % ndocs_div_50 ) == 0 ) {
			cerr << ".";
		}

		rc = metaidx_doc_retrieve( metaidx, &(doc) );
		assert( rc == METAIDX_OK );
		assert( doc.docid == docid );

		// Ignored documents are not considered for statistics
		if( doc.status == STATUS_DOC_IGNORED ) {
			continue;
		}

		// If a status was given, check
		// that the document has that status
		if(    status != STATUS_DOC_ALL
			&& doc.status != status ) {
			continue;
		}

		// If a static_dynamic filter was given, check type
		if( static_dynamic != 0 ) {
			if( doc.is_dynamic && static_dynamic == 1 ) {
				continue;
			} else if( (! doc.is_dynamic) && static_dynamic == 2 ) {
				continue;
			}
		}
		

		doc_count ++;

		// Show this document if necessary
		if( (doc_count % CONF_ANALYSIS_DOC_SAMPLE_EVERY) == 0 ) {
			metaidx_dump_doc( &(doc), output );
		}

		doc_status_map[doc.status] ++;
		http_status_map[doc.http_status] ++;
		depth_count[doc.depth] ++;
		in_degree_map[doc.in_degree] ++;
		out_degree_map[doc.out_degree] ++;
		content_length_kb_map[(uint)(doc.content_length/1024)] ++;
		raw_content_length_kb_map[(uint)(doc.raw_content_length/1024)] ++;
		mime_type_map[doc.mime_type] ++;

		if( doc.is_dynamic ) {
			dynamic_count ++;
		}

		if( doc.duplicate_of > 0 ) {
			duplicates_count ++;
		}

		// Link scores
		pagerank_map[ ROUND_DOUBLE(doc.pagerank)  ] ++;
		wlrank_map[ ROUND_DOUBLE(doc.wlrank) ] ++;
		hubrank_map[ ROUND_DOUBLE(doc.hubrank) ] ++;
		authrank_map[ ROUND_DOUBLE(doc.authrank) ] ++;

		pagerank_depth[ doc.depth ]	+= doc.pagerank;
		wlrank_depth[ doc.depth ]	+= doc.wlrank;
		hubrank_depth[ doc.depth ]	+= doc.hubrank;
		authrank_depth[ doc.depth ]	+= doc.authrank;

		// Last modified
		if( doc.last_visit > 0 && doc.last_modified > 0 ) {
			if( doc.last_visit <= doc.last_modified ) {
				age_hours_map[(time_t)0] ++;
				age_months_map[(time_t)0] ++;
				age_years_map[(time_t)0] ++;
			} else {
				// 60 seconds in 1 minute
				// 60 minutes in 1 hour
				// 1 hour = 3600 seconds
				//
				// 24 hours in 1 day
				// 30 days in 1 month
				// 1 hour = 2592000 seconds
				// 1 year = 31536000
				age_hours_map[(doc.last_visit - doc.last_modified)/((time_t)3600)] ++;
				age_months_map[(doc.last_visit - doc.last_modified)/((time_t)2592000)] ++;
				age_years_map[(doc.last_visit - doc.last_modified)/((time_t)31536000)] ++;
			}
		}
	}

	cerr << "done." << endl;
	fclose( output ); // Sample docs.


	// Write summary statistics

	sprintf( filename, "%s/%s/stats.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	cerr << "Summary stats -> " << filename << endl;
	fprintf( output, "Total,Dynamic,Duplicated\n" );
	fprintf( output, "%lu,%lu,%lu\n", doc_count, dynamic_count, duplicates_count );
	fclose( output );

	// Dump documents status

	sprintf( filename, "%s/%s/doc_status.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Document status,Document code,Documents,Fraction\n" );
	cerr << "- Document status -> " << filename << endl;
	map<doc_status_t, docid_t>::iterator docit;
	for( docit = doc_status_map.begin(); docit != doc_status_map.end();
			docit++ ) {
		fprintf( output, "%s,%d,%lu,%e\n", DOC_STATUS_STR((*docit).first), (*docit).first, (*docit).second, ((double)((*docit).second) / (double)ndocs)  );
	}
	fclose( output );

	// HTTP Codes

	sprintf( filename, "%s/%s/http_code.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "HTTP Status,HTTP Code,Documents,Fraction\n" );
	cerr << "- HTTP Codes -> " << filename << endl;
	map<int, docid_t>::iterator httpit;
	for( httpit = http_status_map.begin(); httpit != http_status_map.end();
			httpit++ ) {
		fprintf( output, "%s,%d,%lu,%e\n", HTTP_STR((*httpit).first), (*httpit).first, (*httpit).second, ((double)((*httpit).second) / (double)ndocs)  );

	}
	fclose( output );

	// Mime type

	sprintf( filename, "%s/%s/mime_type.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "MIME type,Internal code,Documents,Fraction\n" );
	cerr << "- MIME Types -> " << filename << endl;
	map<int, docid_t>::iterator mime_type_iterator;
	for( mime_type_iterator = mime_type_map.begin(); mime_type_iterator != mime_type_map.end();
			mime_type_iterator++ ) {
		fprintf( output, "%s,%d,%lu,%e\n", MIME_TYPE_STR((*mime_type_iterator).first), (*mime_type_iterator).first, (*mime_type_iterator).second, ((double)((*mime_type_iterator).second) / (double)ndocs)  );

	}
	fclose( output );

	// Depth

	sprintf( filename, "%s/%s/depth.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Document depth,Documents,Fraction\n" );

	cerr << "- Depth -> " << filename << endl;
	map<depth_t,docid_t>::iterator depthit;
	for( depthit = depth_count.begin(); depthit != depth_count.end();
			depthit++ ) {
		fprintf( output, "%d,%lu,%e\n", (*depthit).first, (*depthit).second, ((double)((*depthit).second) / (double)ndocs)  );

	}
	fclose( output );

	// Page rank
	sprintf( filename, "%s/%s/pagerank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Pagerank,Documents,Fraction\n" );

	cerr << "- Pagerank > " << filename << endl;
	map<pagerank_t,docid_t>::iterator pagerank_it;
	for( pagerank_it = pagerank_map.begin(); pagerank_it != pagerank_map.end();
			pagerank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*pagerank_it).first, (*pagerank_it).second, (double)((*pagerank_it).second) / (double)ndocs );
	}
	fclose( output );

	// Wl score
	sprintf( filename, "%s/%s/wlrank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Wirerank,Documents,Fraction\n" );

	cerr << "- Wirerank > " << filename << endl;
	map<wlrank_t,docid_t>::iterator wlrank_it;
	for( wlrank_it = wlrank_map.begin(); wlrank_it != wlrank_map.end();
			wlrank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*wlrank_it).first, (*wlrank_it).second, (double)((*wlrank_it).second) / (double)ndocs );
	}
	fclose( output );

	// Hub score
	sprintf( filename, "%s/%s/hubrank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Hub score,Documents,Fraction\n" );

	cerr << "- Hub score > " << filename << endl;
	map<hubrank_t,docid_t>::iterator hubrank_it;
	for( hubrank_it = hubrank_map.begin(); hubrank_it != hubrank_map.end();
			hubrank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*hubrank_it).first, (*hubrank_it).second, (double)((*hubrank_it).second) / (double)ndocs );
	}
	fclose( output );

	// Authority score
	sprintf( filename, "%s/%s/authrank.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Authority score,Documents,Fraction\n" );

	cerr << "- Authority score > " << filename << endl;
	map<authrank_t,docid_t>::iterator authrank_it;
	for( authrank_it = authrank_map.begin(); authrank_it != authrank_map.end();
			authrank_it++ ) {
		fprintf( output, "%e,%lu,%e\n", (*authrank_it).first, (*authrank_it).second, (double)((*authrank_it).second) / (double)ndocs );
	}
	fclose( output );

	// Page rank
	sprintf( filename, "%s/%s/pagerank_depth.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Depth,Pagerank,Cumulative\n" );

	cerr << "- Pagerank and depth > " << filename << endl;
	map<depth_t,pagerank_t>::iterator pagerank_depth_it;
	double cumulative	= 0;
	for( pagerank_depth_it = pagerank_depth.begin(); pagerank_depth_it != pagerank_depth.end();
			pagerank_depth_it++ ) {
		cumulative	+= (*pagerank_depth_it).second;
		fprintf( output, "%d,%e,%e\n", (*pagerank_depth_it).first, (*pagerank_depth_it).second, cumulative );
	}
	fclose( output );

	// WL rank
	sprintf( filename, "%s/%s/wlrank_depth.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Depth,WLrank,Cumulative\n" );

	cerr << "- WLrank and depth > " << filename << endl;
	map<depth_t,wlrank_t>::iterator wlrank_depth_it;
	cumulative	= 0;
	for( wlrank_depth_it = wlrank_depth.begin(); wlrank_depth_it != wlrank_depth.end();
			wlrank_depth_it++ ) {
		cumulative	+= (*wlrank_depth_it).second;
		fprintf( output, "%d,%e,%e\n", (*wlrank_depth_it).first, (*wlrank_depth_it).second, cumulative );
	}
	fclose( output );

	// Hub rank
	sprintf( filename, "%s/%s/hubrank_depth.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Depth,Hubrank,Cumulative\n" );

	cerr << "- Hubrank and depth > " << filename << endl;
	map<depth_t,hubrank_t>::iterator hubrank_depth_it;
	cumulative	= 0;
	for( hubrank_depth_it = hubrank_depth.begin(); hubrank_depth_it != hubrank_depth.end();
			hubrank_depth_it++ ) {
		cumulative	+= (*hubrank_depth_it).second;
		fprintf( output, "%d,%e,%e\n", (*hubrank_depth_it).first, (*hubrank_depth_it).second, cumulative );
	}
	fclose( output );

	// Auth rank
	sprintf( filename, "%s/%s/authrank_depth.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Depth,Authrank,Cumulative\n" );

	cerr << "- Authrank and depth > " << filename << endl;
	map<depth_t,authrank_t>::iterator authrank_depth_it;
	cumulative	= 0;
	for( authrank_depth_it = authrank_depth.begin(); authrank_depth_it != authrank_depth.end();
			authrank_depth_it++ ) {
		cumulative	+= (*authrank_depth_it).second;
		fprintf( output, "%d,%e,%e\n", (*authrank_depth_it).first, (*authrank_depth_it).second, cumulative );
	}
	fclose( output );

	// Age in hours
	sprintf( filename, "%s/%s/age_hours.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age in hours,Documents,Fraction\n" );

	cerr << "- Age in hours -> " << filename << endl;
	map<time_t,docid_t>::iterator age_hours_it;
	for( age_hours_it = age_hours_map.begin(); age_hours_it != age_hours_map.end();
			age_hours_it++ ) {
		fprintf( output, "%d,%lu,%e\n", (int)((*age_hours_it).first), (*age_hours_it).second, ((double)((*age_hours_it).second) / (double)ndocs)  );
	}
	fclose( output );

	// Age in months
	sprintf( filename, "%s/%s/age_months.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age in Months,Documents,Fraction\n" );

	cerr << "- Age in months -> " << filename << endl;
	map<time_t,docid_t>::iterator age_months_it;
	for( age_months_it = age_months_map.begin(); age_months_it != age_months_map.end();
			age_months_it++ ) {
		fprintf( output, "%d,%lu,%e\n", (int)((*age_months_it).first), (*age_months_it).second, ((double)((*age_months_it).second) / (double)ndocs)  );
	}
	fclose( output );

	// Age in years
	sprintf( filename, "%s/%s/age_years.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Age in Years,Documents,Fraction\n" );

	cerr << "- Age in years -> " << filename << endl;
	map<time_t,docid_t>::iterator age_years_it;
	for( age_years_it = age_years_map.begin(); age_years_it != age_years_map.end();
			age_years_it++ ) {
		fprintf( output, "%d,%lu,%e\n", (int)((*age_years_it).first), (*age_years_it).second, ((double)((*age_years_it).second) / (double)ndocs)  );
	}
	fclose( output );

	// IN Degree

	sprintf( filename, "%s/%s/in_degree.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "In-degree,Documents,Fraction\n" );

	cerr << "- Indegree -> " << filename << endl;
	map<uint,docid_t>::iterator indegreeit;
	for( indegreeit = in_degree_map.begin(); indegreeit != in_degree_map.end();
			indegreeit++ ) {
		fprintf( output, "%d,%lu,%e\n", (*indegreeit).first, (*indegreeit).second, ((double)((*indegreeit).second) / (double)ndocs)  );
	}
	fclose( output );

	// OUT Degree

	sprintf( filename, "%s/%s/out_degree.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Out-degree,Documents,Fraction\n" );
	cerr << "- Outdegree -> " << filename << endl;
	map<uint,docid_t>::iterator outdegreeit;
	for( outdegreeit = out_degree_map.begin(); outdegreeit != out_degree_map.end();
			outdegreeit++ ) {
		fprintf( output, "%d,%lu,%e\n", (*outdegreeit).first, (*outdegreeit).second, ((double)((*outdegreeit).second) / (double)ndocs)  );
	}
	fclose( output );

	// Content length, in KB

	sprintf( filename, "%s/%s/content_length_kb.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Parsed content length in KB,Documents,Fraction\n" );
	cerr << "- Content length in KB -> " << filename << endl;
	map<uint,docid_t>::iterator content_length_kb_iterator;
	for( content_length_kb_iterator = content_length_kb_map.begin(); content_length_kb_iterator != content_length_kb_map.end();
			content_length_kb_iterator++ ) {
		fprintf( output, "%d,%lu,%e\n", (*content_length_kb_iterator).first, (*content_length_kb_iterator).second, ((double)((*content_length_kb_iterator).second) / (double)ndocs)  );
	}
	fclose( output );

	// RAW Content length, in KB

	sprintf( filename, "%s/%s/raw_content_length_kb.csv", COLLECTION_ANALYSIS, basename );
	output = fopen64( filename, "w" );
	fprintf( output, "Content length in KB,Documents,Fraction\n" );
	cerr << "- RAW content length in KB -> " << filename << endl;
	map<uint,docid_t>::iterator raw_content_length_kb_iterator;
	for( raw_content_length_kb_iterator = raw_content_length_kb_map.begin(); raw_content_length_kb_iterator != raw_content_length_kb_map.end();
			raw_content_length_kb_iterator++ ) {
		fprintf( output, "%d,%lu,%e\n", (*raw_content_length_kb_iterator).first, (*raw_content_length_kb_iterator).second, ((double)((*raw_content_length_kb_iterator).second) / (double)ndocs)  );
	}
	fclose( output );

	cerr << "All stats calculated." << endl;

}
