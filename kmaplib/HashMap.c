/**************************************************
 * �������� : HashMap.c
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
 *          
 * �޸ļ�¼ :
 *************************************************/
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <stdarg.h>
#include "hashmap.h"

#define hmap_malloc(size) malloc(size)
#define HMAPFREE(x)  free(x);x=NULL;
#define XIP_HASHMAP_REBUILD_ERR (void *)-1

#define MAXIMUM_CAPACITY  1 << 30 /*1 073 741 824 */
#define DEFAULT_CAPACITY  1 << 4 /*16*/
#define DEFAULT_FACTOR  0.75f
#define INTER_MAX_VALUE  1<<31  /*int�����ֵ*/

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
    float factor;             /*��������,Ĭ��Ϊ0.75(0-1)*/
} TxipHashmap;


#define HMAPLOG(...) AppLog(__FILE__,__LINE__,__VA_ARGS__)
static void AppLog(char *file, long line, char *level, char *fmtstr, ...)
{
    va_list ap;
    char   tmpstr[501];
    char   loglevel[11];
    FILE   *fp;

    memset(tmpstr, 0x0, sizeof(tmpstr));

    va_start(ap, fmtstr);

    vsprintf(tmpstr, fmtstr, ap);
    va_end(ap);

    printf("[%s][%s][%03d]", level, file, line);
    printf("[%s]\n", tmpstr);

    return ;
}

/*** static ���� ***/
/*�½�T��*/
static TxipHashmapNode ** create_T_table( int opacity);
/*�½�hashmap�ڵ�*/
static TxipHashmapNode * create_hashmap_node( char * key, void * value, TxipHashmapNode * next);
/*�ؽ�hashmap*/
static int rebuild_hash( TxipHashmap * hashmap);
/*hash�㷨*/
static unsigned int XipHash(char * key);

/*********************************************************************
 * ��ڲ���: hashmap��������ָ��
 * ���ڲ���: ��
 * ����ֵ  : int ��Զ����0�����Բ��ж�
 * ��������: ��ӡhashmap�����е�Ԫ��,��Ҫ���ڵ��Ժͼ��ɢ�зֲ�
 *
 *********************************************************************/
int XipHashmapPrint( void * hashmap)
{
    TxipHashmap * map = ( TxipHashmap *)hashmap;
    HMAPLOG("I", "HashMap:length[%d],size[%d],threshold[%d];�ֲ��������:", map->length, map->size, map->threshold);

    int i = 0;
    int idx = 0;

    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;

    /** ��ӡ��־ **/
    if( map != NULL)
    {
        for ( idx = 0; idx < map->length; idx++)
        {
            for( e = map->table[idx]; e != NULL; )
            {
                next = e->next;
                HMAPLOG("I", "Node[%d]:hash[%d],key[%s],value[%s]", idx, e->hash, e->key, e->value.ptr);
                e = next;
            }
        }
    }
    
    /** ��ӡ�ֲ����ͼ **/
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

/*********************************************************************
 * ��ڲ���: ��
 * ���ڲ���: ��
 * ����ֵ  : ������hashmap��ָ��,ǿ��ת������void *����,�������
 * ��������: ��Ĭ�Ϸ�ʽ����һ��hashmap,Ĭ�ϵĳ�ʼ������16,����������0.75f
 *
 *********************************************************************/
void * XipHashmapNew()
{
    return XipHashmapInit( 0, 0.00);
}

/*********************************************************************
 * ��ڲ���: int opacity ��ʼ����,  float factor ��������
 * ���ڲ���: ��
 * ����ֵ  : ������hashmap��ָ��,ǿ��ת������void *����,�������
 * ��������: ���մ������Ϣ���г�ʼhashmap����,
 *          ���opacityΪ0,��ΪĬ��ֵ16,���factorΪ0,��Ĭ��Ϊ0.75f
 *
 *********************************************************************/
void * XipHashmapInit( int opacity, float factor)
{
    TxipHashmap * map = ( TxipHashmap *)hmap_malloc(sizeof( TxipHashmap));

    if ( opacity < DEFAULT_CAPACITY )
        opacity = DEFAULT_CAPACITY;
    if( opacity > MAXIMUM_CAPACITY)
        opacity = MAXIMUM_CAPACITY;
    if ( factor >= -0.005 && factor <=0.005 )
        factor = DEFAULT_FACTOR;

    if( opacity <= 1)
    {
        HMAPLOG("E","��ʼ�����������1!!!");
        return NULL;
    }
    if( factor < 0.00 || factor > 1.00)
    {
        HMAPLOG("E","��������ȡֵ����Ϊ[0.00-1.00],�����μ�������Ϊ[%.2f]", factor);
        return NULL;
    }

    map->table = create_T_table( opacity);
    if( map->table == NULL)
    {
        HMAPLOG("E","����hashֵ��T��ʧ��!!!");
        return NULL;
    }

    map->length = opacity;
    map->size = 0;
    map->factor = factor;
    map->threshold = (unsigned int)( factor * opacity);

    return (void *)map;
}

/*********************************************************************
 * ��ڲ���: hashmapָ��
 * ���ڲ���: ��
 * ����ֵ  : int , ��Զ����0���Բ��ж�
 * ��������: ���մ������Ϣ���г�ʼhashmap����,
 *          ���opacityΪ0,��ΪĬ��ֵ16,���factorΪ0,��Ĭ��Ϊ0.75f
 *
 *********************************************************************/
int XipHashmapDestory( void * hashmap)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    register unsigned int idx;
    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;

    /** �ͷ�hashmap node �ڴ� **/
    if( map != NULL)
    {
        for ( idx = 0; idx < map->length; idx++)
        {
            for( e = map->table[idx]; e != NULL; )
            {
                next = e->next;
                HMAPFREE(e);
                e = next;
            }
        }
    }

    /** �ͷ�hashmap��T�� **/
    HMAPFREE( map->table);
    map->table = NULL;

    return 0;
}

