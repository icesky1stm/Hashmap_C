/**************************************************
 * �������� : XipHashMap.c
 * �������� : icesky
 * ����汾 : 1.0.0
 * �������� : 2016.07.28
 * ������ : 
 *          �ο�java��hashmapʵ�ֻ���,ʵ����C������
 *          hashmapʵ�֣�֧�ּ��ֹ���:
 *          1.�½�
 *          2.����
 *          3.����/����
 *          4.��ȡ
 *          5.ɾ��
 *          6.�ж��Ƿ����
 *          7.��ӡ
 * ע������: 
 *          
 * �޸ļ�¼ :
 *************************************************/
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <stdarg.h>
#include "xiphashmap.h"

#define hmap_malloc(size) malloc(size)
#define HMAPFREE(x)  free(x);x=NULL;

#define MAXIMUM_CAPACITY  1 << 30 /*1 073 741 824 */
#define DEFAULT_CAPACITY  1 << 4 /*16*/
#define DEFAULT_FACTOR  0.75f
#define INTER_MAX_VALUE  1<<31  /*int�����ֵ*/
#define MALLOC_FLAG_NO   1      /*�������ڴ�*/


/*hashmap node�ṹ */
typedef struct st_xip_hashmap_node
{
    char * key;
    union
    {
        void *ptr;            /*ע��ptr����û�з����ڴ�,ֻ��ָ��*/
        int num;              /*ihashmapʹ��*/
    }value;
    struct st_xip_hashmap_node *next;
    unsigned int  hash;
} TxipHashmapNode;

/*hashmap �ṹ*/
typedef  struct
{
    TxipHashmapNode ** table; /*T��*/
    unsigned int length;      /*Ͱ��С*/
    unsigned int size;        /*ʵ�ʴ�С*/
    unsigned int threshold;   /*�����ٽ�ֵ*/
    int malloc_flag;          /*�Ƿ�Ϊvalue�����ڴ�*/
    float factor;             /*��������,Ĭ��Ϊ0.75(0-1)*/
} TxipHashmap;


#define HMAPLOG(...) AppLog(__FILE__,__LINE__,__VA_ARGS__)
static void AppLog(char *file, long line, char *level, char *fmtstr, ...)
{
    va_list ap;
    char   tmpstr[501];

    memset(tmpstr, 0x0, sizeof(tmpstr));

    va_start(ap, fmtstr);

    vsprintf(tmpstr, fmtstr, ap);
    va_end(ap);

    printf("[%s][%s][%03ld]", level, file, line);
    printf("[%s]\n", tmpstr);

    return ;
}

/*** static ���� ***/
/*�½�T��*/
static TxipHashmapNode ** create_T_table( int opacity);
/*�½�hashmap�ڵ�*/
static TxipHashmapNode * create_hashmap_node( char * key, void * value, TxipHashmapNode * next, int malloc_flag, int size);
/*�ؽ�hashmap*/
static int rebuild_hash( TxipHashmap * hashmap);
/*hash�㷨*/
static unsigned int XipHash(char * key);

/********************************************************************
 * ��������: XipHashmapPrint
 * ��������: ��ӡhashmap�����е�Ԫ��,��Ҫ���ڵ��Ժͼ��ɢ�зֲ�
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
int XipHashmapPrint( void * hashmap, void (*ref_func)(int,char *, void *))
{
    TxipHashmap * map = ( TxipHashmap *)hashmap;

    int idx = 0;

    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;

    /** ��ӡ��־ **/
    if( map != NULL)
    {
        HMAPLOG("I", "HashMap:length(%d),size(%d),threshold(%d),malloc_flag(%d)",
                map->length, map->size, map->threshold, map->malloc_flag);
        for ( idx = 0; idx < map->length; idx++)
        {
            for( e = map->table[idx]; e != NULL; )
            {
                next = e->next;
                if( ref_func == NULL)
                {
                    HMAPLOG("I", "Node[%d]:hash[%d],key[%s],value[%x][%s]", 
                            idx, e->hash, e->key, e->value.ptr, (char *)e->value.ptr);
                }
                else
                {
                    ref_func(idx, e->key, e->value.ptr);
                }
                e = next;
            }
        }
    }
    
    /** ��ӡ�ֲ����ͼ **/
    int i = 0;
    if( map != NULL)
    {
        for( idx = 0; idx < map->length; idx++)
        {
            i = 0;
            for( e= map->table[idx] ; e!= NULL; )
            {
                next = e->next;
                i++;
                e = next;
            }
            HMAPLOG("I","%08ld:%*d", idx, i, i);
        }
    }

    return 0;
}

 /********************************************************************
 * ��������: XipHashmapNew
 * ��������: �����е�Ĭ�ϲ���������һ��hashmap
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
void * XipHashmapNew()
{
    return XipHashmapInit( 0, 0.00, 0); /*ȫĬ��ѡ��*/
}

