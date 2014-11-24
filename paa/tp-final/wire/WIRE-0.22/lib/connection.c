#include "connection.h"


void ReciveString(char *str, int sock){
	int n;
	int size;

	memset(str, 0, MAX_QUERY);

	n = read(sock, &size, sizeof(int));
	if(n<0)
		perror("ERROR reading the size");

	n = read(sock, str, size);
	if(n<0)
		perror("ERROR reading a string");

}

void SendString(char *str, int sock){
	int n;
	int size = strlen(str);
	n = write(sock, (void*)&size, sizeof(int));
	if(n<0)
		perror("ERROR sending the size");
	n = write(sock, str, size);
	if(n<0)
		perror("ERROR sending a string");
}

void SendInt(int *i, int sock){
	int n;
	n = write(sock, (void*)i, sizeof(int));
	if(n<0)
		perror("ERROR sending an int");
}

void ReciveInt(int *i, int sock){
	int n;
	n = read(sock, i, sizeof(int));
	if(n<0)
		perror("ERROR reading an int");
}

void SendULong(ulong *ul, int sock){
	int n;
	n = write(sock, (void*)ul, sizeof(ulong));
	if(n<0)
		perror("ERROR sending an ulong");
}

void ReciveULong(ulong *ul, int sock){
	int n;
	n = read(sock, ul, sizeof(ulong));
	if(n<0)
		perror("ERROR reading an ulong");
}

