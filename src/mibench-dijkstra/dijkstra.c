#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_NODES 100
#define NONE 9999

#ifdef PANCAKE_VERSION
static char cml_memory[1024*500];
extern void *cml_heap;
extern void *cml_stack;
extern void *cml_stackend;
extern void cml_main(void);
extern int dijkstra_main(void);
void cml_exit(int arg) {
    exit(arg);
}
void cml_err(int arg) {
    if (arg == 3) {
        fprintf(stderr, "Memory not ready for entry. You may have not run the init code yet, or be trying to enter during an FFI call.\n");
    }
    cml_exit(arg);
}
void cml_clear() {
}
void init_pancake_mem() {
    unsigned long cml_heap_sz = 1024*400;
    unsigned long cml_stack_sz = 1024*100;
    cml_heap = cml_memory;
    cml_stack = cml_heap + cml_heap_sz;
    cml_stackend = cml_stack + cml_stack_sz;
}
#endif

#if defined(PANCAKE_VERSION) && defined(ENABLE_PRINT)
const char *str_shortest_0 = "Shortest path is 0 in cost. Just stay where you are.\n";
const char *str_shortest_is = "Shortest path is ";
const char *str_in_cost = " in cost. ";
const char *str_path_is = "Path is: ";
const char *str_newline = "\n";
const char *str_space = " ";
#endif

#ifndef PANCAKE_VERSION
long AdjMatrix[NUM_NODES][NUM_NODES];

struct _NODE {
  long iDist;
  long iPrev;
};
typedef struct _NODE NODE;

struct _QITEM {
  long iNode;
  long iDist;
  long iPrev;
  struct _QITEM *qNext;
};
typedef struct _QITEM QITEM;

QITEM *qHead = NULL;
long g_qCount = 0;
NODE rgnNodes[NUM_NODES];
long ch;
long iPrev, iNode;
long i, iCost, iDist;

void print_path(NODE *rgnNodes, long chNode) {
    if (rgnNodes[chNode].iPrev != NONE) {
        print_path(rgnNodes, rgnNodes[chNode].iPrev);
    }
#ifdef ENABLE_PRINT
    printf(" %ld", chNode);
    fflush(stdout);
#endif
}

void enqueue(long iNode, long iDist, long iPrev) {
    QITEM *qNew = (QITEM *) malloc(sizeof(QITEM));
    QITEM *qLast = qHead;
    if (!qNew) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    qNew->iNode = iNode;
    qNew->iDist = iDist;
    qNew->iPrev = iPrev;
    qNew->qNext = NULL;
    if (!qLast) {
        qHead = qNew;
    } else {
        while (qLast->qNext) qLast = qLast->qNext;
        qLast->qNext = qNew;
    }
    g_qCount++;
}

void dequeue(long *piNode, long *piDist, long *piPrev) {
    QITEM *qKill = qHead;
    if (qHead) {
        *piNode = qHead->iNode;
        *piDist = qHead->iDist;
        *piPrev = qHead->iPrev;
        qHead = qHead->qNext;
        free(qKill);
        g_qCount--;
    }
}

long qcount(void) {
    return g_qCount;
}

int dijkstra(long chStart, long chEnd) {
    for (ch = 0; ch < NUM_NODES; ch++) {
        rgnNodes[ch].iDist = NONE;
        rgnNodes[ch].iPrev = NONE;
    }
    if (chStart == chEnd) {
#ifdef ENABLE_PRINT
        printf("Shortest path is 0 in cost. Just stay where you are.\n");
#endif
    } else {
        rgnNodes[chStart].iDist = 0;
        rgnNodes[chStart].iPrev = NONE;
        enqueue(chStart, 0, NONE);
        while (qcount() > 0) {
            dequeue(&iNode, &iDist, &iPrev);
            for (i = 0; i < NUM_NODES; i++) {
                if ((iCost = AdjMatrix[iNode][i]) != NONE) {
                    if ((NONE == rgnNodes[i].iDist) || (rgnNodes[i].iDist > (iCost + iDist))) {
                        rgnNodes[i].iDist = iDist + iCost;
                        rgnNodes[i].iPrev = iNode;
                        enqueue(i, iDist + iCost, iNode);
                    }
                }
            }
        }
#ifdef ENABLE_PRINT
        printf("Shortest path is %ld in cost. ", rgnNodes[chEnd].iDist);
        printf("Path is: ");
        print_path(rgnNodes, chEnd);
        printf("\n");
#endif
    }
    return 0;
}
#endif

int main(int argc, char *argv[]) {
    FILE *fp;
    long i, j, k;
    if (argc < 2) {
        fprintf(stderr, "Usage: dijkstra <filename>\n");
        fprintf(stderr, "Only supports matrix size is #define'd.\n");
        return 1;
    }
    fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file\n");
        return 1;
    }
#ifdef PANCAKE_VERSION
    init_pancake_mem();
    uintptr_t *pnk_mem = (uintptr_t *) cml_heap;
#ifdef ENABLE_PRINT
    pnk_mem[2] = (uintptr_t) str_shortest_0;
    pnk_mem[3] = (uintptr_t) str_shortest_is;
    pnk_mem[4] = (uintptr_t) str_in_cost;
    pnk_mem[5] = (uintptr_t) str_path_is;
    pnk_mem[6] = (uintptr_t) str_newline;
    pnk_mem[7] = (uintptr_t) str_space;
#endif
#endif
    for (i = 0; i < NUM_NODES; i++) {
        for (j = 0; j < NUM_NODES; j++) {
            fscanf(fp, "%ld", &k);
#ifdef PANCAKE_VERSION
            pnk_mem[1000 + i * NUM_NODES + j] = k;
#else
            AdjMatrix[i][j] = k;
#endif
        }
    }
    fclose(fp);
#ifdef PANCAKE_VERSION
    cml_main();
    return dijkstra_main();
#else
    for (i = 0, j = NUM_NODES/2; i < 100; i++, j++) {
        j = j % NUM_NODES;
        dijkstra(i, j);
    }
    return 0;
#endif
}
