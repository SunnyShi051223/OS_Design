#include "memory.h"
#include <limits.h>

// 全局变量
FreeNode *free_list = NULL;
AllocatedNode *alloc_list = NULL;

// 初始化
void initMemory(int size) {
    if (free_list != NULL) clearSystem();

    free_list = (FreeNode *)malloc(sizeof(FreeNode));
    free_list->start_addr = 0;
    free_list->size = size;
    free_list->prev = NULL;
    free_list->next = NULL;
    alloc_list = NULL;
    printf(">> 系统初始化完成，总内存: %d KB\n", size);
}

// 添加分配记录 (头插法，意味着链表尾部是最早分配的进程)
void addAllocNode(int pid, int seg_id, int start, int size) {
    AllocatedNode *newNode = (AllocatedNode *)malloc(sizeof(AllocatedNode));
    newNode->pid = pid;
    newNode->seg_id = seg_id;
    newNode->start_addr = start;
    newNode->size = size;
    newNode->next = alloc_list;
    alloc_list = newNode;
}

// 从双向链表移除空闲节点
void removeFreeNode(FreeNode *node) {
    if (node == NULL) return;
    if (node->prev != NULL) node->prev->next = node->next;
    else free_list = node->next;

    if (node->next != NULL) node->next->prev = node->prev;
    free(node);
}

// 归还内存并执行双向合并
void returnToFreeList(int start, int size) {
    FreeNode *newNode = (FreeNode *)malloc(sizeof(FreeNode));
    newNode->start_addr = start;
    newNode->size = size;
    newNode->prev = NULL;
    newNode->next = NULL;

    // 1. 有序插入
    if (free_list == NULL) {
        free_list = newNode;
    } else if (free_list->start_addr > start) {
        newNode->next = free_list;
        free_list->prev = newNode;
        free_list = newNode;
    } else {
        FreeNode *curr = free_list;
        while (curr->next != NULL && curr->next->start_addr < start) {
            curr = curr->next;
        }
        newNode->next = curr->next;
        newNode->prev = curr;
        if (curr->next != NULL) curr->next->prev = newNode;
        curr->next = newNode;
    }

    // 2. 向后合并 (Merge Next)
    if (newNode->next != NULL &&
        newNode->start_addr + newNode->size == newNode->next->start_addr) {
        FreeNode *next = newNode->next;
        newNode->size += next->size;
        newNode->next = next->next;
        if (next->next != NULL) next->next->prev = newNode;
        free(next);
    }

    // 3. 向前合并 (Merge Prev)
    if (newNode->prev != NULL &&
        newNode->prev->start_addr + newNode->prev->size == newNode->start_addr) {
        FreeNode *prev = newNode->prev;
        prev->size += newNode->size;
        prev->next = newNode->next;
        if (newNode->next != NULL) newNode->next->prev = prev;
        free(newNode);
    }
}

