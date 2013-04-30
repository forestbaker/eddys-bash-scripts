#include <stdio.h>
#include <stdlib.h>
//#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFSIZE 1024
//#define UC unsigned char

void usage(int);
char* file;
char* basename;
int isend(char);
int is_upper(int);
int to_lover(int);
int to_upper(int);

int lc=0;
int cntr=0;

int main(int argc, char* argv[])
{
    int fd, nr, i, kb=0;
    unsigned char buf[BUFFSIZE];
    basename = argv[0];
    if (argc < 2) usage(0);
    file = argv[1];
    if (argc > 2) usage(2);
    if ((fd = open(argv[1],O_RDWR)) < 0)
	usage(1);
        nr = read(fd, buf , BUFFSIZE);
    while(nr > 0){
	for (i=0; i<nr; i++){
	    if (buf[i] == 10 || buf[i] == 13) cntr++;
	    if(is_upper(buf[i])){
		if(lc)
		    buf[i] = to_lower(buf[i]);
		else
		    lc=1;}
	    else{
		if(lc==0 || cntr>1)
		    buf[i] = to_upper(buf[i]);
		if(isend(buf[i]))
		    lc=0;}}
	lseek(fd, -nr, SEEK_CUR);
	write(fd,buf,nr);
	printf("Processed %d kBytes\r", ++kb);
        nr = read(fd, buf , BUFFSIZE);}
    printf("\n%s %s done !!!     \n", argv[0], argv[1]);
    return 0;	
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

int isend(char ch){
    int ret=0;
    if(ch=='.' || ch=='?' || ch=='!' || ch=='\\' || ch=='{' || ch=='}')
	ret = 1;
    return ret;
}

int is_upper(int ltr){
    if (isupper(ltr))
	return 1;
    if (ltr > 223 || ltr == 179)
	return 1;
    else
	return 0;    
}

int to_lower(int ch){
    int ret;
    if (isupper(ch))
	ret = tolower(ch);
    else if (ch > 223)
	ret = ch - 32;
    else if (ch == 179)
	ret = 163;
    else ret = ch;
    return ret;
}

int to_upper(int ch){
    int ret=ch;
    if (cntr > 1) cntr = 0;
    lc = (ch == 32 || ch == 10 || ch == 13)?0:1;
    if (islower(ch))
	ret = toupper(ch);
    else if (ch > 191 && ch < 224)
	ret = ch + 32;
    else if (ch == 163)
	ret = 179;
    else if (!is_upper(ch)) 
	lc = 0;
    return ret;
}
