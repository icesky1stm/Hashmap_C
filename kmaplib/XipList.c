/**************************************************
 * 程序名称 : List.c
 * 程序作者 : icesky
 * 程序版本 : 1.0.0
 * 创建日期 : 2016.07.29
 * 程序功能 : 
 *          参考java的List实现机制,实现了C基本的
 *          list实现，支持几种功能:
 *          1.新建
 *          2.销毁
 *          3:.加入
 *          4.获取
 *          5.打印
 * 注意事项 :
 *          
 * 修改记录 :
 *************************************************/
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <stdarg.h>
#include "xiplist.h"

#define list_malloc(size) malloc(size)
#define LISTFREE(x)  free(x);x=NULL;
#define XIP_LIST_DEFAULT_THRESHOLD  1<<4
#define XIP_LIST_MAX_THRESHOLD  1<<30
#define MALLOC_FLAG_NO   1      /*不分配内存*/

/*list 结构*/
typedef  struct
{
    unsigned int length;    /*实际大小*/
    unsigned int threshold; /*临界值*/
    int malloc_flag;        /*不分配内存*/
    void  ** ele_table;  /*element数组*/

} TxipList;

#define LISTLOG(...) AppLog(__FILE__,__LINE__,__VA_ARGS__)
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
/*重建ele_table表*/
static int rebuild_list( TxipList * lst);

/********************************************************************
 * 函数名称: XipListPrint
 * 函数功能: 打印List中所有的元素，只能显示出字符串的值
 * 函数作者: icesky
 * 创建日期: 2016.07.29
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
int XipListPrint( void * list, void (*ref_func)(void *))
{
    TxipList * lst = ( TxipList *)list;

    unsigned int idx = 0;

    /** 打印日志 **/
    if( lst != NULL)
    {
        for ( idx = 0; idx < lst->length; idx++)
        {
            if( ref_func == NULL)
            {
                LISTLOG("I", "Length[%d], Element[%d]:[%x][%s]", 
                        lst->length, idx, lst->ele_table[idx], (char * )lst->ele_table[idx]);
            }
            else
            {
                ref_func(lst->ele_table[idx]);
            }
        }
    }

    return 0;
}

 /********************************************************************
 * 函数名称: XipListNew
 * 函数功能: 以所有的默认参数，创建一个list
 * 函数作者: icesky
 * 创建日期: 2016.07.29
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
void * XipListNew()
{
    return XipListInit(0);
}

/********************************************************************
 * 函数名称: XipListInit
 * 函数功能: 指定value存储方式，创建一个list
 * 函数作者: icesky
 * 创建日期: 2016.07.29
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
void * XipListInit(int malloc_flag)
{
    TxipList * lst = ( TxipList *)list_malloc(sizeof( TxipList));
    memset( lst, 0x00, sizeof( TxipList));

    lst->length = 0;
    lst->threshold = XIP_LIST_DEFAULT_THRESHOLD;
    lst->ele_table = (void * *)list_malloc(sizeof(void *) * lst->threshold);
    lst->malloc_flag = malloc_flag;
    memset( lst->ele_table, 0x00, sizeof(void *)* lst->threshold);

    return (void *)lst;
}

 /********************************************************************
 * 函数名称: XipListDestory
 * 函数功能: 释放list的内存资源
 * 函数作者: icesky
 * 创建日期: 2016.07.29
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
int XipListDestory( void * list)
{
    TxipList * lst = (TxipList *) list;
    register unsigned int idx;

    /** 释放list element 内存 **/
    if( lst != NULL)
    {
        if( lst->malloc_flag !=  MALLOC_FLAG_NO)  /*不分配内存*/
        {
            for ( idx = 0; idx < lst->length; idx++)
            {
                /* 释放element 空间*/
                LISTFREE(lst->ele_table[idx]);
                lst->ele_table[idx] = NULL;

            }
        }

        /* 释放指向element table的空间 */
        LISTFREE(lst->ele_table);
        lst->ele_table = NULL;

        /** 释放List空间 **/
        LISTFREE( lst);
        lst = NULL;
    }


    return 0;
}

