#include <stdint.h>
#include <stddef.h>

typedef struct _copy_table_t  
{
    uint32_t const* src;
    uint32_t* dest;
    uint32_t  wlen;
} copy_table_t;

typedef struct _zero_table_t
{
    uint32_t* dest;
    uint32_t  wlen;
} zero_table_t;

extern const copy_table_t __copy_table_start__;
extern const copy_table_t __copy_table_end__;
extern const zero_table_t __zero_table_start__;
extern const zero_table_t __zero_table_end__;

void __initialize_hardware_early();
void _start() __attribute__((noreturn));

void Reset_Handler()
{
    __initialize_hardware_early();

    for (copy_table_t const* table = &__copy_table_start__; table < &__copy_table_end__; ++table) 
    {
        for (size_t i=0; i<table->wlen; ++i) 
        {
            table->dest[i] = table->src[i];
        }
    }

    for (zero_table_t const* table = &__zero_table_start__; table < &__zero_table_end__; ++table) 
    {
        for (size_t i=0; i<table->wlen; ++i) 
        {
            table->dest[i] = 0u;
        }
    }

    _start();
}


void _exit(int code)
{
    (void) code;
    __asm__("BKPT");
    while (1);
}