//
// Created by 32874 on 2025/12/28.
//

#include "memory.h"

// 全局变量定义
FreeNode *free_list = NULL;
AllocatedNode *alloc_list = NULL;
int total_memory = 0;

// 初始化内存：建立一个包含全部内存的空闲块
void initMemory(int size) {
    total_memory = size;
    free_list = (FreeNode *)malloc(sizeof(FreeNode));
    free_list->start_addr = 0;
    free_list->size = size;
    free_list->next = NULL;
    alloc_list = NULL;
    printf("系统初始化完成，总内存: %d KB\n", size);
}

// 辅助函数：向已分配链表添加记录
void addAllocNode(int pid, int seg_id, int start, int size) {
    AllocatedNode *newNode = (AllocatedNode *)malloc(sizeof(AllocatedNode));
    newNode->pid = pid;
    newNode->seg_id = seg_id;
    newNode->start_addr = start;
    newNode->size = size;

    // 头插法简单插入（或者按PID排序也可，这里简单处理）
    newNode->next = alloc_list;
    alloc_list = newNode;
}

// 最坏适应算法实现 (Worst Fit)
// 策略：遍历整个空闲链表，找到容量最大的那个分区进行分配
int requestMemory(int pid, int seg_count, int *seg_sizes) {
    // 备份当前的空闲链表和已分配链表，以便分配失败时回滚
    // 注意：为简化模拟，本代码假设若分配失败需手动释放已分配部分，
    // 或者在分配前进行预检查。这里采用“预检查”策略。

    // 1. 预检查：模拟分配，看是否所有段都能满足
    // 为了不破坏真实链表，我们需要更复杂的逻辑。
    // 这里采用简单策略：尝试分配，如果中间失败，则通过 releaseMemory(pid) 回滚。

    int i;
    for (i = 0; i < seg_count; i++) {
        int req_size = seg_sizes[i];

        FreeNode *worst_fit = NULL;
        FreeNode *p = free_list;
        int max_size = -1;

        // --- 核心算法：寻找最大的空闲分区 (Worst Fit) ---
        while (p != NULL) {
            if (p->size >= req_size && p->size > max_size) {
                max_size = p->size;
                worst_fit = p;
            }
            p = p->next;
        }

        // 如果找不到合适的块
        if (worst_fit == NULL) {
            printf("内存不足！进程 %d 的第 %d 段 (大小 %d) 分配失败。\n", pid, i, req_size);
            // 回滚：释放该进程刚才已分配的段
            if (i > 0) {
                printf("正在回滚已分配的资源...\n");
                releaseMemory(pid);
            }
            return 0; // 失败
        }

        // --- 执行分配 ---
        // 1. 记录分配信息
        addAllocNode(pid, i, worst_fit->start_addr, req_size);

        // 2. 修改空闲块（切分）
        worst_fit->start_addr += req_size;
        worst_fit->size -= req_size;

        // 3. 如果该空闲块大小变为0，需要从链表中删除它
        // 注意：由于worst_fit是指针，我们需要重新遍历找到它的前驱来删除，
        // 或者简单地在下一次整理时处理。为了严谨，这里做清理：
        if (worst_fit->size == 0) {
            if (worst_fit == free_list) {
                free_list = free_list->next;
                free(worst_fit);
            } else {
                FreeNode *prev = free_list;
                while (prev->next != worst_fit) prev = prev->next;
                prev->next = worst_fit->next;
                free(worst_fit);
            }
        }
    }

    return 1; // 成功
}

// 辅助函数：将释放的内存块按地址顺序插入空闲链表，并合并
void returnToFreeList(int start, int size) {
    FreeNode *newNode = (FreeNode *)malloc(sizeof(FreeNode));
    newNode->start_addr = start;
    newNode->size = size;
    newNode->next = NULL;

    // 1. 插入位置查找（按地址递增有序排列，方便合并）
    if (free_list == NULL || free_list->start_addr > start) {
        newNode->next = free_list;
        free_list = newNode;
    } else {
        FreeNode *curr = free_list;
        while (curr->next != NULL && curr->next->start_addr < start) {
            curr = curr->next;
        }
        newNode->next = curr->next;
        curr->next = newNode;
    }

    // 2. --- 核心算法：合并空闲区 (Coalescing) ---
    // 遍历链表，检查 curr.end == next.start
    FreeNode *curr = free_list;
    while (curr != NULL && curr->next != NULL) {
        // 判断当前块的尾部是否紧挨着下一个块的头部
        if (curr->start_addr + curr->size == curr->next->start_addr) {
            // 合并
            FreeNode *temp = curr->next;
            curr->size += temp->size; // 大小累加
            curr->next = temp->next;  // 链表指针跳过
            free(temp); // 释放节点内存
            // 注意：合并后不要急着curr=curr->next，因为可能还要和新的后继合并
        } else {
            curr = curr->next;
        }
    }
}

// 内存回收逻辑
void releaseMemory(int pid) {
    AllocatedNode *curr = alloc_list;
    AllocatedNode *prev = NULL;
    int found = 0;

    // 遍历已分配链表，找到所有属于 PID 的段
    while (curr != NULL) {
        if (curr->pid == pid) {
            found = 1;
            // 1. 将空间归还给空闲链表（包含合并逻辑）
            returnToFreeList(curr->start_addr, curr->size);

            // 2. 从已分配链表中删除该节点
            AllocatedNode *toDelete = curr;
            if (prev == NULL) {
                alloc_list = curr->next;
                curr = alloc_list;
            } else {
                prev->next = curr->next;
                curr = prev->next;
            }
            free(toDelete);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }

    if (found) {
        printf("进程 %d 的资源已全部回收并合并。\n", pid);
    } else {
        printf("未找到进程 %d 的相关资源。\n", pid);
    }
}

// 显示状态
void showStatus() {
    printf("\n----------- 当前内存状态 ------------\n");
    printf("[空闲分区表] (按地址排序)\n");
    FreeNode *f = free_list;
    if (!f) printf("  无空闲区 (内存已满)\n");
    while (f) {
        printf("  地址: %5d | 大小: %5d | 状态: 空闲\n", f->start_addr, f->size);
        f = f->next;
    }

    printf("[已分配分区表]\n");
    AllocatedNode *a = alloc_list;
    if (!a) printf("  无活动进程\n");
    while (a) {
        printf("  进程ID: %3d | 段号: %2d | 地址: %5d | 大小: %5d\n",
               a->pid, a->seg_id, a->start_addr, a->size);
        a = a->next;
    }
    printf("--------------------------------------\n\n");
}

void clearSystem() {
    // 清理内存代码略（实际运行时操作系统会回收，但在严谨代码中应遍历free/alloc链表逐个free）
}