/********************************************************************
 * ��������: XipHashmapInit
 * ��������: ָ����ʼ�������������Ӻ�value�洢��ʽ������һ��hashmap
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
void * XipHashmapInit( int opacity, float factor, int malloc_flag)
{
    TxipHashmap * map = ( TxipHashmap *)hmap_malloc(sizeof( TxipHashmap));
    if( map == NULL)
    {
        HMAPLOG("E","��ʼ��hashmap�ڴ�ʧ��!!!");
        return NULL;
    }
    memset( map, 0x00, sizeof(TxipHashmap));

    if ( opacity < DEFAULT_CAPACITY )
        opacity = DEFAULT_CAPACITY;
    if( opacity > MAXIMUM_CAPACITY)
        opacity = MAXIMUM_CAPACITY;
    if ( factor >= -0.005 && factor <=0.005 )
        factor = DEFAULT_FACTOR;

    if( opacity <= 1)
    {
        HMAPLOG("E","��ʼ�����������1!!!");
        HMAPFREE(map);
        map = NULL;
        return NULL;
    }
    if( factor < 0.00 || factor > 1.00)
    {
        HMAPLOG("E","��������ȡֵ����Ϊ[0.00-1.00],�����μ�������Ϊ[%.2f]", factor);
        HMAPFREE(map);
        map = NULL;
        return NULL;
    }

    map->table = create_T_table( opacity);
    if( map->table == NULL)
    {
        HMAPLOG("E","����hashֵ��T��ʧ��!!!");
        HMAPFREE(map);
        map = NULL;
        return NULL;
    }

    map->length = opacity;
    map->size = 0;
    map->factor = factor;
    map->threshold = (unsigned int)( factor * opacity);
    map->malloc_flag = malloc_flag;

    return (void *)map;
}
 
 /********************************************************************
 * ��������: XipHashmapDestory
 * ��������: �ͷ�hashmap���ڴ���Դ
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
int XipHashmapDestory( void * hashmap)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;

    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;
    register unsigned int idx;

    /** �ͷ�hashmap node �ڴ� **/
    if( map != NULL)
    {
        /** �ͷ�hashmap��node�ռ� **/
        for ( idx = 0; idx < map->length; idx++)
        {
            for( e = map->table[idx]; e != NULL; )
            {
                next = e->next;
                /* �ͷ�node��value�ռ�*/
                if( map->malloc_flag == MALLOC_FLAG_NO)
                {
                    HMAPFREE(e);
                }
                else
                {
                    HMAPFREE(e->value.ptr);
                    e->value.ptr = NULL;
                }
                e = next;
            }
        }

        /** �ͷ�hashmap��T�� **/
        HMAPFREE( map->table);
        map->table = NULL;
        
        /** �ͷ�hashmap����Ŀռ� **/
        HMAPFREE( map);
        map = NULL;
    }


    return 0;
}

 /********************************************************************
 * ��������: XipHashmapDestory
 * ��������: ��key��value����hashmap�У����key�Ѵ��ڣ��൱��Set����
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
void * XipHashmapPut( void * hashmap, char * key, void * value, int size)
{
    TxipHashmap * map = (TxipHashmap *)hashmap;

    if( map == NULL)
    {
        HMAPLOG("E","hashmapָ��Ϊ��,�����Ƿ��Ѿ�����XipHashmapNew/XipHashmapInit,����÷����쳣");
        return NULL;
    }

    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Keyֵ����Ϊ�ַ���,��Keyֵ����Ϊ�ջ���NULL!!!");
        return NULL;
    }
    if ( value == NULL)
    {
        HMAPLOG("E","value������Ϊ��(NULL)");
        return NULL;
    }
    if ( size <= 0)
    {
        HMAPLOG("E","ֵ�Ĵ�Сsize������Ϊ0");
        return NULL;
    }

    void * oldvalue = NULL;
    TxipHashmapNode *e = NULL;
    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    /*����ҵ���key,���൱��set*/
    for ( e = map->table[idx]; e != NULL ; e = e->next)
    {
        /*�ο�java��hashmapʵ��,�ȱȽ�hash,�ٱȽ�strcmp*/
        if ( hcode == e->hash && (key == e->key || strcmp( key, e->key) == 0))
        {
            oldvalue = e->value.ptr;
            if( map->malloc_flag == 1) /*�������ڴ�*/
            {
                e->value.ptr = value;
            }
            else 
            {
                HMAPFREE(e->value.ptr);
                e->value.ptr = NULL;
                e->value.ptr = hmap_malloc(size);
                if( e->value.ptr == NULL)
                {
                    HMAPLOG("E","Ϊvalue�����ڴ�ʱ�쳣!!!");
                    return NULL;
                }
                memset( e->value.ptr, 0x00, size);
                memcpy( e->value.ptr, value, size);
                return e->value.ptr;
            }
            return oldvalue; /*��ԭvalue���ظ�������*/
        }
    }

    /*�½�node*/
    map->table[idx] = create_hashmap_node( key, value, map->table[idx], map->malloc_flag, size);
    if( map->table[idx] == NULL)
    {
        HMAPLOG("E","�½�hashnode�ڵ��쳣malloc_flag[%d],size[%d]!!!", map->malloc_flag, size);
        return NULL; /*���ع̶�ֵ*/
    }
    
    /*�޸�hashmap*/
    e = map->table[idx];
    e->hash = hcode;
    map->size++;

    /*��������ٽ�ֵ�����ؽ�hash��*/
    if( map->size > map->threshold)
    {
        if( rebuild_hash(map) != 0)
        {
            HMAPLOG("E","�ؽ�hash�����!!!");
            return NULL; /*���ع̶�ֵ*/
        }
    }

    return map->table[idx]->value.ptr; /*�����ڴ��ַ*/
}

 /********************************************************************
 * ��������: XipHashmapGet
 * ��������: ����Keyֵ��ȡ��Ӧ��valueֵ
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
void * XipHashmapGet( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    if( map == NULL)
    {
        HMAPLOG("E","hashmapָ��Ϊ��,�����Ƿ��Ѿ�����XipHashmapNew/XipHashmapInit,����÷����쳣");
        return NULL;
    }

    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Keyֵ����Ϊ�ַ���,��Keyֵ����Ϊ�ջ���NULL!!!");
        return NULL;
    }

    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    TxipHashmapNode * e = NULL;

    for ( e = map->table[idx]; e != NULL ; e = e->next)
    {
        /*�ο�java��hashmapʵ��,�ȱȽ�hash,�ٱȽ�strcmp, key == e->key��������ihashmap�õ�,����չ*/
        if ( hcode == e->hash && (key == e->key || strcmp( key, e->key) == 0))
        {
            return e->value.ptr;
        }
    }
    return NULL;
}

 /********************************************************************
 * ��������: XipHashmapExists
 * ��������: �ж�Key�Ƿ���hashmap�д���
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
int XipHashmapExists( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    if( map == NULL)
    {
        HMAPLOG("E","hashmapָ��Ϊ��,�����Ƿ��Ѿ�����XipHashmapNew/XipHashmapInit,����÷����쳣");
        return -5;
    }
    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Keyֵ����Ϊ�ַ���,��Keyֵ����Ϊ�ջ���NULL!!!");
        return -10;
    }

    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    TxipHashmapNode * e = NULL;

    for ( e = map->table[idx]; e != NULL ; e = e->next)
    {
        /*�ο�java��hashmapʵ��,�ȱȽ�hash,�ٱȽ�strcmp*/
        if ( hcode == e->hash && (key == e->key || strcmp( key, e->key) == 0))
        {
            return 1;
        }
    }

    return 0;
}

 /********************************************************************
 * ��������: XipHashmapDestory
 * ��������: ɾ��map�ж�Ӧ��Key��VALUE�Ľڵ�
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiphashmap.h������
 *********************************************************************/
