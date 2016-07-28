#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hashmap.h>

int main()
{
    char *name[] = {
        "chenmanwen", "yangqinghua", "chenxin", "yanzi", "yangjian", "zhangrenfang",
        "panzi", "zhangqiang", "webssky", "jcseg", "friso", "robbe", "lionsoul",
        "tankwar", "jteach"
    };             

    void * hashmap = XipHashmapNew();

    int i = 0;
    int length = 15;
    for ( i = 0; i <  length ; i++)
    {
        printf("put(%15s, %15s);\n", name[i], name[i]);
        XipHashmapPut( hashmap, name[i], name[i]);
    }

    XipHashmapPrint(hashmap);
    XipHashmapPut( hashmap, "panzi", "iceskyiceskyicesky");
    XipHashmapPrint(hashmap);

    for ( i = 0; i< length; i++)
        printf("get(%15s): %15s\n", name[i], (char *)XipHashmapGet(hashmap, name[i]));

    XipHashmapPrint(hashmap);

    printf("remove(%15s): %15s\n", "lionsoul", (char *)XipHashmapRemove(hashmap, "lionsoul"));

    XipHashmapPrint(hashmap);

    XipHashmapDestory(hashmap);

    return 0;
}

