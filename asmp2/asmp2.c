/** *********************************************************************************
 *	R16 アセンブラ.
 ************************************************************************************
 */

/*
ＴｏＤｏ

[O]	-	行分解
	-	ラベル
	-	ニモニック
	-	オペランド
	-	リスト出力
	-	バイナリ出力

注意:
	２パスアセンブラなので、
	-	１パス目はアドレス確定のみ行なう.
	-	２パス目で完全なバイナリーを確定する.
	
	-	つまり、１パス目と２パス目で、命令を落とす処理が変わってはならない
		（アドレスがずれてはならない）
	-	特に、１パス目は未定義ラベル等のためにアドレス不確定となるけれども
		２パス目と同一の振る舞いをしなければならない（生成コードが違っても良いがアドレスがずれるのはだめ）

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "opcode.h"
#include "hash.h"

OPCODE opcode_init_tab[]={
 //ニモ,意味					,機械語,f_emu,d_dis --------------------
 {"LD" ,"Load"                  ,0xc0  ,f_ADD,0},//
 {"ST" ,"Store"                 ,0xc8  ,f_ADD,0},//
 {"AND","AND"                   ,0xd0  ,f_ADD,0},//
 {"OR" ,"OR"                    ,0xd8  ,f_ADD,0},//
 {"XOR","eXcusive OR"           ,0xe0  ,f_ADD,0},//
 {"DAD","ADD Decimal"           ,0xe8  ,f_ADD,0},//
 {"ADD","ADD"                   ,0xf0  ,f_ADD,0},//
 {"CAD","Compliment ADD"        ,0xf8  ,f_ADD,0},//
 {"NOP","NOP"                   ,0x08  ,f_NOP,0},//-,-,-,
// {"HALT","HALT"                 ,0x00  ,f_HLT,0},//-,-,-,停止

 {"LDI","Load Imm"              ,0xc0  ,f_ADI,0},//
 {"ANI","AND Imm"               ,0xd0  ,f_ADI,0},//
 {"ORI","OR Imm"                ,0xd8  ,f_ADI,0},//
 {"XRI","eXcusive OR Imm"       ,0xe0  ,f_ADI,0},//
 {"DAI","ADD Decimal Imm"       ,0xe8  ,f_ADI,0},//
 {"ADI","ADD Imm"               ,0xf0  ,f_ADI,0},//
 {"CAI","Compliment ADD Imm"    ,0xf8  ,f_ADI,0},//

 {"JMP","JUMP"            ,0x90  ,f_JMP,0},//
 {"JP","JUMP if plus"     ,0x94  ,f_JMP,0},//
 {"JZ","JUMP if zero"     ,0x98  ,f_JMP,0},//
 {"JNZ","JUMP if not zero",0x9c  ,f_JMP,0},//
	
 {"ILD","",   0xA8 ,f_ILD,0},    //22 2      AC, EA <- (EA) + 1
 {"DLD","",   0xB8 ,f_ILD,0},    //22 2      AC, EA <- (EA) - 1

 {"LDE","",   0x40 ,f_NOP,0},    // 6 1      AC <- (E)
 {"XAE","",   0x01 ,f_NOP,0},    // 7 1      (AC) <-> (E)
 {"ANE","",   0x50 ,f_NOP,0},    // 6 1      AC <- (AC) & (E)
 {"ORE","",   0x58 ,f_NOP,0},    // 6 1      AC <- (AC) | (E)
 {"XRE","",   0x60 ,f_NOP,0},    // 6 1      AC <- (AC) ^ (E)
 {"DAE","",   0x68 ,f_NOP,0},    //11 1  *   AC <- (AC) + (E) + (CY/L) {BCD format}
 {"ADE","",   0x70 ,f_NOP,0},    // 7 1  **  AC <- (AC) + (E) + (CY/L)
 {"CAE","",   0x78 ,f_NOP,0},    // 8 1  **  AC <- (AC) + !(E) + (CY/L)
 {"XPAL","",  0x30 ,f_XPAL,0},    // 8 1      (AC) <-> (PL)
 {"XPAH","",  0x34 ,f_XPAL,0},    // 8 1      (AC) <-> (PH)
 {"XPPC","",  0x3C ,f_XPAL,0},    // 7 1      (PC) <-> (PTR)
//    LDI    L(SUBR1)
 {"SIO","",   0x19 ,f_NOP,0},    // 5 1      Serial Input/Output
 {"SR" ,"",   0x1C ,f_NOP,0},    // 5 1      Shift Right
 {"SRL","",   0x1D ,f_NOP,0},    // 5 1      Shift Right with Link
 {"RR" ,"",   0x1E ,f_NOP,0},    // 5 1      Rotate Right
 {"RRL","",   0x1F ,f_NOP,0},    // 5 1  *   Rotate Right with Link
 {"HALT","",  0x00 ,f_NOP,0},    // 8 1      Output Halt pulse
 {"CCL","",   0x02 ,f_NOP,0},    // 5 1      CY/L <- 0
 {"SCL","",   0x03 ,f_NOP,0},    // 5 1      CY/L <- 1
 {"IEN","",   0x05 ,f_NOP,0},    // 6 1      IE <- 1
 {"DINT","",  0x04 ,f_NOP,0},    // 6 1      IE <- 0
 {"CSA","",   0x06 ,f_NOP,0},    // 5 1      AC <- (SR)
 {"CAS","",   0x07 ,f_NOP,0},    // 6 1      SR <- (AC)
 {"NOP","",   0x08 ,f_NOP,0},    // 5 1      No operation
 {"DLY","",   0x8F ,f_DLY,0},    //?? 2      Delay
 //擬似命令
 {"ORG",".Set Origin"   ,0x100     ,f_ORG,0},
 {"EQU",".EQU" 		    ,0x100     ,f_EQU,0},
 {"=",".EQU" 		    ,0x100     ,f_EQU,0},
 {"DW" ,".DW" 		    ,0x100     ,f_DW ,0},
 {"DOT" ,".DOT" 	    ,0x100     ,f_DOT,0},
 {"END",".END" 		    ,0x100     ,f_END,0},
 {"MOV",".MOV"          ,0x100     ,f_MOV,0},
 {"CALL",".CALL"        ,0x100     ,f_CALL,0},
 {"RET",".RET"          ,0x100     ,f_RET,0},
 {"CP", ".CP"           ,0x100     ,f_CP,0},
 {"LDH","LD H(n)"       ,0xc4      ,f_LDH,0},
 {"LDL","LD L(n)"       ,0xc4      ,f_LDL,0},
 {"LEA","Pn,ea"         ,0x100     ,f_LEA,0},
 {"LDPI","Pn,ea"        ,0x100     ,f_LEA,0},
 {"JS" ,"Jump Subr"     ,0x100     ,f_JS ,0},
 {"MACRO" ,"START macro",0x200     ,f_MACRO,0},
 {"ENDM" ,"END macro"   ,0x201     ,f_ENDM ,0},
 {"FUNCTION" ,"def fun" ,0x100     ,f_FUNCTION,0},
 {"MESSAGE" ,"text"     ,0x100     ,f_MESSAGE,0},
//ニモ,意味				,機械語	   ,f_emu,d_dis  //z,c,m,動作 --------------------
 { NULL,NULL,0}
};

FILE *ifp;
FILE *ofp;
FILE *lfp;

char  *s_infile;	//	ソースファイル名.
int	   lnum;		//	行番号.
int	   macrof;		//	マクロ定義中.
char   line[1024];
char  *lptr;
int		s_pass;
int		s_lopt;		// '-l' ?

#define Ropen(name) {ifp=fopen(name,"rb");if(ifp==NULL) \
{ printf("Fatal: can't open file:%s\n",name);exit(1);}  \
}
#define Wopen(name) {ofp=fopen(name,"wb");if(ofp==NULL) \
{ printf("Fatal: can't create file:%s\n",name);exit(1);}  \
}
#define Lopen(name) {lfp=fopen(name,"wb");if(lfp==NULL) \
{ printf("Fatal: can't create file:%s\n",name);exit(1);}  \
}

#define Write(buf,siz)  fwrite(buf,1,siz,ofp)

#define Wclose()  fclose(ofp)
#define Lclose()  fclose(lfp)
#define Rclose()  fclose(ifp)


int	str_index(char *s,int a);

void db(int data)
{
	memory[reg.pc++]=data;
	
	if( save_ptr < reg.pc ) {
		save_ptr = reg.pc;
	}
}

void dw(int data)
{
	db(data);
	db(data>>8);
}

void Wsave(void)
{
	Write(memory,save_ptr);
}

/** *********************************************************************************
 *
 ************************************************************************************
 */
