/************************************************************************************
 *
 ************************************************************************************
 */
#ifndef	opcode_h_
#define	opcode_h_

#include "hash.h"

typedef	unsigned short WORD;
typedef	unsigned char  BYTE;

struct _OPCODE ;		// ちょこっと宣言.

#ifndef Extern
#define Extern extern
#endif

//	命令実行関数のプロトタイプ型.
typedef int (*EMUFUNC) (int code,struct _OPCODE *tab);
//	命令逆アセンブル関数のプロトタイプ型.
//typedef int (*DISFUNC) (int code,struct _OPCODE *tab);


typedef	struct _OPCODE {
	char *mnemonic;			// ニモ
	char *comment;			// 意味
	int   pattern;			// 機械語
	EMUFUNC emufunc;		// 命令実行関数
	int	  data1;				// 
} OPCODE;


#define	MEMSIZE		0x10000			// 実装メモリーサイズ (WORD)

/** *********************************************************************************
 *	メモリーコンテキスト
 ************************************************************************************
 */
Extern BYTE  memory[MEMSIZE];

/** *********************************************************************************
 *	レジスタコンテキスト
 ************************************************************************************
 */
typedef	struct _CONTEXT {
	WORD	pc;
	WORD	acc;
	char	z;	// zero
	char	c;	// carry
	char	m;	// minus
//
	WORD	pc_bak;	// jumpする前のPC.	(逆アセンブル時に必要)
} CONTEXT;

Extern CONTEXT	reg;


#define	EMUFUNC_(x_)	int x_(int code,struct _OPCODE *tab)
#define	DISFUNC_(x_)	int x_(int code,struct _OPCODE *tab)

//	命令実行関数のプロトタイプ宣言.
EMUFUNC_( f_JMP );
EMUFUNC_( f_LD  );
EMUFUNC_( f_ADD );
EMUFUNC_( f_ADI );
EMUFUNC_( f_ILD );
EMUFUNC_( f_ST  );
EMUFUNC_( f_DLY );
EMUFUNC_( f_NOP );
EMUFUNC_( f_HLT );
EMUFUNC_( f_XPAL );

EMUFUNC_( f_ORG );
EMUFUNC_( f_EQU );
EMUFUNC_( f_DW  );
EMUFUNC_( f_DOT );
EMUFUNC_( f_END );

EMUFUNC_( f_MOV );
EMUFUNC_( f_INC );
EMUFUNC_( f_DEC );
EMUFUNC_( f_CP );
EMUFUNC_( f_CALL);
EMUFUNC_( f_ENT );
EMUFUNC_( f_RET );
EMUFUNC_( f_LDH );
EMUFUNC_( f_LDL );
EMUFUNC_( f_LEA );

EMUFUNC_( f_JS );
EMUFUNC_( f_MACRO);
EMUFUNC_( f_ENDM );
EMUFUNC_( f_FUNCTION);
EMUFUNC_( f_MESSAGE);

// アドレスモード.
enum {
	d_PC,
	d_P1,
	d_P2,
	d_P3,
	d_IMM,
	d_P1A,
	d_P2A,
	d_P3A,
};

/*
enum {
	XX_IMM8 =0x0000,
	XX_REG8 =0x1000,
	XX_IND8 =0x2000,
	XX_IMM16=0x3000,	//後続する IMM16 即値を持つ.
	XX_MASK =0x3000,

	IS_REG	=0x10000,
};
//	dd	ディスプレイスメント
//	00	Abs
//	01	fwd
//	10	back
//	11	fwd+carry
enum {
	DD_ABS  =0x0000,
	DD_FWD  =0x0400,
	DD_BACK =0x0800,
	DD_FWDC =0x0C00,
	DD_MASK =0x0C00,
};
//
//	演算命令で、結果をAccに戻すかどうか.
//
enum {
	OP_TEST	    =0x0100,	//テストのみでAccを更新しない.
	OP_TEST_MASK=0x0100,
};
*/

Extern char symbuf[128];
Extern char label[128];
Extern char mnemonic[128];
Extern char operandbuf[128];
Extern char comment[512];

Extern char *operand;

Extern int	save_ptr;	//memory[]の有効サイズ.
Extern HASH *sym;		//シンボル表.

#define	ZZ	printf("%s:%d: ZZ\n",__FILE__,__LINE__);

#endif	//opcode_h_
