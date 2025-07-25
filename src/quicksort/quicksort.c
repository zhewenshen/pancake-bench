#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>

#ifdef PANCAKE_VERSION
static char cml_memory[1024 * 500];
extern void *cml_heap;
extern void *cml_stack;
extern void *cml_stackend;
extern void cml_main(void);
extern int quicksort_main(void);

static int *original_array = NULL;
static int array_size = 0;

void cml_exit(int arg) {
    exit(arg);
}

void cml_err(int arg) {
    if (arg == 3) {
        fprintf(stderr, "Memory not ready for entry. You may have not run the init code yet, or be trying to enter during an FFI call.\n");
    }
    cml_exit(arg);
}

void cml_clear() {}

void init_pancake_mem() {
    unsigned long cml_heap_sz = 1024 * 300;
    unsigned long cml_stack_sz = 1024 * 200;
    cml_heap = cml_memory;
    cml_stack = cml_heap + cml_heap_sz;
    cml_stackend = cml_stack + cml_stack_sz;
}

void ffireset_array(unsigned char *c, long clen, unsigned char *a, long alen) {
    uintptr_t *pnk_mem = (uintptr_t *) cml_heap;
    int *array = (int *) pnk_mem[1];
    int n = pnk_mem[0];
    
    for (int i = 0; i < n; i++) {
        array[i] = rand() & 0xFFFF;
    }
}
#endif

#ifndef PANCAKE_VERSION
static int call_count = 0;

void quicksort(int lo, int hi, int base[]) {
    int i, j;
    int pivot, temp;
    
    call_count++;
    
    if (lo < hi) {
        for (i = lo, j = hi, pivot = base[hi]; i < j;) {
            while (i < hi && base[i] <= pivot) i++;
            while (j > lo && base[j] >= pivot) j--;
            if (i < j) {
                temp = base[i];
                base[i] = base[j];
                base[j] = temp;
            }
        }
        temp = base[i];
        base[i] = base[hi];
        base[hi] = temp;
        quicksort(lo, i - 1, base);
        quicksort(i + 1, hi, base);
    }
}

void print_call_count() {
    printf("C quicksort calls: %d\n", call_count);
    call_count = 0;
}
#endif

int cmpint(const void *i, const void *j) {
    int vi = *((int *) i);
    int vj = *((int *) j);
    if (vi == vj) return 0;
    if (vi < vj) return -1;
    return 1;
}

#define NITER 10

int main(int argc, char **argv) {
    int n, i;
    int *a, *b;
#ifndef PANCAKE_VERSION
    int j;
    
    struct rlimit rl;
    getrlimit(RLIMIT_STACK, &rl);
    rl.rlim_cur = 64 * 1024 * 1024;
    setrlimit(RLIMIT_STACK, &rl);
#endif

    if (argc >= 2) 
        n = atoi(argv[1]); 
    else 
        n = 100000;
    
    a = malloc(n * sizeof(int));
    b = malloc(n * sizeof(int));
    
    for (i = 0; i < n; i++) {
        a[i] = rand() & 0xFFFF;
    }

#ifdef PANCAKE_VERSION
    init_pancake_mem();
    uintptr_t *pnk_mem = (uintptr_t *) cml_heap;
    
    pnk_mem[0] = n;
    pnk_mem[1] = (uintptr_t) a;
    pnk_mem[2] = NITER;
    
    cml_main();
    quicksort_main();
    
    memcpy(b, a, n * sizeof(int));
    
#else
    for (j = 0; j < NITER; j++) {
        if (j < NITER - 1) {
            for (i = 0; i < n; i++) 
                a[i] = rand() & 0xFFFF;
        } else {
            memcpy(b, a, n * sizeof(int));
        }
        quicksort(0, n - 1, a);
    }
    print_call_count();
#endif

    qsort(b, n, sizeof(int), cmpint);
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) { 
            printf("Bug at position %d: got %d, expected %d\n", i, a[i], b[i]); 
            return 2; 
        }
    }
    printf("OK\n");
    
    free(a);
    free(b);
    return 0;
}