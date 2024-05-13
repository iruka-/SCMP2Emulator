/**********************************************************************
 *  ハッシュの管理
 **********************************************************************
 *
 * ハッシュ表は２のべき乗個のエントリー表で構成されます。
 * ハッシュ値を示すセルの構造は、｛整数値.文字列｝のペアです。

 * 表の登録、検索は文字列キーで行い、結果は32bit整数値のみです。

 * ハッシュ表のエントリー数は２のべきであれば任意個数で生成されます。
 * 文字列のハッシュ値が衝突した場合は、セルの単方向リストを辿ります。
 * 表の削除は出来ません。



//	ハッシュ初期化、登録、検索 の各関数.

HASH *hash_create(int entsize);
int   hash_insert(HASH *h,char *str,int val,int dupok);
int	  hash_search(HASH *h,char *str,int *val);


 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "hash.h"

char *xstrdup(char *s);


/**********************************************************************
 *  
 **********************************************************************
 */
static int power_of_2(int nmax)
{
	int i,n;
	n = 1;
	for(i=0;i<29;i++,n<<=1) {
		if(nmax <= n) return n;	// nmax 以上の 2のべき数があればそれを返す.
	}
	return 0x4000;	//デフォルト.
//	return (-1);	//2^30以上 はダメ. (テーブルが２Ｇになる)
}

/**********************************************************************
 *  
 **********************************************************************
 */
void *xmalloc(int size)
{
	void *t = malloc(size);

	if(t==NULL) {
		printf("memory exhaust.\n");
		exit(-1);
	}
	return t;
}

/**********************************************************************
 * 	単方向リストの作成.
 **********************************************************************
 */
CELL *make_cell(char *str,int val,CELL *next)
{
	int size = sizeof(CELL) + strlen(str);
	CELL *hval = malloc(size);

	if(hval==NULL) {
		printf("memory exhaust.\n");
		exit(-1);
	}

	strcpy(hval->str,str);
	hval->val  = val;
	hval->next = next;

	return hval;
}

/**********************************************************************
 *  アドレスハッシュの初期化（クリア）
 **********************************************************************
 */
HASH *hash_create(int entsize)
{
    int len;
	HASH *h = xmalloc(sizeof(HASH));
	entsize = power_of_2(entsize);
	len = entsize * sizeof(void*);

	h->entry_size  = entsize;
	h->entry_mask  = entsize - 1;
	h->entry_table = xmalloc(len);

	//	エントリーを全部クリアします.
	memset(h->entry_table,0,len);

	return h;
}

//	文字列のハッシュ値を計算します。
static	int	calc_hash(char *s,int mask)
{
	int val=0;
	while(*s) {
		val *= 9;
		val += *s & 0xff;
		s++;
	}
    return val & mask;
}


/**********************************************************************
 *  検索：見つかったら1を返す. 存在しない場合は 0.
 **********************************************************************
 */
int	hash_search(HASH *h,char *str,int *val)
{
	int	 index = calc_hash(str,h->entry_mask);	//ハッシュ値からエントリーindexを得る.
	CELL *hval = h->entry_table[index];	//ハッシュセルのポインタを得る.

    while(hval) {
        if( strcmp(hval->str,str)==0 ) {	//文字列を実際に比較します.
            *val = hval->val;				//文字列と一緒に登録していた値を渡します.
            return 1;					//ハッシュ表の中に該当文字列が発見されました.
        }
        hval = hval->next;				//文字列がマッチしない場合、次のエントリーに移動.
    }

	return 0;	// 見つかりません.
}

/**********************************************************************
 *  挿入：重複があれば 1 を返す. 新規登録時は 0 を返す.
 **********************************************************************
 */
int hash_insert(HASH *h,char *str,int val,int dupok)
{
	int	 index = calc_hash(str,h->entry_mask);	//ハッシュ値からエントリーindexを得る.
	CELL *hval = (CELL *)h->entry_table[index];	//ハッシュセルのポインタを得る.
	//printf("hash_insert %s=%x\n",str,val);
	//
	// すでに同一キーが存在しているかどうか調べる.
	//
    while(hval) {
        if( strcmp(hval->str,str)==0 ) {	//文字列を実際に比較します.
			if(dupok) {
	            hval->val = val;			//値のみを書き換えます.
			}
			return 1;						//ハッシュ表の中に該当文字列が発見されました.
        }
        hval = hval->next;					//文字列がマッチしない場合、次のエントリーに移動.
    }

	hval = (CELL *)h->entry_table[index];	//ハッシュセルのポインタを再度取得.

	//
	// 単方向リストの最初に挿入する.
	//
	h->entry_table[index] = make_cell(str,val,hval);
	return 0;	// 新規挿入しました.
}


/**********************************************************************
 *  列挙:
 **********************************************************************
 */
int	hash_iterate(HASH *h , int (*func)(char *p,int v))
{
	CELL *hval;
	int   count = 0;
	int  i;
	int  m = h->entry_size;

	for(i=0;i<m;i++) {
		hval = h->entry_table[i];	//ハッシュセルのポインタを得る.
	    if(hval) {
			do {
				if(	func ) {
					func(hval->str,hval->val);
				}
				count++;
		        hval = hval->next;		//次のエントリーに移動.
			}while(hval);
        }
    }

	return count;	// 登録総数.
}

/**********************************************************************
 *  
 **********************************************************************
 */
