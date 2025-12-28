//
// Created by 32874 on 2025/12/28.
//

#ifndef OS_DESIGN_MEMORY_H
#define OS_DESIGN_MEMORY_H


#include <stdio.h>
#include <stdlib.h>

// 定义空闲分区节点
typedef struct FreeNode {
    int start_addr;         // 起始地址
    int size;               // 分区大小
    struct FreeNode *next;  // 指向下一个空闲区
} FreeNode;

// 定义已分配分区节点（记录某进程的某个段）
typedef struct AllocatedNode {
    int pid;                // 进程ID
    int seg_id;             // 段号
    int start_addr;         // 起始地址
    int size;               // 段大小
    struct AllocatedNode *next;
} AllocatedNode;

// 全局变量声明（在memory.c中定义）
extern FreeNode *free_list;       // 空闲分区链表
extern AllocatedNode *alloc_list; // 已分配分区链表
extern int total_memory;          // 总内存大小

// 函数声明

// 1. 初始化内存
void initMemory(int size);

// 2. 内存分配（最坏适应算法）
// 返回 1 表示成功，0 表示失败
int requestMemory(int pid, int seg_count, int *seg_sizes);

// 3. 内存回收（包含合并逻辑）
void releaseMemory(int pid);

// 4. 显示当前内存状态
void showStatus();

// 5. 辅助函数：释放所有链表内存（程序退出用）
void clearSystem();





#endif //OS_DESIGN_MEMORY_H