// 释放指定进程的内存
void releaseMemory(int pid) {
    AllocatedNode *curr = alloc_list;
    AllocatedNode *prev = NULL;
    int count = 0;

    while (curr != NULL) {
        if (curr->pid == pid) {
            count++;
            // 归还并合并
            returnToFreeList(curr->start_addr, curr->size);

            AllocatedNode *temp = curr;
            if (prev == NULL) alloc_list = curr->next;
            else prev->next = curr->next;
            curr = curr->next;
            free(temp);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    if (count > 0) printf(">> [系统消息] 进程 %d 已释放 %d 个段，内存已合并。\n", pid, count);
}

// --- 淘汰函数 (FIFO) ---
// 返回 true 表示成功淘汰了一个进程，false 表示无进程可淘汰
bool runElimination(int current_pid) {
    if (alloc_list == NULL) return false;

    // 策略：寻找 alloc_list 的尾部节点。
    // 因为 addAllocNode 使用的是头插法，所以链表尾部的节点是最早分配的 (FIFO)。
    AllocatedNode *curr = alloc_list;
    AllocatedNode *victim = NULL;

    // 遍历找到最后一个不属于当前进程的节点
    while (curr != NULL) {
        if (curr->pid != current_pid) {
            victim = curr; // 记录候选人
        }
        curr = curr->next;
    }

    if (victim == NULL) {
        return false; // 没有其他进程可淘汰
    }

    int victim_pid = victim->pid;
    printf(">> [警告] 内存不足！触发淘汰机制：正在移除最旧进程 (PID: %d)...\n", victim_pid);
    releaseMemory(victim_pid); // 释放并合并
    return true;
}

// --- 内存请求 ---
bool requestMemory(int pid, int seg_count, int *seg_sizes, AllocAlgorithm algo) {
    if (seg_count <= 0) return false;

    for (int i = 0; i < seg_count; i++) {
        int req_size = seg_sizes[i];
        bool allocated = false;

        // 循环尝试分配：如果失败，则淘汰一个进程再试，直到成功或无法淘汰
        while (!allocated) {
            FreeNode *target_node = NULL;
            FreeNode *curr = free_list;

            // --- 1. 执行分配算法 ---
            if (algo == ALG_FIRST_FIT) {
                // [首次适应]
                while (curr != NULL) {
                    if (curr->size >= req_size) {
                        target_node = curr;
                        break;
                    }
                    curr = curr->next;
                }
            }
            else if (algo == ALG_BEST_FIT) {
                // [最佳适应]
                int min_diff = INT_MAX;
                while (curr != NULL) {
                    if (curr->size >= req_size) {
                        int diff = curr->size - req_size;
                        if (diff < min_diff) {
                            min_diff = diff;
                            target_node = curr;
                            if (diff == 0) break;
                        }
                    }
                    curr = curr->next;
                }
            }
            else if (algo == ALG_WORST_FIT) {
                // [最坏适应]
                int max_size = -1;
                while (curr != NULL) {
                    if (curr->size >= req_size && curr->size > max_size) {
                        max_size = curr->size;
                        target_node = curr;
                    }
                    curr = curr->next;
                }
            }

            // --- 2. 判断是否成功 ---
            if (target_node != NULL) {
                // 分配成功
                addAllocNode(pid, i, target_node->start_addr, req_size);
                target_node->start_addr += req_size;
                target_node->size -= req_size;
                if (target_node->size == 0) removeFreeNode(target_node);
                allocated = true;
            } else {
                // --- 3. 分配失败，调用淘汰函数 ---
                // 尝试淘汰一个进程来腾出空间
                if (!runElimination(pid)) {
                    printf("!! 致命错误：内存严重不足，且无可淘汰进程。进程 %d 分配失败。\n", pid);
                    // 回滚本次已分配的段
                    if (i > 0) {
                        printf(">> 回滚：释放进程 %d 已分配的资源...\n", pid);
                        releaseMemory(pid);
                    }
                    return false;
                }
                // 淘汰成功后，while循环继续，重新尝试分配当前段
            }
        }
    }
    return true;
}

// 显示状态
void showStatus() {
    printf("\n|----------- 当前内存状态 -----------|\n");
    printf("| [空闲表] (地址: 容量)              |\n");
    FreeNode *f = free_list;
    if(!f) printf("|  (无空闲)                          |\n");
    while (f) {
        printf("|  %5d -> %5d : %5d KB        |\n", f->start_addr, f->start_addr+f->size, f->size);
        f = f->next;
    }
    printf("| [已分配]                           |\n");
    AllocatedNode *a = alloc_list;
    if(!a) printf("|  (无)                              |\n");
    while (a) {
        printf("|  PID:%3d (段%d) 地址:%3d  大小：%3d    |\n", a->pid, a->seg_id, a->start_addr,a->size);
        a = a->next;
    }
    printf("|------------------------------------|\n\n");
}

// 清理资源
void clearSystem() {
    while(free_list) { FreeNode *t=free_list; free_list=free_list->next; free(t); }
    while(alloc_list) { AllocatedNode *t=alloc_list; alloc_list=alloc_list->next; free(t); }
}