/*********************************************************************
 * ��ڲ���: hashmapָ��, char * key, void * value
 * ���ڲ���: ��
 * ����ֵ  : ���� void * oldvalue ��ָ��,���Ϊ�����򷵻�NULL
 *          ���putʧ���򷵻�XIP_HASHMAP_PUT_ERR
 * ��������: ����key��value���뵽hashmap��,���map���Ѿ����ڸ�key��ֵ
 *          ���滻�����µ�value,ͬʱ���ؾ�valueָ��
 *          ���put֮�󣬴ﵽ���ٽ�ֵ,�����´���hashmap
 *********************************************************************/
void * XipHashmapPut( void * hashmap, char * key, void * value)
{
    TxipHashmap * map = (TxipHashmap *)hashmap;

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
            e->value.ptr = value;
            return oldvalue; /*��ԭvalue���ظ�������*/
        }
    }

    /*�½�node*/
    map->table[idx] = create_hashmap_node( key, value, map->table[idx]);
    if( map->table[idx] == NULL)
    {
        HMAPLOG("E","�½�hashnode�ڵ��쳣!!!");
        return XIP_HASHMAP_PUT_ERR; /*���ع̶�ֵ*/
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
            return XIP_HASHMAP_PUT_ERR; /*���ع̶�ֵ*/
        }
    }

    return NULL;
}

/*********************************************************************
 * ��ڲ���: hashmapָ��, char * key
 * ���ڲ���: ��
 * ����ֵ  : ���� void * value ��ָ��
 * ��������: ����keyֵ��hashmap��ȡ��value��ָ�뷵��
 *********************************************************************/
void * XipHashmapGet( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
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

/*********************************************************************
 * ��ڲ���: hashmapָ��, char * key
 * ���ڲ���: ��
 * ����ֵ  : int ����ֵXIP_HASHMAP_EXIST_TURE(1), XIP_HASHMAP_EXIST_FALSE(0)
 * ��������: ����keyֵ��hashmap�в����Ƿ����,���ڷ�����,�����ڷ��ؼ�
 *********************************************************************/
int XipHashmapExists( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
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

/*********************************************************************
 * ��ڲ���: hashmapָ��, char * key
 * ���ڲ���: ��
 * ����ֵ  : void * value
 * ��������: ����keyֵ��hashmap��ɾ��key��Ӧ��node�ڵ�,���ɾ���ɹ���
 *          �򷵻�ɾ���ڵ��value�ĵ�ַ,δ�ҵ��ڵ��򷵻�NULL
 *********************************************************************/
void * XipHashmapRemove( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    void * oldvalue = NULL;
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

            oldvalue = e->value.ptr;

            HMAPFREE(e);
            map->size--;
            return oldvalue;
        }
    }

    return NULL;
}

static TxipHashmapNode ** create_T_table( int opacity)
{
    register unsigned int i=0;

    TxipHashmapNode ** table = (TxipHashmapNode **)hmap_malloc( sizeof(TxipHashmapNode*) *opacity);
    if( table == NULL)
    {
        return NULL;
    }

    /*��ʼ��*/
    for ( i = 0; i < opacity; i++)
        table[i] = NULL;

    return table;
}

static TxipHashmapNode * create_hashmap_node( char * key, void * value, TxipHashmapNode * next)
{
    TxipHashmapNode * node = ( TxipHashmapNode *) hmap_malloc( sizeof(TxipHashmapNode));
    if ( node == NULL)
    {
        return NULL;
    }

    node->key = key;
    node->value.ptr = value;
    node->next = next;

    return node;
}
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
 * hash�㷨,�˴�ʹ�õ��� simple BKDR hash algorithm
 * ΪJAVA��JDK��ʹ�õ��㷨
 * Ҳ����ʹ��PHP��ʹ�õ�time33�㷨(DJP hash algorithm��
 * ������ʹ�ñ�ѩ��˾�ĵ�One-Way-Hash,�ų�����hash�㷨
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
