/** *********************************************************************************
 *	オペコードの解釈実行.
 ************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define Extern /* */

#include "opcode.h"
#include "hash.h"

/** *********************************************************************************
 *
 ************************************************************************************
 */
int	 str_cmpi(char *t,char *s);
int	 set_label(char *label,int val);
void dw(int);
void db(int);
void Error(char *format, ...);
int	 op_exec(char *inst,char *operand);
extern int	 s_pass;
extern int   macrof;
char *op1,*op2;
char opbuf[256];

#define	SPLIT_MAX 256
char *split_result[SPLIT_MAX];
char  split_string[1024];

enum {
	m_HI=1,
	m_LO=2,
};

//
//	文字列(s)に文字(a)が含まれていれば、その位置を文字数(>=0)で返す.
//	含まれていなければ -1 を返す.
int	str_index(char *s,int a)
{
	int c;
	int idx=0;
	while(1) {
		c = *s;
		if(c==0) return -1;
		if(c==a) return idx;
		s++;idx++;
	}
}
//
//	文字列(s)をカンマ(,)で複数に分解する.
//	-----------------------------------------------
//	分解された個数(最大256個) を戻り値にする.
//		分解後の文字列バッファは split_result[]
int	 split_str(char *s)
{
	strcpy(split_string,s);
	char *p=split_string;
	int cnt=0;
	int c;

	split_result[cnt++]=p;	//最初の位置.
	while(1) {
		c = *p;
		if(c==0) break;
		if(c==',') {
			*p=0;
			split_result[cnt++]=(p+1);	// ','の次の位置.
		}
		p++;
	}
	return cnt;
}

//
//	文字列(s)をカンマ(,)で２つまでに分解する.
//	-----------------------------------------------
//	分解された個数(最大2個) を戻り値にする.
//	分解された文字列は op1とop2になる.
int	 split_str2(char *s)
{
	char *p=opbuf;
	int c;
	if(str_index(s,',')<0) return 1;

	strcpy(opbuf,s);
	op1=opbuf;
	while(1) {
		c = *p;
		if(c==0) break;
		if(c==',') {
			*p=0;op2=&p[1];return 2;
		}
		p++;
	}
	op2=p;
	return 1;
}

//
//	asm命令(op) を cnt個生成する.
//
void op_execN(char *op,char *cnt)
{
	int n=1;
	int i;
	int rc = sscanf(cnt,"%d",&n);
	if(rc==0) {
		Error("OP cnt(%s)",cnt);
	}
	for(i=0;i<n;i++) {
		op_exec(op,"1");
	}
}
/** *********************************************************************************
 *	１文字(c)が１６進数なら、数値に変換する.
 ************************************************************************************
 *	エラーの場合は (-1)
 */
int	is_hex(int c)
{
	if((c>='0')&(c<='9')) return c-'0';
	if((c>='A')&(c<='F')) return c-'A'+10;
	if((c>='a')&(c<='f')) return c-'a'+10;
	return -1;
}
/** *********************************************************************************
 *	文字列(src)を数値、もしくは１６進数値に変換して(*val)に入れる.
 ************************************************************************************
 *	最後に'H'が付いているものだけ１６進数値に変換する.
 *
 *	成功すれば(1) 失敗すれば(0)を返す.
 */
int	is_numhex(char *src,int *val)
{
	int d=0,hex=0;
	int c,x,hexf=0;

	c = *src++;
	d = is_hex(c);hex=d;
	if(d>=10) hexf=1;

	if(d==(-1)) return 0;	// Error
	while(1) {
		c = *src++;
		if(c==0) {
			if(hexf==0) {
				*val = d;	// 10進.
				return 1;	// OK
			}
			return 0;		// Error
		}
		if((c=='h')||(c=='H')) {
			if(*src==0) {
				*val = hex;
				return 1;	// OK
			}
		}
		x = is_hex(c);
		if(x == (-1)) {
			return 0;		// Error
		}
		if(x>=10) hexf=1;
		d=d*10+x;
		hex=hex*16+x;
	}
}