int	str_cmpi(char *t,char *s)
{
	while(*t) {
		if(tolower(*t)!=tolower(*s)) return 1;
		t++;
		s++;
	}
	if(*s) return 1;
	return 0;
}

/** *********************************************************************************
 *	
 ************************************************************************************
 */
void Error(char *format, ...)
{
	va_list argptr;
	char buffer[256];

	if(s_pass == 0) return;
	
	va_start(argptr, format);
	vsnprintf(buffer, sizeof(buffer), format, argptr);

	fprintf(stderr,"FATAL:%s\n",buffer);
	fprintf(stderr,"%s:%d:%s\n",s_infile,lnum,line);

	fprintf(lfp,"FATAL:%s\n",buffer);
	fprintf(lfp,"%s:%d:%s\n",s_infile,lnum,line);

	va_end(argptr);

//	exit(1);
}

/** *********************************************************************************
 *
 ************************************************************************************
 */
OPCODE *search_code(char *mne)
{
	OPCODE *t = opcode_init_tab;
	while(t->mnemonic) {
		if(str_cmpi(mne,t->mnemonic)==0) return t;
		t++;
	}
	return NULL;
}



/** *********************************************************************************
 *	ファイル(fp)から１行分の文字列を(buf)に読み込む.
 ************************************************************************************
 *	成功すると0
 *	文字数オーバー=1
 *	EOFに達するとEOFを返す.
 */
