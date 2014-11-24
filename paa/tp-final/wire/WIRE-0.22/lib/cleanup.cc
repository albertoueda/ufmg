
#include "cleanup.h"

//
// Name: cleanup_enable
//
// Description:
//   Enables all signal handlers to routine die()
//

void cleanup_enable() {
	enable_signal_handler( signal_handler );
}

//
// Name: cleanup_disable
//
// Description:
//   Disables signal handlers
//

void cleanup_disable() {
	disable_signal_handler();
}

//
// Name: die
//
// Description:
//   Prints a message, calls cleanup() and exit()
//   Writes to syslog
//
// Input:
//   reason - String to print to screen
//

void die( const char *reason ) {
	cerr << endl;

	// Write to syslog
	syslog( LOG_ERR, "die: %s", reason );

	// Report
	fprintf( stderr, "\n*** Error: %s ***\n", reason );
	fprintf( stderr, "*** Emergency cleanup ... " );

	// Call the cleanup routine
	cleanup();
	fprintf( stderr, " done, exit(1)\n" );

	// Exit
	exit(1);
}

//
// Name: signal_handler
//
// Description:
//   Generic signal handler, calls die("Signal")
//

void signal_handler( int signum ) {
	// (Try to) avoid repeated signals (typical on multi-thread programs)
	disable_signal_handler();

	// Report the signal number
	// strsignal() seems to lock under some circumstances, so we
	// don't use it here, and we don't use streams either
	fprintf( stderr, "** Got signal %d **\n", signum );

	// Die
	die( "Signal" );
}


//
// Name: enable_signal_handler
//
// Description:
//   Puts a function as a handler for all trapable signals
//
// Input:
//   sighandler - Function to call in response to signals
//

void enable_signal_handler( void (*sighandler)(int) ) {
	struct sigaction new_action;
	int rc;

	// Set parameters for sigaction
	new_action.sa_handler = sighandler;
	sigemptyset( &new_action.sa_mask );
	new_action.sa_flags = 0;

	// Enables the handlers
	rc = sigaction( SIGINT, &new_action, NULL );
	assert( rc == 0 );
	rc = sigaction( SIGSEGV, &new_action, NULL );
	assert( rc == 0 );
	rc = sigaction( SIGABRT, &new_action, NULL );
	assert( rc == 0 );

	// Ignore SIGHUP, because it's usual
	// to run as a background process in a terminal that
	// can be disconnected.
	new_action.sa_handler = SIG_IGN;
	rc = sigaction( SIGHUP, &new_action, NULL );
	assert( rc == 0 );

	// Ignore SIGPIPE, because write() to network under older glibcs can
	// produce that signal
	new_action.sa_handler = SIG_IGN;
	rc = sigaction( SIGPIPE, &new_action, NULL );
	assert( rc == 0 );
}

//
// Name: disable_signal_handler
//
// Description:
//   Uninstall the handlers of enable_signal_handler
//

void disable_signal_handler() {
	struct sigaction new_action;

	// Set parameters for sigaction
	new_action.sa_handler = SIG_DFL;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

	// Enables the handlers
	sigaction( SIGINT, &new_action, NULL );
	sigaction( SIGSEGV, &new_action, NULL );
	sigaction( SIGABRT, &new_action, NULL );
	sigaction( SIGHUP, &new_action, NULL );
}
