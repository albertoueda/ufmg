

#include <config.h>

#include "const.h"
#include "utils.h"
#include "xmlconf.h"
#include "xmlconf-main.h"

#define MAX_STR_LEN 1024

int main(int argc, char *argv[])
{
	regex_t preg;
	char *regex, *match;

	/* get input from user */
	if (argc != 3) {
		printf("Usage: %s <pattern> <string to match>\n", argv[0]);
		return 1;
	}
	regex = argv[1];
	match = argv[2];

	/* compile the regex */
	tokenizeToRegex( regex, &preg );

	/* match the regex */
	if (regexec(&preg, match, 0, NULL, 0) == 0)
		printf("match !\n");
	else
		printf("no match !\n");

	/* clean up */
	regfree(&preg);

	return 0;
}

void cleanup() {

}
