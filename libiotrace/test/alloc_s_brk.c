#include <stdio.h>
#include <unistd.h>


int main(void) {

    // Get current program break  (NOTE: Error checking is missing ..)
    void* ptr = sbrk(0);
    (void)printf("Address of program break: %p\n", ptr);

    // Change program break
    const intptr_t new_addr = (intptr_t)ptr + 2;
    (void)brk((void*)new_addr);

    // Get updated program break
    ptr = sbrk(0);
    (void)printf("Address of program break: %p\n", ptr);


    return 0;
}