int evalptr(char *p)
{
	if(str_cmpi(p,"pc")==0) return 0;
	if(str_cmpi(p,"p0")==0) return 0;
	if(str_cmpi(p,"p1")==0) return 1;
	if(str_cmpi(p,"p2")==0) return 2;
	if(str_cmpi(p,"p3")==0) return 3;
	return -1;
}
/** *********************************************************************************
 *	オペランドを解析して、実効アドレス(*val) を求める.
 ************************************************************************************
 *	modeはアドレスモードを入れる.
 *	成功すれば真(非ゼロ)を返す.
 */
int eval(char *src,int *val,int *mode)
{
	int rc=0;
	//   "オペランド(ポインタ)"  　・・・（srcbuf=オペランド ptrbuf=ポインタ名）
	char srcbuf[256];
	char ptrbuf[256];

	int c;
	int	len1;        // ')' の現われる場所.
	int	len2;        // '(' の現われる場所.
	int	minus1=1;    // '-' で始まる場合のみ (-1)が代入される.
	int autoindex=0; // '@' で始まる場合のみ (1) が代入される.
	int pointer=-1;  // (pN) が後置される場合(0,1,2,3)のいずれかの数値.(ポインタ番号)
	int hilow=0;
	
	strcpy(srcbuf,src);
	strcpy(ptrbuf,"" );
//	printf("eval(%s)\n",src);
	
	*mode = d_PC;

	//
	// 「Offset(Pointer)」 を解釈.
	//   例外的に、 H(Label) L(Label) を解釈.
	//
	len2=str_index(srcbuf,'(');
	//printf("\n;;;len2=%d (%s)\n",len2,srcbuf);
	if(len2 >= 0) {
		srcbuf[len2]=0;
		strcpy(ptrbuf,src+len2+1);
		len1=str_index(ptrbuf,')');
		if(len1<0) {
			Error("Error Operand Syntax. ' %s '",src);	// )がない.
		}else{
			ptrbuf[len1]=0;
			// H() かL()
			if(strcmp(srcbuf,"H")==0) {
				hilow=m_HI;
				*mode = d_IMM;
				strcpy(srcbuf,ptrbuf);strcpy(ptrbuf,"");
			}else
			if(strcmp(srcbuf,"L")==0) {
				hilow=m_LO;
				*mode = d_IMM;
				strcpy(srcbuf,ptrbuf);strcpy(ptrbuf,"");
			}else{
				// (pN)に限定.
				pointer = evalptr(ptrbuf);
				if(pointer<0) {
					Error("Error Operand need Pointer(pN). ' %s '",src);	// )がない.
				}
				*mode = d_PC + pointer;
			}
		}
	}

	//printf("eval(%s)<srcbuf\n",srcbuf);
	char *s = srcbuf;
	if(*s=='@') {
		if(pointer<0) {
			Error("Error Operand need Pointer(pN). ' %s '",src);	// )がない.
		}
		autoindex = 1;
		*mode = d_P1A + pointer;
		s++;
	}else
	if(*s=='#') {
		*mode = d_IMM;
		s++;
	}


	c = tolower(*s);
	// $ffff
	if(c=='$') {
		rc = sscanf(s+1,"%x",val);
		goto L1;
	}
	// 0xaaaa
	if((c=='0') && (s[1]=='x')) {
		rc = sscanf(s+2,"%x",val);
		goto L1;
	}
	
	if(*s=='-') {
		minus1=(-1);
		s++;
	}
	// 1234 || aaffh
	if(is_numhex(s,val)){
		*val = *val * minus1;
		rc=1;
		goto L1;
	}

	//printf("hash_search(%s)\n",s);
	//	label
	if( hash_search(sym,s,val) ) {
		//						そうでない場合は定数ラベル.
		goto L1;
	}else{
		if(s_pass) {
			Error("Symbol Not Found(%s)",s);	//未定義ラベル.
		}
		return 0;
	}
	
L1:
	if(hilow) {
		if(hilow==m_HI) {
			*val = (*val >>   8);
		}else{
			*val = (*val & 0xff);
		}
	}
	
	(void)autoindex;
	return rc;
}


