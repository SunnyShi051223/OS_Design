#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// 分配算法类型枚举
typedef enum {
    ALG_FIRST_FIT = 1, // 首次适应
    ALG_BEST_FIT  = 2, // 最佳适应
    ALG_WORST_FIT = 3  // 最坏适应
} AllocAlgorithm;

// 1. 空闲分区节点 (双向链表，地址递增)
typedef struct FreeNode {
    int start_addr;
    int size;
    struct FreeNode *prev;
    struct FreeNode *next;
} FreeNode;

// 2. 已分配分区节点 (单向链表，用于记录段的资源)
typedef struct AllocatedNode {
    int pid;   //进程ID
    int seg_id;  //段号
    int start_addr;  //起始物理地址
    int size;   //段长
    struct AllocatedNode *next;
} AllocatedNode;

// 全局变量
extern FreeNode *free_list;
extern AllocatedNode *alloc_list;  //段表


// 初始化
void initMemory(int size);

// 请求内存 (支持算法选择 + 自动淘汰)
bool requestMemory(int pid, int seg_count, int *seg_sizes, AllocAlgorithm algo);

// 释放内存 (双向链表合并)
void releaseMemory(int pid);

// 淘汰函数 (当空间不足时调用)
bool runElimination(int current_pid);

// 显示状态
void showStatus();

// 清理系统
void clearSystem();

#endif