/********************************************************************
 * 函数名称: XipListAdd
 * 函数功能: 将element放入list中
 * 函数作者: icesky
 * 创建日期: 2016.07.29
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
void * XipListAdd( void * list, void * element, unsigned int size)
{
    TxipList * lst = (TxipList *)list;
    void * e = NULL;

    if( lst == NULL)
    {
        LISTLOG("E","list内存未初始化,请建调用创建函数!!");
        return NULL;
    }

    e = lst->ele_table[lst->length];
    if( e != NULL)
    {
        LISTLOG("E","要放入的指针不为空,请检查列表状态![%x],长度[%d],临界值[%d]", e, lst->length, lst->threshold);
        return NULL;
    }
    
    if( element == NULL)
    {
        LISTLOG("E","要放入的element不能为空!!!");
        return NULL;
    }

    if( lst->malloc_flag ==  MALLOC_FLAG_NO)  /*不分配内存*/
    {
        lst->ele_table[lst->length] = element;
    }
    else
    {
        /*新建element->value*/
        e = (void *)list_malloc(size);
        if( e == NULL)
        {
            LISTLOG("E","分配value内存失败!!!");
            return NULL;
        }
        memset( e, 0x00, size);
        memcpy( e, element, size);
        lst->ele_table[lst->length] = e;
    }


    lst->length++; /*增加当前长度*/

    /*如果触发临界值，则重建ele_table*/
    if( lst->length >= lst->threshold)
    {
        if( rebuild_list(lst) != 0)
        {
            LISTLOG("E","重建list错误!!!");
            return NULL; 
        }
    }

    return lst->ele_table[lst->length];
}

/********************************************************************
 * 函数名称: XipListGet
 * 函数功能: 根据顺序号idx,获取List中的值
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
void * XipListGet( void * list, int idx)
{
    TxipList * lst = (TxipList *) list;

    if( lst != NULL)
    {
        if( idx <= lst->length)
        {
            return lst->ele_table[idx];
        }
    }

    return NULL;
}
/********************************************************************
 * 函数名称: XipListLen
 * 函数功能: 查看当前list的大小
 * 函数作者: icesky
 * 创建日期: 2016.08.01
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
int XipListLen( void * list)
{
    TxipList * lst = (TxipList *) list;
    if( lst != NULL)
    {
        return lst->length;
    }

    return 0;
}
/********************************************************************
 * 函数名称: XipListThreshold
 * 函数功能: 查看的当前临界值
 * 函数作者: icesky
 * 创建日期: 2016.07.28
 * 调用说明: 外部调用函数，参见xiplist.h中描述
 *********************************************************************/
int XipListThreshold( void * list)
{
    TxipList * lst = (TxipList *) list;

    if( lst != NULL)
    {
        return lst->threshold;
    }
    return 0;
}
/********************************************************************
 * 函数名称: rebuild_list
 * 函数功能: 当前list中的length达到了临界值的时候，需要重新创建更大的listtable
            来进行存储。
 * 函数作者: icesky
 * 创建日期: 2016.07.29
 * 调用说明: 内部调用函数. 注意重建失败了要判断返回值
 *********************************************************************/
static int rebuild_list( TxipList * lst)
{
    register unsigned int i = 0;
    void * * newtable=NULL;

    /*如果达到最大了,则不在rebuild*/
    if( lst->threshold == XIP_LIST_MAX_THRESHOLD)
    {
        LISTLOG("E","已达到list最大值[%d]", lst->threshold);
        return -5;
    }

    /*扩容*/
    unsigned int threshold =  lst->threshold * 2;
    if( threshold > XIP_LIST_MAX_THRESHOLD)
        threshold = XIP_LIST_MAX_THRESHOLD;

    /*创建新的ele_table*/
    newtable = (void * *)list_malloc(sizeof(void *) * threshold);

    /*赋值和转移list->ele_table*/
    for( i = 0; i < lst->length; i++)
    {
        /*遍历*/
        newtable[i] = lst->ele_table[i];
    }

    /*释放oldtable*/
    LISTFREE(lst->ele_table);

    lst->ele_table = newtable;
    lst->threshold = threshold;

    return 0;
}

