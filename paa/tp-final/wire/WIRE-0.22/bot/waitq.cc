
#include "waitq.h"

//
// Name: waitq_create
//
// Description:
//   Creates a waiting queue
//
// Input:
//    maxservers - the maximum number of servers
//
// Return:
//    the created structure
//

waitq_t *waitq_create( uint maxservers ) {
	assert( maxservers > 0 );

	waitq_t *waitq = (waitq_t *)malloc(sizeof(waitq_t));
	assert( waitq != NULL );

	waitq->nservers		= 0;
	waitq->maxservers	= maxservers;
	waitq->queue		= (waitq_item_t *)malloc(sizeof(waitq_item_t)*maxservers);
	assert( waitq->queue != NULL );

	return waitq;
}

//
// Name: waitq_push
//
// Description:
//   Inserts a server; implemented using a priority queue
//
// Input:
//   waitq - the structure
//   serverid - the serverid to insert
//

void waitq_push( waitq_t *waitq, uint serverid, time_t nextvisit ) {
	assert( waitq != NULL );
	assert( serverid > 0 );
	assert( waitq->nservers < waitq->maxservers );

	uint n = waitq->nservers;

	// Check that the item is not in the heap,
/* Disabled
	for( uint i=0; i<n; i++ ) {
		if( waitq->queue[i].serverid == serverid ) {
			cerr << "Serverid " << serverid << " already inserted" << endl;
			die( "Attempt to insert duplicate in waitq" );
		}
	}
*/

	// Put item in the last position of the heap
	waitq->nservers++;

	waitq->queue[n].serverid	= serverid;
	waitq->queue[n].nextvisit	= nextvisit;

	// Adjust heap, so the nextvisit of a parent node is less than
	// the nextvisit of the children nodes
	uint i = n + 1;
	while( i > 1 ) {
		int p = i/2;
		if( waitq->queue[i-1].nextvisit < waitq->queue[p-1].nextvisit ) {
			waitq_swap(waitq, p-1, i-1);
			i = p;
		} else {
			break;
		}
	}

}

//
// Name: waitq_swap
//
// Description:
//   swaps to elements in the waitq
//
// Input:
//   waitq - the structure
//   a,b - indexes of the two elements to swap
//

void waitq_swap( waitq_t *waitq, uint a, uint b ) {
	assert( waitq != NULL );
	assert( waitq->nservers <= waitq->maxservers );
	assert( b <= waitq->nservers );
	assert( a < b );

	uint tmp_serverid		= waitq->queue[a].serverid;
	time_t tmp_nextvisit	= waitq->queue[a].nextvisit;

	waitq->queue[a].serverid	= waitq->queue[b].serverid;
	waitq->queue[a].nextvisit	= waitq->queue[b].nextvisit;

	waitq->queue[b].serverid	= tmp_serverid;
	waitq->queue[b].nextvisit	= tmp_nextvisit;
}


//
// Name: waitq_pop
//
// Description:
//   Gets the next server and dequeue
//
// Input:
//   waitq - the structure
//
// Return:
//   The next server
//

void waitq_pop( waitq_t *waitq, uint *serverid, time_t *nextvisit ) {
	assert( waitq != NULL );
	assert( waitq->nservers > 0 );
	assert( waitq->queue != NULL );

	// Copy the data of the topmost node
	(*serverid)		= waitq->queue[0].serverid;
	(*nextvisit)	= waitq->queue[0].nextvisit;


	// Special case: single node
	if( waitq->nservers == 1 ) {
		waitq->nservers = 0;
		return;
	}

	// There are nodes in the heap, swap the top and bottom
	uint n = waitq->nservers;
	waitq_swap( waitq, 0, n-1 );
	waitq->nservers--;

	// Adjust the heap
	uint i = 1;
	while( (i*2) < n ) {
		uint c;
		if( (i*2) >= n ) {
			// No childs
			break;
		} else if( ((i*2) + 1) >= n ) {
			// Parent of a single child
			c = i * 2;
		} else if( waitq->queue[ (i*2)-1 ].nextvisit < 
				   waitq->queue[ (i*2)   ].nextvisit ) {
			// The left child is lower
			c = i * 2;
		} else {
			// The right child is lower
			c = (i*2) + 1;
		}

		// Now test
		if( waitq->queue[i-1].nextvisit > waitq->queue[c-1].nextvisit ) {
			waitq_swap( waitq, i-1, c-1 );
			i = c;
		} else {
			break;
		}
	}

}

//
// Name: waitq_empty
//
// Description:
//   Checks if the queue is empty
//
// Input:
//   waitq - the structure
//
// Return:
//   1 if the queue is empty
//   0 if the queue has elements
//

int waitq_empty( waitq_t *waitq ) {
	assert( waitq != NULL );
	if( waitq->nservers == 0 ) {
		return 1;
	} else {
		return 0;
	}
}

//
// Name: waitq_available
//
// Description:
//   Checks if we have servers.
//   This is not the same as waitq_empty, because
//   we check for politeness also.
//
// Input:
//   waitq - the structure
//   now - the present timestamp
//
// Return:
//   1 if there are servers immediatly available
//   0 if the queue is empty or no servers are available
//

bool waitq_available( waitq_t *waitq, time_t now ) {
	assert( waitq != NULL );
	assert( now > 0 );

	if( waitq_empty( waitq ) ) {
		return 0;
	} else {
		// Check if the topmost element is ready
		if( waitq->queue[0].nextvisit <= now ) {
			return 1;
		} else {
			return 0;
		}
	}
}

//
// Name: waitq_destroy
//
// Description:
//   Destroys a waiting queue
//
// Input:
//   waitq - the structure
//

void waitq_destroy( waitq_t *waitq ) {
	assert( waitq != NULL );
	assert( waitq->nservers == 0 );
	assert( waitq->queue != NULL );

	// Free
	free( waitq->queue );
	free( waitq );
}

//
// Name: waitq_dump
//
// Description:
//   Shows the waitq
//
// Input:
//   waitq - the structure
//


void waitq_dump( waitq_t *waitq ) {
	assert( waitq != NULL );

	cerr << "WAITQ> ";
	for( uint i=0; i<waitq->nservers; i++ ) {
		cerr << "(" << waitq->queue[i].serverid << ":"
			 << waitq->queue[i].nextvisit << ")";
	}
	cerr << endl;
}
