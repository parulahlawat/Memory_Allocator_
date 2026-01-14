#include <stdio.h>
#include <stdlib.h>

/* Declare allocator functions */
void* my_malloc(size_t size);
void my_free(void* ptr);

int main() {
    int* a = my_malloc(100 * sizeof(int));
    int* b = my_malloc(50 * sizeof(int));

    for (int i = 0; i < 100; i++) {
        a[i] = i;
    }

    printf("a[10] = %d\n", a[10]);

    my_free(a);
    my_free(b);

    int* c = my_malloc(80 * sizeof(int));
    printf("Reused block c[10] = %d\n", c[10]);

    my_free(c);

    return 0;
}