int  XipHashmapRemove( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    if( map == NULL)
    {
        HMAPLOG("E","hashmapָ��Ϊ��,�����Ƿ��Ѿ�����XipHashmapNew/XipHashmapInit,����÷����쳣");
        return -5;
    }
    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Keyֵ����Ϊ�ַ���,��Keyֵ����Ϊ�ջ���NULL!!!");
        return -10;
    }

    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    TxipHashmapNode * e = NULL;
    TxipHashmapNode * prev = NULL;

    for ( e = map->table[idx]; e != NULL ; prev = e, e = e->next)
    {
        /*�ο�java��hashmapʵ��,�ȱȽ�hash,�ٱȽ�strcmp*/
        if ( hcode == e->hash && (key == e->key || strcmp( key, e->key) == 0))
        {
            if( prev == NULL)
                map->table[idx] = e->next;
            else
                prev->next = e->next;

            if( map->malloc_flag == MALLOC_FLAG_NO) /*�������ڴ�*/
            {
                ;
            }
            else
            {
                HMAPFREE(e->value.ptr);
                e->value.ptr = NULL;
            }

            HMAPFREE(e);
            map->size--;
            return 0;
        }
    }

    return 0;
}

/********************************************************************
 * ��������: create_T_table
 * ��������: ����ָ��node�������ڴ�ָ�룬�ڡ��㷨���ۡ�����ɢ�б��У�����λ��ΪT��
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ڲ����ú���.ע�ⷵ��ֵ��ָ��TxipHashmapNode��ָ���ָ��
 *********************************************************************/
