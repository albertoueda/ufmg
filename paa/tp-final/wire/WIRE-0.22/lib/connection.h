
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define MAX_QUERY 200
#define MAX_STRING 10000

void SendString(char *str, int sock);
void ReciveString(char *str, int sock);

void SendInt(int *i, int sock);
void ReciveInt(int *i, int sock);

void SendULong(ulong *ul, int sock);
void ReciveULong(ulong *ul, int sock);