/** *********************************************************************************
 *	オペランドを解析して、実効アドレス(*val) を求める.
 ************************************************************************************
 *	成功すれば真(非ゼロ)を返す.
 */
int evalnum(char *src,int *val)
{
	int rc=0;
	//   "オペランド(ポインタ)"  　・・・（srcbuf=オペランド ptrbuf=ポインタ名）
	char srcbuf[256];

	int c;
	int	minus1=1;    // '-' で始まる場合のみ (-1)が代入される.
	
	strcpy(srcbuf,src);

	char *s = srcbuf;
	c = tolower(*s);
	// $ffff
	if(c=='$') {
		rc = sscanf(s+1,"%x",val);
		return rc;
	}
	// 0xaaaa
	if((c=='0') && (s[1]=='x')) {
		rc = sscanf(s+2,"%x",val);
		return rc;
	}
	
	if(*s=='-') {
		minus1=(-1);
		s++;
	}
	// 1234 || aaffh
	if(is_numhex(s,val)){
		*val = *val * minus1;
		rc=1;
		return rc;
	}
	//	label
	if( hash_search(sym,s,val) ) {
		//						そうでない場合は定数ラベル.
//		printf("evalnum val=%x\n",*val);
		return rc;
	}else{
		if(s_pass) {
			Error("Symbol Not Found(%s)",s);	//未定義ラベル.
		}
		return 0;
	}
	
	return rc;
}
/** *********************************************************************************
 *	こっから下は、命令の実行を行なう.
 ************************************************************************************
 {"JMP","JMP always"  ,"00xxdd00"  ,f_JMP,d_JMP},//-,-,-,無条件に分岐する
 */
int f_JMP (int code,OPCODE *tab)
{
	int val=0,mode,rc,offset=0;
	rc = eval(operand,&val,&mode);
	//定数アドレスへの分岐.
	//常に相対分岐にする.
	offset = val - (reg.pc+2);
//	if(offset>=0) {
//		rel = DD_FWD;
//	}else{
//		rel = DD_BACK;
//		offset = -offset;
//	}

	db(code);
	db(offset);

	(void)rc;
	return 0;
}
int f_ADD (int code,OPCODE *tab)
{
	int val=0,mode,rc;
	rc = eval(operand,&val,&mode);
//	if(rc>0) 
	{
			code |= mode;
			db(code);
			db(val);
	}
	(void)rc;
	return 0;
}
int f_ADI (int code,OPCODE *tab)
{
	int val=0,mode,rc;
	rc = eval(operand,&val,&mode);
	if(mode == 0) {mode=4;}
	if(mode != 4) {
		Error("Operand must be immediate. ( %s )",operand);
	}
	//	if(rc>0) 
	{
			code |= mode;
			db(code);
			db(val);
	}
	(void)rc;
	return 0;
}
int f_ILD (int code,OPCODE *tab)
{
	int val=0,mode,rc;
	rc = eval(operand,&val,&mode);
//	if(rc>0) 
	{
			code |= mode;
			db(code);
			db(val);
	}
	(void)rc;
	return 0;
}
int f_XPAL  (int code,OPCODE *tab)
{
	int val=0,mode,rc=0;
	rc = eval(operand,&val,&mode);
	db(code|val);
	(void)rc;
	return 0;
}
int f_DLY  (int code,OPCODE *tab)
{
	int val=0,mode,rc=0;
	rc = eval(operand,&val,&mode);
	db(code);
	db(val);
	(void)rc;
	return 0;
}
int f_ST  (int code,OPCODE *tab)
{
	db(code);
	return 0;
}
int f_HLT (int code,OPCODE *tab)
{
	return f_ST(code,tab);
}
int f_NOP (int code,OPCODE *tab)
{
	return f_ST(code,tab);
}