static TxipHashmapNode ** create_T_table( int opacity)
{
    register unsigned int i=0;

    TxipHashmapNode ** table = (TxipHashmapNode **)hmap_malloc( sizeof(TxipHashmapNode*) *opacity);
    if( table == NULL)
    {
        return NULL;
    }
    memset( table, 0x00, sizeof(TxipHashmapNode*) * opacity);

    /*��ʼ��*/
    for ( i = 0; i < opacity; i++)
        table[i] = NULL;

    return table;
}

/********************************************************************
 * ��������: create_hashmap_node
 * ��������: ���������key,value��node�ڵ�
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ڲ����ú���. ���malloc_flag��1�Ļ�����size��Ч����Ϊ����Ҫ
        Ϊvalue�����ڴ档
 *********************************************************************/
static TxipHashmapNode * create_hashmap_node( char * key, void * value, TxipHashmapNode * next, int malloc_flag, int size)
{
    TxipHashmapNode * node = ( TxipHashmapNode *) hmap_malloc( sizeof(TxipHashmapNode));
    if ( node == NULL)
    {
        return NULL;
    }
    memset( node, 0x00, sizeof( TxipHashmapNode));

    if( malloc_flag ==  MALLOC_FLAG_NO) /*�������ڴ�*/
    {
        node->key = key;
        node->value.ptr = value;
    }
    else 
    {
        /*�޶�KEY�������ַ�������*/
        node->key = hmap_malloc(strlen(key)+1);
        if( node->key == NULL)
        {
            return NULL;
        }
        memset(node->key, 0x00, strlen(key)+1);
        memcpy( node->key, key, strlen(key)+1);

        node->value.ptr = hmap_malloc(size);
        if( node->value.ptr == NULL)
        {
            return NULL;
        }
        memset(node->value.ptr, 0x00, size);
        memcpy( node->value.ptr, value, size);
    }
    node->next = next;

    return node;
}

/********************************************************************
 * ��������: rebuild_hash
 * ��������: ��ǰhashmap�е�size�ﵽ���ٽ�ֵ��ʱ����Ҫ���´��������hashmap
            �����д洢��
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ڲ����ú���. ע���ؽ�ʧ����Ҫ�жϷ���ֵ
 *********************************************************************/
static int rebuild_hash( TxipHashmap * map)
{
    register unsigned int i = 0;
    unsigned int idx = 0;

    /*����ﵽ�����,����rebuild*/
    if( map->length == MAXIMUM_CAPACITY)
    {
        map->threshold = INTER_MAX_VALUE;
        return 0;
    }

    unsigned int length =  map->length*2;
    if( length > MAXIMUM_CAPACITY)
        length = MAXIMUM_CAPACITY;

    TxipHashmapNode *e=NULL;
    TxipHashmapNode *next=NULL;

    TxipHashmapNode ** newtable = create_T_table( length);
    if ( newtable == NULL)
    {
        return -5;
    }

    for( i = 0; i < map->length; i++)
    {
        /*����*/
        e = *(map->table + i);
        if( e != NULL)
        {
            do
            {
                next = e->next;

                idx = e->hash % length;

                e->next = newtable[idx]; /*���newtable[idx]��ֵ�Ѿ�������,������Ὣ��ֵ��Ϊnewtable[idx]->idx*/

                newtable[idx] = e;

                e = next;
            }while( e != NULL);
        }
    }

    /*�ͷ�oldtable*/
    HMAPFREE(map->table);
    map->table = newtable;
    map->length = length;
    map->threshold = (unsigned int)(map->factor * length);

    return 0;

}

/********************************************************
 * ��������: XipHash
 * ��������: ͨ��key������hashֵ���㷨���㷨Խ�ã�ɢ�е�һ�·ֲ���Խ��
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ڲ����ú���.
 * hash�㷨,�˴�ʹ�õ��� simple BKDR hash algorithm
 * ΪJAVA��JDK��ʹ�õ��㷨
 * Ҳ����ʹ��PHP��ʹ�õ�time33�㷨(DJP hash algorithm��
 * ������ʹ�ñ�ѩ��˾�ĵ�One-Way-Hash(�ų�����hash�㷨)
 * 
 * ֮����û��ʹ�����Ƽ����㷨,����Ϊ�Ұѻ�����������.���
 * �㿴��������ط�,������ͼ�Ż��㷨�Ļ�,����ʹ���ҽ����
 * �㷨���г���.
 * ��������ȵ�ϵͳƿ��׷�ݵ��������ʱ��,����������˸�
 * �������㷨����֤����һ��ɢ����.o(��_��)o
 *                              icesky.2016.07.08
 *********************************************************/
static unsigned int XipHash(char * key)
{
    register uint32_t h = 0;
    uint32_t seed = 131;    //31 131 1313 13131

    while ( *key != '\0' )
    {
        h = h * seed + ( *key++ );
    }

    return (h & 0x7FFFFFFF);
}