int getln(char *buf,FILE *fp)
{
	int c;
	int l;
	l=0;
	while(1) {
		c=getc(fp);
		if(c==EOF)  return(EOF);/* EOF */
		if(c==0x0d) continue;
		if(c==0x0a) {
			*buf = 0;	/* EOL */
			return(0);	/* OK  */
		}
		*buf++ = c;l++;
		if(l>=255) {
			*buf = 0;
			return(1);	/* Too long line */
		}
	}
}

/** *********************************************************************************
 *
 ************************************************************************************
 */
int	is_spc(int c)
{
	if(c == '\t') return 1;
	if(c == ' ') return 1;
	return 0;
}
/** *********************************************************************************
 *
 ************************************************************************************
 */
char *spskip(void)
{
	while(*lptr) {
		if(is_spc(*lptr)==0) break;
		lptr++;
	}
	return lptr;
}

int remove_tailsp(char *s)
{
	int l=strlen(s);
	int i;
	for(i=(l-1);i>=0;i--) {
		if( is_spc(s[i])==0 ) break;
		s[i]=0;
	}
	return 0;
}


/** *********************************************************************************
 *	行バッファから、単語を１つ取得する.
 ************************************************************************************
 */
char *getsym(char *buf)
{
	char *t = buf;
	*t=0;
	spskip();
	while(*lptr) {
		if(is_spc(*lptr)) break;
		*t++ = *lptr++;
	}
	*t=0;
	return buf;
}
int is_comment(char *symbuf)
{
	int c = *symbuf;
	if((c==0)||(c==';')) return 1;	//コメント.
	if((c=='/')&&(symbuf[1]=='/')) return 1;	//コメント.
	return 0;
}
/** *********************************************************************************
 *	アセンブラ：１行の文字列から、ラベル: ニモニック オペランドに分解する.
 ************************************************************************************
 */
int	asm_pre(char *line,int pass)
{
	int c,l;
//	printf("%d:%s\n",lnum,line);
	getsym(symbuf);
	c = *symbuf;
	if(is_comment(symbuf)) {return 0;}
	(void)c;

	l=strlen(symbuf);
	if((symbuf[l-1] == ':')||(is_spc(*line)==0)) {
		strcpy(label,symbuf);
		if(symbuf[l-1] == ':') {
			label[l-1]=0;			// ':'  を取る.
		}
		getsym(symbuf);
	}
	
	if(is_comment(symbuf)) {return 1;}
	strcpy(mnemonic,symbuf);	//ニモニック取得.
	
#if 0
	getsym(symbuf);
	if(is_comment(symbuf)) {return 2;}
	strcpy(operandbuf,symbuf);		//オペランド取得.
#else
	char *t = spskip();
	strcpy(operandbuf,t);		    //オペランド取得.
	if(is_comment(operandbuf)) {
		operandbuf[0]=0;
		return 2;
	}
	// ';'を取る.
	int c1 = str_index(operandbuf,';');
	if(c1>=0) {
		operandbuf[c1]=0;
	}
	remove_tailsp(operandbuf);
#endif
	
	return 3;
}

