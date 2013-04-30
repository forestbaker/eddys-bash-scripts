#include <stdio.h>

int main(){
    FILE* f=fopen("KOI8-R","w");
    int i;
    for(i=128; i<256; i++){
	fputc(i,f);
	putchar(i);
    }
    fclose(f);
}
