#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xiplist.h>

void name_printf( void * element);
int main()
{
    char *name[] = {
        "chenmanwen", "yangqinghua", "chenxin", "yanzi", "yangjian", "zhangrenfang",
        "panzi", "zhangqiang", "webssky", "jcseg", "friso", "robbe", "lionsoul",
        "tankwar", "jteach"
    };             

    char tmp_name[51];

    void * list = XipListNew();
    /*
    void * list = XipListInit(1);
    */
    int ret = 0;

    int i = 0;
    int length = 15;
    for ( i = 0; i <  length ; i++)
    {
        strcpy( tmp_name, name[i]);
        printf("Add(value:[%s]--[%s]);\n", name[i], XipListAdd( list, tmp_name,strlen(tmp_name)+1));
    }
    printf("Len--[%d],threshold[%d]\n", XipListLen(list), XipListThreshold(list));

    XipListPrint(list, name_printf);
    XipListPrint(list, NULL);

    for ( i = 0; i< length; i++)
        printf("get(%d): [%s], [%x]\n", i, (char *)XipListGet(list, i), XipListGet(list, i));

    XipListDestory(list);
    list = NULL;

    XipListPrint(list, name_printf);

    return 0;
}

void name_printf( void * element)
{
    printf("aaaaaaaaaaa---[%s]\n", element);
}


