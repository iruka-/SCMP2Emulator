/**********************************************************************
 *  ハッシュの管理
 **********************************************************************
 */
#ifndef	_hash__h_
#define	_hash__h_

// ===================================================
//  ハッシュ表のエントリーアドレスが指すデータの構造.
//	（単方向リンクリストです.）
// ===================================================

typedef struct _CELL {
	struct _CELL *next;		//次のエントリ.
	int   val;				//登録した整数値.
	char  str[1];			//可変長の文字列.
} CELL;


//	ルート管理表.
typedef	struct {
	int	  entry_size;
	int	  entry_mask;
	struct _CELL **entry_table;
} HASH;

//	ハッシュ初期化、登録、検索 の各関数.
HASH *hash_create(int entsize);
int   hash_insert(HASH *h,char *str,int val,int dupok);
int	  hash_search(HASH *h,char *str,int *val);
int	  hash_iterate(HASH *h , int (*func)(char *p,int v));

//	単方向リストの作成.
CELL *make_cell(char *str,int val,CELL *next);

#endif	//_hash__h_

/**********************************************************************
 *  
 **********************************************************************
 */
