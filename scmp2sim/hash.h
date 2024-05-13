/**********************************************************************
 *  �n�b�V���̊Ǘ�
 **********************************************************************
 */
#ifndef	_hash__h_
#define	_hash__h_

// ===================================================
//  �n�b�V���\�̃G���g���[�A�h���X���w���f�[�^�̍\��.
//	�i�P���������N���X�g�ł�.�j
// ===================================================

typedef struct _CELL {
	struct _CELL *next;		//���̃G���g��.
	int   val;				//�o�^���������l.
	char  str[1];			//�ϒ��̕�����.
} CELL;


//	���[�g�Ǘ��\.
typedef	struct {
	int	  entry_size;
	int	  entry_mask;
	struct _CELL **entry_table;
} HASH;

//	�n�b�V���������A�o�^�A���� �̊e�֐�.
HASH *hash_create(int entsize);
int   hash_insert(HASH *h,char *str,int val,int dupok);
int	  hash_search(HASH *h,char *str,int *val);
int	  hash_iterate(HASH *h , int (*func)(char *p,int v));

//	�P�������X�g�̍쐬.
CELL *make_cell(char *str,int val,CELL *next);

#endif	//_hash__h_

/**********************************************************************
 *  
 **********************************************************************
 */
