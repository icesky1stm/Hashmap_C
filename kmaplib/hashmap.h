#ifndef __XIPHASHMAP__
#define __XIPHASHMAP__

#define XIP_HASHMAP_EXIST_TURE  1       /*存在- 真*/
#define XIP_HASHMAP_EXIST_FALSE 0       /*不存在- 假*/
#define XIP_HASHMAP_PUT_ERR (void *)1 /*put异常*/ 

/*********************************************************************
 * 入口参数: hashmap的无类型指针
 * 出口参数: 无
 * 返回值  : int 永远返回0，可以不判断
 * 函数功能: 打印hashmap中所有的元素,主要用于调试和检查散列分布
 *
 *********************************************************************/
int XipHashmapPrint( void * hashmap);

/*********************************************************************
 * 入口参数: 无
 * 出口参数: 无
 * 返回值  : 创建的hashmap的指针,强制转换成了void *类型,方便调用
 * 函数功能: 以默认方式创建一个hashmap,默认的初始容量是16,加载因子是0.75f
 *
 *********************************************************************/
void * XipHashmapNew();

/*********************************************************************
 * 入口参数: int opacity 初始容量,  float factor 加载因子
 * 出口参数: 无
 * 返回值  : 创建的hashmap的指针,强制转换成了void *类型,方便调用
 * 函数功能: 按照传入的信息进行初始hashmap创建,
 *          如果opacity为0,则为默认值16,如果factor为0,则默认为0.75f
 *
 *********************************************************************/
void * XipHashmapInit( int opacity , float factor);

/*********************************************************************
 * 入口参数: hashmap指针
 * 出口参数: 无
 * 返回值  : int , 永远返回0可以不判断
 * 函数功能: 按照传入的信息进行初始hashmap创建,
 *          如果opacity为0,则为默认值16,如果factor为0,则默认为0.75f
 *
 *********************************************************************/
int XipHashmapDestory( void * in_map);

/*********************************************************************
 * 入口参数: hashmap指针, char * key, void * value
 * 出口参数: 无
 * 返回值  : 返回 void * oldvalue 的指针,如果为新增则返回NULL
 *          如果put失败则返回XIP_HASHMAP_PUT_ERR
 * 函数功能: 根据key和value放入到hashmap中,如果map中已经存在该key的值
 *          则替换成最新的value,同时返回旧value指针
 *          如果put之后，达到了临界值,则重新创建hashmap
 *********************************************************************/
void * XipHashmapPut( void * in_map, char * key, void * value);

/*********************************************************************
 * 入口参数: hashmap指针, char * key
 * 出口参数: 无
 * 返回值  : 返回 void * value 的指针
 * 函数功能: 根据key值从hashmap中取得value的指针返回
 *********************************************************************/
void * XipHashmapGet( void * TxipHashmap, char * key);

/*********************************************************************
 * 入口参数: hashmap指针, char * key
 * 出口参数: 无
 * 返回值  : int 返回值XIP_HASHMAP_EXIST_TURE(1), XIP_HASHMAP_EXIST_FALSE(0)
 * 函数功能: 根据key值从hashmap中查找是否存在,存在返回真,不存在返回假
 *********************************************************************/
int XipHashmapExists( void * TxipHashmap, char * key);

/*********************************************************************
 * 入口参数: hashmap指针, char * key
 * 出口参数: 无
 * 返回值  : void * value
 * 函数功能: 根据key值从hashmap中删除key对应的node节点,如果删除成功，
 *          则返回删除节点的value的地址,未找到节点则返回NULL
 *********************************************************************/
void * XipHashmapRemove( void * TxipHashmap, char * key);

#endif
