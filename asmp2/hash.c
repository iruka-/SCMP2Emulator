/**********************************************************************
 *  �n�b�V���̊Ǘ�
 **********************************************************************
 *
 * �n�b�V���\�͂Q�ׂ̂���̃G���g���[�\�ō\������܂��B
 * �n�b�V���l�������Z���̍\���́A�o�����l.������p�̃y�A�ł��B

 * �\�̓o�^�A�����͕�����L�[�ōs���A���ʂ�32bit�����l�݂̂ł��B

 * �n�b�V���\�̃G���g���[���͂Q�ׂ̂��ł���ΔC�ӌ��Ő�������܂��B
 * ������̃n�b�V���l���Փ˂����ꍇ�́A�Z���̒P�������X�g��H��܂��B
 * �\�̍폜�͏o���܂���B



//	�n�b�V���������A�o�^�A���� �̊e�֐�.

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
		if(nmax <= n) return n;	// nmax �ȏ�� 2�ׂ̂���������΂����Ԃ�.
	}
	return 0x4000;	//�f�t�H���g.
//	return (-1);	//2^30�ȏ� �̓_��. (�e�[�u�����Q�f�ɂȂ�)
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
 * 	�P�������X�g�̍쐬.
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
 *  �A�h���X�n�b�V���̏������i�N���A�j
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

	//	�G���g���[��S���N���A���܂�.
	memset(h->entry_table,0,len);

	return h;
}

//	������̃n�b�V���l���v�Z���܂��B
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
 *  �����F����������1��Ԃ�. ���݂��Ȃ��ꍇ�� 0.
 **********************************************************************
 */
int	hash_search(HASH *h,char *str,int *val)
{
	int	 index = calc_hash(str,h->entry_mask);	//�n�b�V���l����G���g���[index�𓾂�.
	CELL *hval = h->entry_table[index];	//�n�b�V���Z���̃|�C���^�𓾂�.

    while(hval) {
        if( strcmp(hval->str,str)==0 ) {	//����������ۂɔ�r���܂�.
            *val = hval->val;				//������ƈꏏ�ɓo�^���Ă����l��n���܂�.
            return 1;					//�n�b�V���\�̒��ɊY�������񂪔�������܂���.
        }
        hval = hval->next;				//�����񂪃}�b�`���Ȃ��ꍇ�A���̃G���g���[�Ɉړ�.
    }

	return 0;	// ������܂���.
}

/**********************************************************************
 *  �}���F�d��������� 1 ��Ԃ�. �V�K�o�^���� 0 ��Ԃ�.
 **********************************************************************
 */
int hash_insert(HASH *h,char *str,int val,int dupok)
{
	int	 index = calc_hash(str,h->entry_mask);	//�n�b�V���l����G���g���[index�𓾂�.
	CELL *hval = (CELL *)h->entry_table[index];	//�n�b�V���Z���̃|�C���^�𓾂�.
	//printf("hash_insert %s=%x\n",str,val);
	//
	// ���łɓ���L�[�����݂��Ă��邩�ǂ������ׂ�.
	//
    while(hval) {
        if( strcmp(hval->str,str)==0 ) {	//����������ۂɔ�r���܂�.
			if(dupok) {
	            hval->val = val;			//�l�݂̂����������܂�.
			}
			return 1;						//�n�b�V���\�̒��ɊY�������񂪔�������܂���.
        }
        hval = hval->next;					//�����񂪃}�b�`���Ȃ��ꍇ�A���̃G���g���[�Ɉړ�.
    }

	hval = (CELL *)h->entry_table[index];	//�n�b�V���Z���̃|�C���^���ēx�擾.

	//
	// �P�������X�g�̍ŏ��ɑ}������.
	//
	h->entry_table[index] = make_cell(str,val,hval);
	return 0;	// �V�K�}�����܂���.
}


/**********************************************************************
 *  ��:
 **********************************************************************
 */
int	hash_iterate(HASH *h , int (*func)(char *p,int v))
{
	CELL *hval;
	int   count = 0;
	int  i;
	int  m = h->entry_size;

	for(i=0;i<m;i++) {
		hval = h->entry_table[i];	//�n�b�V���Z���̃|�C���^�𓾂�.
	    if(hval) {
			do {
				if(	func ) {
					func(hval->str,hval->val);
				}
				count++;
		        hval = hval->next;		//���̃G���g���[�Ɉړ�.
			}while(hval);
        }
    }

	return count;	// �o�^����.
}

/**********************************************************************
 *  
 **********************************************************************
 */