//	擬似命令.
int f_ORG (int code,OPCODE *tab)
{
	int val=0,mode,rc;
	rc = eval(operand,&val,&mode);
	if(rc>0) {
		reg.pc = val;
		reg.pc_bak = val;
	}
	return 0;
}
int f_EQU (int code,OPCODE *tab)
{
	int val=0,mode,rc;
	rc = eval(operand,&val,&mode);
	if(rc>0) {
//		set_label(label,val);
		hash_insert(sym,label,val,1);
	}
	(void)rc;
	return 0;
}
int f_DW (int code,OPCODE *tab)
{
	int i,val,mode,rc;
	int cnt=split_str(operand);
	for(i=0;i<cnt;i++) {
		rc = eval(split_result[i],&val,&mode);
		if(rc>0) {
			dw(val);
		}else{
			Error("DW value(%s)",split_result[i]);
		}
	}
	return 0;
}
int f_DOT (int code,OPCODE *tab)
{
	int i,val,mode,rc,high=0;
	int cnt=split_str(operand);
	for(i=0;i<cnt;i++) {
		rc = eval(split_result[i],&val,&mode);
		if(rc>0) {
			if(i & 1) {
				dw(high | val);
			}else{
				high=(val<<4);
			}
		}else{
			Error("DW value(%s)",split_result[i]);
		}
	}
	if(cnt & 1) {
		dw(high | 0);
	}
	return 0;
}
int f_END (int code,OPCODE *tab)
{
	return 0;
}
int f_MOV (int code,OPCODE *tab)
{
	int n = split_str2(operand);
	if(n!=2) Error("Need 2 operand!");
	op_exec("ld",op2);		// src
	op_exec("st",op1);		// dst
	return 0;
}
int f_CP (int code,OPCODE *tab)
{
	int n = split_str2(operand);
	if(n!=2) Error("Need 2 operand!");
//	op2-op1にしているのは Borrowになっているから.
	op_exec("ld" ,op2);		// src
	op_exec("sub",op1);		// dst
#if	0	// Z判定出来ないので...
	op_exec("sub","1");		// dst
#endif
	return 0;
}

int f_LDH (int code,OPCODE *tab)
{
	int val=0,rc;
	rc = evalnum(operand,&val);
//printf("LDH val=%d %s\n",val,operand);
	db(code);
	db(val>>8);
	(void)rc;
	return 0;
}
int f_LDL (int code,OPCODE *tab)
{
	int val=0,rc;
	rc = evalnum(operand,&val);
//printf("LDL val=%d %s\n",val,operand);
	db(code);
	db(val);
	(void)rc;
	return 0;
}
//	LEA Pn,EA
int f_LEA (int code,OPCODE *tab)
{
	int n = split_str2(operand);
	if(n!=2) Error("Need 2 operand!");
	op_exec("ldh",op2);
	op_exec("xpah",op1);
	op_exec("ldl",op2);
	op_exec("xpal",op1);
	return 0;
}
//	関数呼び出し:
int f_CALL (int code,OPCODE *tab)
{
	char op3[256];
	strcpy(op3,operand);
	op_exec("ldh",operand);
	op_exec("xpah","3");
	op_exec("ldl",op3);
	op_exec("xpal","3");
	op_exec("xppc","3");
	return 0;
}

int f_JS (int code,OPCODE *tab)
{
	char op3[256];
	strcpy(op3,operand);
	op_exec("ldh",operand);
	op_exec("xpah","3");
	op_exec("ldl",op3);
	op_exec("xpal","3");
	op_exec("xppc","3");
	return 0;
}
int f_MACRO(int code,OPCODE *tab)
{
	macrof=1;
	return 0;
}
int f_ENDM (int code,OPCODE *tab)
{
	macrof=0;
	return 0;
}
int f_FUNCTION(int code,OPCODE *tab)
{
	return 0;
}
int f_MESSAGE(int code,OPCODE *tab)
{
	return 0;
}

//	関数の戻りで行なう処理:
int f_RET (int code,OPCODE *tab)
{
	op_exec("xppc","3");
	// pc = ra;
	return 0;
}
//	関数の先頭で行なう処理:
int f_ENT (int code,OPCODE *tab)
{
	return 0;
}

/** *********************************************************************************
 *
 ************************************************************************************
 */
