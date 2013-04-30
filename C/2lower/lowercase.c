#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define BUFFSIZE 1024
#define UC unsigned char

void usage(int);
UC* file;
UC file1[BUFFSIZE];
UC* basename;
UC* newfile;

int to_lover(int);

int main(int argc, UC* argv[])
{
    int fd, nr, i, kb=0;
    basename = argv[0];
    if (argc < 2) usage(0);
    file = argv[1];
    newfile = file;
    strcpy((char*)file1, (char*)file);
    if (argc > 2) usage(2);
//    if ((fd = open(argv[1],O_RDWR)) < 0)
//	usage(1);
    while (*file != 0) *file = to_lower(*(file++));
    link((char*)file1, (char*)newfile);
    unlink((char*)file1);
}

void usage(int err)
{
    switch (err){
    case 0: 	printf("\tYou have missed filename\n");
		break;
    case 1: 	printf("\tFile %s dosen't exists or permissions denied\n", file);
		break;
    default:	printf("\tToo many parameters\n");
		break;
    }
    printf("\t\tUsage:\n\t%s <filename>\n", basename);
    exit (1);
}

int to_lower(int ch){
    int ret = ch;
    if (isupper(ch))
	ret = tolower(ch);
    if (ch > 223)
	ret = ch - 32;
    if (ch == 179)
	ret = 163;
    return ret;
}

