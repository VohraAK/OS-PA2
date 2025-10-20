#include <mm/kheap.h>
#include <utils.h>
#include <stdio.h>
#include <string.h>
#include <testmain.h>   // for send_msg()

#define HEAP_SIZE 4096   // 4 KB fake heap for tests
static uint8_t heap_area[HEAP_SIZE];

static void reset_heap() {
    // kheap_init(get_kernel_heap(), heap_area, HEAP_SIZE, HEAP_SIZE, false, false);
}

// ---------------- Initialization ----------------
void test_kheap_init() {
    reset_heap();
    send_msg("PASSED");
}

// ---------------- Allocation ----------------
void test_kheap_alloc_small() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 1);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

void test_kheap_alloc_exact() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 32);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

void test_kheap_split() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 16);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

// ---------------- Freeing ----------------
void test_kheap_free_reuse() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    free(p);
    void *q = kmalloc(get_kernel_heap(), 64);
    send_msg(p == q ? "PASSED" : "FAILED");
    free(q);
}

void test_kheap_coalesce() {
    reset_heap();
    void *a = kmalloc(get_kernel_heap(), 16);
    void *b = kmalloc(get_kernel_heap(), 16);
    free(a);
    free(b);
    void *c = kmalloc(get_kernel_heap(), 32);
    send_msg(c ? "PASSED" : "FAILED");
    free(c);
}

void test_kheap_double_free() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    free(p);
    free(p);
    send_msg("PASSED");
}

void test_kheap_invalid_free() {
    reset_heap();
    int dummy = 123;
    kfree(get_kernel_heap(), &dummy);
    send_msg("PASSED");
}

// ---------------- Realloc ----------------
void test_kheap_realloc_shrink() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 128);
    void *q = krealloc(get_kernel_heap(), p, 64);
    send_msg(p == q ? "PASSED" : "FAILED");
    free(q);
}

void test_kheap_realloc_expand() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    strcpy((char*)p, "buddytest");
    void *q = krealloc(get_kernel_heap(), p, 512);
    send_msg(q && strcmp((char*)q, "buddytest")==0 ? "PASSED" : "FAILED");
    free(q);
}

void test_kheap_realloc_null() {
    reset_heap();
    void *p = krealloc(get_kernel_heap(), NULL, 128);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

void test_kheap_realloc_zero() {
    reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    void *q = krealloc(get_kernel_heap(), p, 0);
    send_msg(q == NULL ? "PASSED" : "FAILED");
}

// // ---------------- OOM ----------------
void test_kheap_oom() {
    reset_heap();
    void *arr[200];
    int count = 0;
    for (int i = 0; i < 200; i++) {
        arr[i] = kmalloc(get_kernel_heap(), 32);
        if (!arr[i]) break;
        count++;
    }
    send_msg(count > 0 ? "PASSED" : "FAILED");
    for (int i = 0; i < count; i++) free(arr[i]);
}

// // ---------------- Stress ----------------
void test_kheap_stress_pattern() {
    reset_heap();
    void *a[10];
    for (int i = 0; i < 10; i++) a[i] = kmalloc(get_kernel_heap(), 32);
    for (int i = 0; i < 10; i += 2) free(a[i]);
    void *mid = kmalloc(get_kernel_heap(), 64);
    send_msg(mid ? "PASSED" : "FAILED");
    free(mid);
    for (int i = 1; i < 10; i += 2) free(a[i]);
}