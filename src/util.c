#include <util.h>

inline int disable_irq(void) {
    int primask;
    asm volatile("mrs %0, PRIMASK\n"
                 "cpsid i\n" : "=r"(primask));
    return primask & 1;
}

inline void enable_irq(int primask) {
    if (primask)
        asm volatile("cpsie i\n");
}
