#include <stdio.h>
#include "memory.h"

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    int totalSize;
    int choice;

    printf("=========================================\n");
    printf("   段式存储管理模拟系统 (最坏适应算法)   \n");
    printf("=========================================\n");

    // 1. 输入内存大小 [任务书要求]
    printf("请输入系统内存总大小 (单位KB): ");
    scanf("%d", &totalSize);
    initMemory(totalSize);

    while (1) {
        printf("\n功能菜单:\n");
        printf("1. 申请内存 (Process Request)\n");
        printf("2. 回收内存 (Process Release)\n");
        printf("3. 显示状态 (Show Status)\n");
        printf("0. 退出系统 (Exit)\n");
        printf("请选择: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                int pid, seg_count;
                printf("请输入进程ID: ");
                scanf("%d", &pid);
                printf("请输入该进程的段数: ");
                scanf("%d", &seg_count);

                // 动态分配数组来存每段大小
                int *sizes = (int *)malloc(seg_count * sizeof(int));
                for (int i = 0; i < seg_count; i++) {
                    printf("  请输入第 %d 段的大小: ", i);
                    scanf("%d", &sizes[i]);
                }

                // 调用分配核心
                if (requestMemory(pid, seg_count, sizes)) {
                    printf(">> 成功：进程 %d 分配完成。\n", pid);
                } else {
                    printf(">> 失败：进程 %d 无法满足。\n", pid);
                }

                free(sizes); // 释放临时数组
                showStatus();
                break;
            }
            case 2: {
                int pid;
                printf("请输入要回收的进程ID: ");
                scanf("%d", &pid);
                releaseMemory(pid);
                showStatus();
                break;
            }
            case 3:
                showStatus();
                break;
            case 0:
                clearSystem();
                printf("程序退出。\n");
                return 0;
            default:
                printf("输入无效，请重试。\n");
        }
    }
    return 0;
}