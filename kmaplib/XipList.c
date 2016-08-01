/**************************************************
 * �������� : List.c
 * �������� : icesky
 * ����汾 : 1.0.0
 * �������� : 2016.07.29
 * ������ : 
 *          �ο�java��Listʵ�ֻ���,ʵ����C������
 *          listʵ�֣�֧�ּ��ֹ���:
 *          1.�½�
 *          2.����
 *          3:.����
 *          4.��ȡ
 *          5.��ӡ
 * ע������ :
 *          
 * �޸ļ�¼ :
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
#define MALLOC_FLAG_NO   1      /*�������ڴ�*/

/*list �ṹ*/
typedef  struct
{
    unsigned int length;    /*ʵ�ʴ�С*/
    unsigned int threshold; /*�ٽ�ֵ*/
    int malloc_flag;        /*�������ڴ�*/
    void  ** ele_table;  /*element����*/

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

/*** static ���� ***/
/*�ؽ�ele_table��*/
static int rebuild_list( TxipList * lst);

/********************************************************************
 * ��������: XipListPrint
 * ��������: ��ӡList�����е�Ԫ�أ�ֻ����ʾ���ַ�����ֵ
 * ��������: icesky
 * ��������: 2016.07.29
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
 *********************************************************************/
int XipListPrint( void * list, void (*ref_func)(void *))
{
    TxipList * lst = ( TxipList *)list;

    unsigned int idx = 0;

    /** ��ӡ��־ **/
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
 * ��������: XipListNew
 * ��������: �����е�Ĭ�ϲ���������һ��list
 * ��������: icesky
 * ��������: 2016.07.29
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
 *********************************************************************/
void * XipListNew()
{
    return XipListInit(0);
}

/********************************************************************
 * ��������: XipListInit
 * ��������: ָ��value�洢��ʽ������һ��list
 * ��������: icesky
 * ��������: 2016.07.29
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
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
 * ��������: XipListDestory
 * ��������: �ͷ�list���ڴ���Դ
 * ��������: icesky
 * ��������: 2016.07.29
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
 *********************************************************************/
int XipListDestory( void * list)
{
    TxipList * lst = (TxipList *) list;
    register unsigned int idx;

    /** �ͷ�list element �ڴ� **/
    if( lst != NULL)
    {
        if( lst->malloc_flag !=  MALLOC_FLAG_NO)  /*�������ڴ�*/
        {
            for ( idx = 0; idx < lst->length; idx++)
            {
                /* �ͷ�element �ռ�*/
                LISTFREE(lst->ele_table[idx]);
                lst->ele_table[idx] = NULL;

            }
        }

        /* �ͷ�ָ��element table�Ŀռ� */
        LISTFREE(lst->ele_table);
        lst->ele_table = NULL;

        /** �ͷ�List�ռ� **/
        LISTFREE( lst);
        lst = NULL;
    }


    return 0;
}

/********************************************************************
 * ��������: XipListAdd
 * ��������: ��element����list��
 * ��������: icesky
 * ��������: 2016.07.29
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
 *********************************************************************/
void * XipListAdd( void * list, void * element, unsigned int size)
{
    TxipList * lst = (TxipList *)list;
    void * e = NULL;

    if( lst == NULL)
    {
        LISTLOG("E","list�ڴ�δ��ʼ��,�뽨���ô�������!!");
        return NULL;
    }

    e = lst->ele_table[lst->length];
    if( e != NULL)
    {
        LISTLOG("E","Ҫ�����ָ�벻Ϊ��,�����б�״̬![%x],����[%d],�ٽ�ֵ[%d]", e, lst->length, lst->threshold);
        return NULL;
    }
    
    if( element == NULL)
    {
        LISTLOG("E","Ҫ�����element����Ϊ��!!!");
        return NULL;
    }

    if( lst->malloc_flag ==  MALLOC_FLAG_NO)  /*�������ڴ�*/
    {
        lst->ele_table[lst->length] = element;
    }
    else
    {
        /*�½�element->value*/
        e = (void *)list_malloc(size);
        if( e == NULL)
        {
            LISTLOG("E","����value�ڴ�ʧ��!!!");
            return NULL;
        }
        memset( e, 0x00, size);
        memcpy( e, element, size);
        lst->ele_table[lst->length] = e;
    }


    lst->length++; /*���ӵ�ǰ����*/

    /*��������ٽ�ֵ�����ؽ�ele_table*/
    if( lst->length >= lst->threshold)
    {
        if( rebuild_list(lst) != 0)
        {
            LISTLOG("E","�ؽ�list����!!!");
            return NULL; 
        }
    }

    return lst->ele_table[lst->length];
}

/********************************************************************
 * ��������: XipListGet
 * ��������: ����˳���idx,��ȡList�е�ֵ
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
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
 * ��������: XipListLen
 * ��������: �鿴��ǰlist�Ĵ�С
 * ��������: icesky
 * ��������: 2016.08.01
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
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
 * ��������: XipListThreshold
 * ��������: �鿴�ĵ�ǰ�ٽ�ֵ
 * ��������: icesky
 * ��������: 2016.07.28
 * ����˵��: �ⲿ���ú������μ�xiplist.h������
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
 * ��������: rebuild_list
 * ��������: ��ǰlist�е�length�ﵽ���ٽ�ֵ��ʱ����Ҫ���´��������listtable
            �����д洢��
 * ��������: icesky
 * ��������: 2016.07.29
 * ����˵��: �ڲ����ú���. ע���ؽ�ʧ����Ҫ�жϷ���ֵ
 *********************************************************************/
static int rebuild_list( TxipList * lst)
{
    register unsigned int i = 0;
    void * * newtable=NULL;

    /*����ﵽ�����,����rebuild*/
    if( lst->threshold == XIP_LIST_MAX_THRESHOLD)
    {
        LISTLOG("E","�Ѵﵽlist���ֵ[%d]", lst->threshold);
        return -5;
    }

    /*����*/
    unsigned int threshold =  lst->threshold * 2;
    if( threshold > XIP_LIST_MAX_THRESHOLD)
        threshold = XIP_LIST_MAX_THRESHOLD;

    /*�����µ�ele_table*/
    newtable = (void * *)list_malloc(sizeof(void *) * threshold);

    /*��ֵ��ת��list->ele_table*/
    for( i = 0; i < lst->length; i++)
    {
        /*����*/
        newtable[i] = lst->ele_table[i];
    }

    /*�ͷ�oldtable*/
    LISTFREE(lst->ele_table);

    lst->ele_table = newtable;
    lst->threshold = threshold;

    return 0;
}