/** *********************************************************************************
 *	
 ************************************************************************************
 */
int	set_label(char *label,int val)
{
	hash_insert(sym,label,reg.pc,1);
	return 0;
}
void add_spc(char *buf,int spc)
{
	int len = strlen(buf);
	int i = spc - len;
	if( i <= 0) return;

	buf += len;
	while(i>=0) {
		*buf++ = ' ';
		i--;
	}
	*buf = 0;
}
/** *********************************************************************************
 *	
 ************************************************************************************
 */
int	asm_list(int pass)
{
	char hexbuf[256]="";
	char label1[256];
	char *t = hexbuf;
	BYTE *d = &memory[reg.pc_bak];
	int  m = reg.pc - reg.pc_bak;
	int	 i;
	int	 n;

	if(pass == 0) return 0;		//１パスと２パスでアドレスがずれるときは、この行を
								//コメントアウトの状態で２個のリストを出してみる.
	if(m>=12) m=12;
	for(i=0;i<m;i++) {
		n = sprintf(t,"%02x ",*d++);
		t+=n;
	}
	if(s_lopt) { add_spc(hexbuf,32);}

	strcpy(label1,label);
	if(label1[0]) {
		strcat(label1,":");
	}
	if((label1[0]==0)&&(mnemonic[0]==0)) {
		fprintf(lfp,"%5s %-10s%s\n","",hexbuf,line);
	}else{
		fprintf(lfp,"%04x:%-10s%-10s %-4s %s %s\n"
			,reg.pc_bak,hexbuf,label1,mnemonic,operandbuf,comment);
	}
	return 0;
}

int	op_exec(char *inst,char *oper)
{
	OPCODE *p = search_code(inst);
	if(p==NULL) {
		Error("Mnemonic error(%s)",inst);
	}else{
		operand = oper;
		if((macrof==0)||(p->pattern >=0x200)) {
			p->emufunc(p->pattern,p);	//ニモニックの各処理.
		}
	}
	return 0;
}
/** *********************************************************************************
 *	アセンブラ：１行の処理.
 ************************************************************************************
 */
int	asm_line(char *line,int pass)
{
	int pre = asm_pre(line,pass);

	reg.pc_bak = reg.pc;

	strcpy(comment,lptr);
	if(pre<1) {
		return pre;				//コメントだけの行.
	}
	
	set_label(label,reg.pc);	//ラベル:
	if(mnemonic[0]==0) {
		return 0;				//ラベル:  だけの行.
	}

	op_exec(mnemonic,operandbuf);

	return 0;
}
/** *********************************************************************************
 *	
 ************************************************************************************
 */
void init_line(void)
{
	    lptr = line;
		symbuf[0]  =0;
		label[0]   =0;
		mnemonic[0]=0;
		operandbuf[0] =0;
}
/** *********************************************************************************
 *	アセンブラ：１パスの処理.
 ************************************************************************************
 */
int	asm_pass(char *infile,int pass)
{
	int rc;
	lnum = 1;
	macrof = 0;
	
	s_pass = pass;

	reg.pc=0;

	Ropen(infile);
	while(1) {
		rc = getln(line,ifp);
		if(rc == EOF) break;
		init_line();
		asm_line(line,pass);
		asm_list(pass);

		lnum++;
	}
	Rclose();
	return 0;
}

/** *********************************************************************************
 *	アセンブラ：シンボル表示.
 ************************************************************************************
 */
int printsym_func(char *p,int v)
{
	char *is_reg="";
	fprintf(lfp,"%-16s = %s%d\t(0x%x)\n",p,is_reg,v,v);
	return 0;
}
void asm_printsym(void)
{
	fprintf(lfp,"\n\nSYMBOL LIST:\n");
	hash_iterate(sym,printsym_func);
}

/** *********************************************************************************
 *	アセンブラ：メインルーチン
 ************************************************************************************
 */
int	scmp2_asm(char *infile,char *outfile,char *listfile,int lopt)
{
	int i;
	sym = hash_create(0x1000);
	s_infile=infile;
	s_lopt = lopt;
	save_ptr=0;

	Lopen(listfile);
	Wopen(outfile);
	for(i=0;i<3;i++) {		//今のところ適当.
		asm_pass(infile,0);
	}
	asm_pass(infile,1);
	Wsave();
	Wclose();

	asm_printsym();

	Lclose();
	return 0;
}
/** *********************************************************************************
 *
 ************************************************************************************
 */
