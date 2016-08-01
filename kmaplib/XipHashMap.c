/**************************************************
 * 程序名称 : XipHashMap.c
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
 * 注意事项: 
 *          
 * 修改记录 :
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
#define INTER_MAX_VALUE  1<<31  /*int的最大值*/
#define MALLOC_FLAG_NO   1      /*不分配内存*/


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
    int malloc_flag;          /*是否为value分配内存*/
    float factor;             /*加载因子,默认为0.75(0-1)*/
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

/*** static 声明 ***/
/*新建T表*/
static TxipHashmapNode ** create_T_table( int opacity);
/*新建hashmap节点*/
static TxipHashmapNode * create_hashmap_node( char * key, void * value, TxipHashmapNode * next, int malloc_flag, int size);
/*重建hashmap*/
static int rebuild_hash( TxipHashmap * hashmap);
/*hash算法*/
static unsigned int XipHash(char * key);

/********************************************************************
 * 函数名称: XipHashmapPrint
 * 函数功能: 打印hashmap中所有的元素,主要用于调试和检查散列分布
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
int XipHashmapPrint( void * hashmap, void (*ref_func)(int,char *, void *))
{
    TxipHashmap * map = ( TxipHashmap *)hashmap;

    int idx = 0;

    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;

    /** 打印日志 **/
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
    
    /** 打印分布情况图 **/
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
 * 函数名称: XipHashmapNew
 * 函数功能: 以所有的默认参数，创建一个hashmap
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
void * XipHashmapNew()
{
    return XipHashmapInit( 0, 0.00, 0); /*全默认选项*/
}

/********************************************************************
 * 函数名称: XipHashmapInit
 * 函数功能: 指定初始容量，加载因子和value存储方式，创建一个hashmap
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
void * XipHashmapInit( int opacity, float factor, int malloc_flag)
{
    TxipHashmap * map = ( TxipHashmap *)hmap_malloc(sizeof( TxipHashmap));
    if( map == NULL)
    {
        HMAPLOG("E","初始化hashmap内存失败!!!");
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
        HMAPLOG("E","初始容量必须大于1!!!");
        HMAPFREE(map);
        map = NULL;
        return NULL;
    }
    if( factor < 0.00 || factor > 1.00)
    {
        HMAPLOG("E","加载因子取值区间为[0.00-1.00],但本次加载因子为[%.2f]", factor);
        HMAPFREE(map);
        map = NULL;
        return NULL;
    }

    map->table = create_T_table( opacity);
    if( map->table == NULL)
    {
        HMAPLOG("E","创建hash值的T表失败!!!");
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
 * 函数名称: XipHashmapDestory
 * 函数功能: 释放hashmap的内存资源
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
int XipHashmapDestory( void * hashmap)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;

    TxipHashmapNode * e = NULL;
    TxipHashmapNode * next = NULL;
    register unsigned int idx;

    /** 释放hashmap node 内存 **/
    if( map != NULL)
    {
        /** 释放hashmap的node空间 **/
        for ( idx = 0; idx < map->length; idx++)
        {
            for( e = map->table[idx]; e != NULL; )
            {
                next = e->next;
                /* 释放node的value空间*/
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

        /** 释放hashmap的T表 **/
        HMAPFREE( map->table);
        map->table = NULL;
        
        /** 释放hashmap本身的空间 **/
        HMAPFREE( map);
        map = NULL;
    }


    return 0;
}

 /********************************************************************
 * 函数名称: XipHashmapDestory
 * 函数功能: 将key和value放入hashmap中，如果key已存在，相当于Set命令
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
void * XipHashmapPut( void * hashmap, char * key, void * value, int size)
{
    TxipHashmap * map = (TxipHashmap *)hashmap;

    if( map == NULL)
    {
        HMAPLOG("E","hashmap指针为空,请检查是否已经调用XipHashmapNew/XipHashmapInit,或调用返回异常");
        return NULL;
    }

    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Key值必须为字符串,且Key值不能为空或者NULL!!!");
        return NULL;
    }
    if ( value == NULL)
    {
        HMAPLOG("E","value不可以为空(NULL)");
        return NULL;
    }
    if ( size <= 0)
    {
        HMAPLOG("E","值的大小size不可以为0");
        return NULL;
    }

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
            if( map->malloc_flag == 1) /*不分配内存*/
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
                    HMAPLOG("E","为value分配内存时异常!!!");
                    return NULL;
                }
                memset( e->value.ptr, 0x00, size);
                memcpy( e->value.ptr, value, size);
                return e->value.ptr;
            }
            return oldvalue; /*将原value返回给调用者*/
        }
    }

    /*新建node*/
    map->table[idx] = create_hashmap_node( key, value, map->table[idx], map->malloc_flag, size);
    if( map->table[idx] == NULL)
    {
        HMAPLOG("E","新建hashnode节点异常malloc_flag[%d],size[%d]!!!", map->malloc_flag, size);
        return NULL; /*返回固定值*/
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
            return NULL; /*返回固定值*/
        }
    }

    return map->table[idx]->value.ptr; /*返回内存地址*/
}

 /********************************************************************
 * 函数名称: XipHashmapGet
 * 函数功能: 根据Key值获取对应的value值
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
void * XipHashmapGet( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    if( map == NULL)
    {
        HMAPLOG("E","hashmap指针为空,请检查是否已经调用XipHashmapNew/XipHashmapInit,或调用返回异常");
        return NULL;
    }

    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Key值必须为字符串,且Key值不能为空或者NULL!!!");
        return NULL;
    }

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

 /********************************************************************
 * 函数名称: XipHashmapExists
 * 函数功能: 判断Key是否在hashmap中存在
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
int XipHashmapExists( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    if( map == NULL)
    {
        HMAPLOG("E","hashmap指针为空,请检查是否已经调用XipHashmapNew/XipHashmapInit,或调用返回异常");
        return -5;
    }
    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Key值必须为字符串,且Key值不能为空或者NULL!!!");
        return -10;
    }

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

 /********************************************************************
 * 函数名称: XipHashmapDestory
 * 函数功能: 删除map中对应的Key和VALUE的节点
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiphashmap.h中描述
 *********************************************************************/
