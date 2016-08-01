#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xiphashmap.h>

static void name_print( int idx, char * key, void * value);

int main()
{
    char *name[] = {
        "chenmanwen", "yangqinghua", "chenxin", "yanzi", "yangjian", "zhangrenfang",
        "panzi", "zhangqiang", "webssky", "jcseg", "friso", "robbe", "lionsoul",
        "tankwar", "jteach"
    };             

    void * hashmap = XipHashmapNew();
    /*
    void * hashmap = XipHashmapInit(0, 0.00, 1);
    */

    int i = 0;
    int length = 15;
    char tmpname[51];
    for ( i = 0; i <  length ; i++)
    {
        strcpy( tmpname, name[i]);
        printf("put(%15s, %15s);\n", tmpname, tmpname);
        XipHashmapPut( hashmap, tmpname, tmpname, strlen(tmpname)+1);
    }

    XipHashmapPrint(hashmap, NULL);
    XipHashmapPut( hashmap, "panzi", "iceskyiceskyicesky", strlen("iceskyiceskyicesky")+1);
    XipHashmapPrint(hashmap, name_print);

    for ( i = 0; i< length; i++)
        printf("get(%15s): %15s\n", name[i], (char *)XipHashmapGet(hashmap, name[i]));

    XipHashmapPrint(hashmap, NULL);

    printf("remove(%15s): %d\n", "lionsoul", XipHashmapRemove(hashmap, "lionsoul"));

    XipHashmapPrint(hashmap, NULL);

    XipHashmapDestory(hashmap);

    return 0;
}

static void name_print( int idx, char * key, void * value)
{
    printf("aaaaaaaaaaaaa-idx[%d], key[%s], value[%s]\n", idx, key, value);
    return;
}
