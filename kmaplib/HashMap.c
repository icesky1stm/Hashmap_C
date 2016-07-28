/**************************************************
 * 程序名称 : HashMap.c
 * 程序作者 : icesky
 * 程序版本 : 1.0.0
 * 创建日期 : 2016.07.28
 * 程序功能 : 
 *          参考java的hashmap实现机制,实现了C基本的
 *          hashmap实现，支持几种功能:
 *          1.新建
 *          2.销毁
 *          3.加入/设置
 *          4.获取
 *          5.删除
 *          6.判断是否存在
 *          7.打印
 *          
 * 修改记录 :
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
#define INTER_MAX_VALUE  1<<31  /*int的最大值*/

/*hashmap node结构 */
typedef struct st_xip_hashmap_node
{
    char * key;
    union
    {
        void *ptr;            /*注意ptr本身没有分配内存,只是指针*/
        int num;              /*ihashmap使用*/
    }value;
    struct st_xip_hashmap_node *next;
    unsigned int  hash;
} TxipHashmapNode;

/*hashmap 结构*/
typedef  struct
{
    TxipHashmapNode ** table; /*T表*/
    unsigned int length;      /*桶大小*/
    unsigned int size;        /*实际大小*/
    unsigned int threshold;   /*加载临界值*/
    float factor;             /*加载因子,默认为0.75(0-1)*/
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

/*** static 声明 ***/
/*新建T表*/
static TxipHashmapNode ** create_T_table( int opacity);
/*新建hashmap节点*/
static TxipHashmapNode * create_hashmap_node( char * key, void * value, TxipHashmapNode * next);
/*重建hashmap*/
static int rebuild_hash( TxipHashmap * hashmap);
/*hash算法*/
static unsigned int XipHash(char * key);

/*********************************************************************
 * 入口参数: hashmap的无类型指针
 * 出口参数: 无
 * 返回值  : int 永远返回0，可以不判断
 * 函数功能: 打印hashmap中所有的元素,主要用于调试和检查散列分布
 *
 *********************************************************************/
int XipHashmapPrint( void * hashmap)
{
    TxipHashmap * map = ( TxipHashmap *)hashmap;
    HMAPLOG("I", "HashMap:length[%d],size[%d],threshold[%d];分布情况如下:", map->length, map->size, map->threshold);

    int i = 0;
    int idx = 0;

    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;

    /** 打印日志 **/
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
    
    /** 打印分布情况图 **/
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
 * 入口参数: 无
 * 出口参数: 无
 * 返回值  : 创建的hashmap的指针,强制转换成了void *类型,方便调用
 * 函数功能: 以默认方式创建一个hashmap,默认的初始容量是16,加载因子是0.75f
 *
 *********************************************************************/
void * XipHashmapNew()
{
    return XipHashmapInit( 0, 0.00);
}

/*********************************************************************
 * 入口参数: int opacity 初始容量,  float factor 加载因子
 * 出口参数: 无
 * 返回值  : 创建的hashmap的指针,强制转换成了void *类型,方便调用
 * 函数功能: 按照传入的信息进行初始hashmap创建,
 *          如果opacity为0,则为默认值16,如果factor为0,则默认为0.75f
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
        HMAPLOG("E","初始容量必须大于1!!!");
        return NULL;
    }
    if( factor < 0.00 || factor > 1.00)
    {
        HMAPLOG("E","加载因子取值区间为[0.00-1.00],但本次加载因子为[%.2f]", factor);
        return NULL;
    }

    map->table = create_T_table( opacity);
    if( map->table == NULL)
    {
        HMAPLOG("E","创建hash值的T表失败!!!");
        return NULL;
    }

    map->length = opacity;
    map->size = 0;
    map->factor = factor;
    map->threshold = (unsigned int)( factor * opacity);

    return (void *)map;
}

/*********************************************************************
 * 入口参数: hashmap指针
 * 出口参数: 无
 * 返回值  : int , 永远返回0可以不判断
 * 函数功能: 按照传入的信息进行初始hashmap创建,
 *          如果opacity为0,则为默认值16,如果factor为0,则默认为0.75f
 *
 *********************************************************************/
int XipHashmapDestory( void * hashmap)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    register unsigned int idx;
    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;

    /** 释放hashmap node 内存 **/
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

    /** 释放hashmap的T表 **/
    HMAPFREE( map->table);
    map->table = NULL;

    return 0;
}

/*********************************************************************
 * 入口参数: hashmap指针, char * key, void * value
 * 出口参数: 无
 * 返回值  : 返回 void * oldvalue 的指针,如果为新增则返回NULL
 *          如果put失败则返回XIP_HASHMAP_PUT_ERR
 * 函数功能: 根据key和value放入到hashmap中,如果map中已经存在该key的值
 *          则替换成最新的value,同时返回旧value指针
 *          如果put之后，达到了临界值,则重新创建hashmap
 *********************************************************************/
void * XipHashmapPut( void * hashmap, char * key, void * value)
{
    TxipHashmap * map = (TxipHashmap *)hashmap;

    void * oldvalue = NULL;
    TxipHashmapNode *e = NULL;
    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    /*如果找到了key,则相当于set*/
    for ( e = map->table[idx]; e != NULL ; e = e->next)
    {
        /*参考java的hashmap实现,先比较hash,再比较strcmp*/
        if ( hcode == e->hash && (key == e->key || strcmp( key, e->key) == 0))
        {
            oldvalue = e->value.ptr;
            e->value.ptr = value;
            return oldvalue; /*将原value返回给调用者*/
        }
    }

    /*新建node*/
    map->table[idx] = create_hashmap_node( key, value, map->table[idx]);
    if( map->table[idx] == NULL)
    {
        HMAPLOG("E","新建hashnode节点异常!!!");
        return XIP_HASHMAP_PUT_ERR; /*返回固定值*/
    }
    
    /*修改hashmap*/
    e = map->table[idx];
    e->hash = hcode;
    map->size++;

    /*如果触发临界值，则重建hash表*/
    if( map->size > map->threshold)
    {
        if( rebuild_hash(map) != 0)
        {
            HMAPLOG("E","重建hash表错误!!!");
            return XIP_HASHMAP_PUT_ERR; /*返回固定值*/
        }
    }

    return NULL;
}

/*********************************************************************
 * 入口参数: hashmap指针, char * key
 * 出口参数: 无
 * 返回值  : 返回 void * value 的指针
 * 函数功能: 根据key值从hashmap中取得value的指针返回
 *********************************************************************/
void * XipHashmapGet( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    TxipHashmapNode * e = NULL;

    for ( e = map->table[idx]; e != NULL ; e = e->next)
    {
        /*参考java的hashmap实现,先比较hash,再比较strcmp, key == e->key是用来给ihashmap用的,待扩展*/
        if ( hcode == e->hash && (key == e->key || strcmp( key, e->key) == 0))
        {
            return e->value.ptr;
        }
    }
    return NULL;
}

/*********************************************************************
 * 入口参数: hashmap指针, char * key
 * 出口参数: 无
 * 返回值  : int 返回值XIP_HASHMAP_EXIST_TURE(1), XIP_HASHMAP_EXIST_FALSE(0)
 * 函数功能: 根据key值从hashmap中查找是否存在,存在返回真,不存在返回假
 *********************************************************************/
int XipHashmapExists( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

    TxipHashmapNode * e = NULL;

    for ( e = map->table[idx]; e != NULL ; e = e->next)
    {
        /*参考java的hashmap实现,先比较hash,再比较strcmp*/
        if ( hcode == e->hash && (key == e->key || strcmp( key, e->key) == 0))
        {
            return 1;
        }
    }

    return 0;
}

/*********************************************************************
 * 入口参数: hashmap指针, char * key
 * 出口参数: 无
 * 返回值  : void * value
 * 函数功能: 根据key值从hashmap中删除key对应的node节点,如果删除成功，
 *          则返回删除节点的value的地址,未找到节点则返回NULL
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
        /*参考java的hashmap实现,先比较hash,再比较strcmp*/
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

    /*初始化*/
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

    /*如果达到最大了,则不在rebuild*/
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
        /*遍历*/
        e = *(map->table + i);
        if( e != NULL)
        {
            do
            {
                next = e->next;

                idx = e->hash % length;

                e->next = newtable[idx]; /*如果newtable[idx]的值已经存在了,该行则会将该值变为newtable[idx]->idx*/

                newtable[idx] = e;

                e = next;
            }while( e != NULL);
        }
    }

    /*释放oldtable*/
    HMAPFREE(map->table);
    map->table = newtable;
    map->length = length;
    map->threshold = (unsigned int)(map->factor * length);

    return 0;

}

/********************************************************
 * hash算法,此处使用的是 simple BKDR hash algorithm
 * 为JAVA的JDK中使用的算法
 * 也可以使用PHP等使用的time33算法(DJP hash algorithm）
 * 还可以使用暴雪公司的的One-Way-Hash,号称最快的hash算法
 * 
 * 之所以没有使用我推荐的算法,是因为我把机会留给了你.如果
 * 你看到了这个地方,并且试图优化算法的话,可以使用我建议的
 * 算法进行尝试.
 * 不过我想等到系统瓶颈追溯到本代码的时候,或许早就有了更
 * 完美的算法来保证趋近一致散列了.o(∩_∩)o
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