int  XipHashmapRemove( void * hashmap, char *key)
{
    TxipHashmap * map = (TxipHashmap *) hashmap;
    if( map == NULL)
    {
        HMAPLOG("E","hashmap指针为空,请检查是否已经调用XipHashmapNew/XipHashmapInit,或调用返回异常");
        return -5;
    }
    if ( key == NULL || strlen(key) == 0)
    {
        HMAPLOG("E","Key值必须为字符串,且Key值不能为空或者NULL!!!");
        return -10;
    }

    unsigned int hcode = XipHash(key); 
    unsigned int idx = hcode % map->length;

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

            if( map->malloc_flag == MALLOC_FLAG_NO) /*不分配内存*/
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
 * 函数名称: create_T_table
 * 函数功能: 申请指向node的连续内存指针，在《算法导论》关于散列表中，所处位置为T表
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 内部调用函数.注意返回值是指向TxipHashmapNode的指针的指针
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

    /*初始化*/
    for ( i = 0; i < opacity; i++)
        table[i] = NULL;

    return table;
}

/********************************************************************
 * 函数名称: create_hashmap_node
 * 函数功能: 创建具体的key,value的node节点
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 内部调用函数. 如果malloc_flag是1的话，则size无效，因为不需要
        为value申请内存。
 *********************************************************************/
static TxipHashmapNode * create_hashmap_node( char * key, void * value, TxipHashmapNode * next, int malloc_flag, int size)
{
    TxipHashmapNode * node = ( TxipHashmapNode *) hmap_malloc( sizeof(TxipHashmapNode));
    if ( node == NULL)
    {
        return NULL;
    }
    memset( node, 0x00, sizeof( TxipHashmapNode));

    if( malloc_flag ==  MALLOC_FLAG_NO) /*不分配内存*/
    {
        node->key = key;
        node->value.ptr = value;
    }
    else 
    {
        /*限定KEY必须是字符串类型*/
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
 * 函数名称: rebuild_hash
 * 函数功能: 当前hashmap中的size达到了临界值的时候，需要重新创建更大的hashmap
            来进行存储。
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 内部调用函数. 注意重建失败了要判断返回值
 *********************************************************************/
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
 * 函数名称: XipHash
 * 函数功能: 通过key来计算hash值的算法，算法越好，散列的一致分布性越好
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 内部调用函数.
 * hash算法,此处使用的是 simple BKDR hash algorithm
 * 为JAVA的JDK中使用的算法
 * 也可以使用PHP等使用的time33算法(DJP hash algorithm）
 * 还可以使用暴雪公司的的One-Way-Hash(号称最快的hash算法)
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
