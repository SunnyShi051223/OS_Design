/*
 * 文件名: main.c
 * 描述: 主程序，提供算法选择菜单
 */

#include <stdio.h>
#include "memory.h"

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    int totalSize, choice;

    printf("==================================================\n");
    printf("  段式存储管理 (支持淘汰机制 & 多种算法)          \n");
    printf("==================================================\n");

    printf("请输入总内存大小 (KB): ");
    while (scanf("%d", &totalSize) != 1 || totalSize <= 0) {
        printf("输入无效，请重输: ");
        clearInputBuffer();
    }
    initMemory(totalSize);

    while (1) {
        printf("\n功能菜单:\n");
        printf("1. 申请内存 (Request)\n");
        printf("2. 回收内存 (Release)\n");
        printf("3. 显示状态 (Status)\n");
        printf("0. 退出系统 (Exit)\n");
        printf("请选择: ");

        if (scanf("%d", &choice) != 1) {
            printf("输入无效。\n");
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1: {
                int pid, seg_count, algo_choice;
                printf("请输入 进程ID: "); scanf("%d", &pid);
                printf("请输入 段数: "); scanf("%d", &seg_count);

                if (seg_count <= 0) break;

                int *sizes = (int *)malloc(seg_count * sizeof(int));
                for (int i = 0; i < seg_count; i++) {
                    printf("  -> 第 %d 段大小: ", i);
                    scanf("%d", &sizes[i]);
                }

                // 算法选择
                printf("\n请选择分配算法:\n");
                printf("  1. 首次适应 (First Fit)\n");
                printf("  2. 最佳适应 (Best Fit)\n");
                printf("  3. 最坏适应 (Worst Fit)\n");
                printf("算法选择 [1-3]: ");
                scanf("%d", &algo_choice);

                AllocAlgorithm algo;
                if (algo_choice == 1) algo = ALG_FIRST_FIT;
                else if (algo_choice == 2) algo = ALG_BEST_FIT;
                else algo = ALG_WORST_FIT;

                // 调用申请 (如果失败会自动触发淘汰)
                if (requestMemory(pid, seg_count, sizes, algo)) {
                    printf(">> 成功：进程 %d 分配完成。\n", pid);
                } else {
                    printf(">> 失败：进程 %d 无法满足 (内存极度紧缺)。\n", pid);
                }

                free(sizes);
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
                return 0;
            default:
                printf("选项错误。\n");
        }
    }
    return 0;
}