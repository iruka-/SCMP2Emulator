/** *********************************************************************************
 *	R16 アセンブラ.
 ************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcode.h"
#include "hash.h"

void usage()
{
	printf(
	"Usage:\n"
	" $ asmp2 infile.asm\n"
	);
	exit(1);
}

int	scmp2_asm(char *infile,char *outfile,char *listfile,int lopt);

char  infile[256];
char  outfile[256];
char  listfile[256];
/*
 *	オプション文字列チェック
 */
char  *opt[128];	/* オプション文字が指定されていたらその文字に
			   続く文字列を格納、指定なければNULL	*/

#define Getopt(argc,argv)  \
 {int i;for(i=0;i<128;i++) opt[i]=NULL; \
   while( ( argc>1 )&&( *argv[1]=='-') ) \
    { opt[ argv[1][1] & 0x7f ] = &argv[1][2] ; argc--;argv++; } \
 }

#define IsOpt(c) ((opt[ c & 0x7f ])!=NULL)
#define   Opt(c)   opt[ c & 0x7f ]

void addext(char *name,char *ext)
{
	char *p,*q;
	p=name;q=NULL;
	while( *p ) {
		if ( *p == '.' ) q=p;
		p++;
	}
	if(q==NULL) q=p;
	strcpy(q,".");
	strcpy(q+1,ext);
}

void memdump(int adr,int len);

/** *********************************************************************************
 *	メインルーチン.
 ************************************************************************************
 */
int main(int argc,char **argv)
{
	Getopt(argc,argv);
	if(argc<2) usage();

	strcpy(infile,argv[1]);
	if(argc<3) {
		strcpy(outfile,infile);
		addext(outfile,"bin");
	}else{
		strcpy(outfile,argv[2]);
	}
		strcpy(listfile,infile);
		addext(listfile,"lst");
	
	scmp2_asm(infile,outfile,listfile,IsOpt('l'));
	
	return 0;
}


/** *********************************************************************************
 *
 ************************************************************************************
 */
