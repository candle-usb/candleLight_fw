#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct _copy_table_t
{
	uint32_t const* src;
	uint32_t* dest;
	uint32_t wlen;
} copy_table_t;

extern const copy_table_t __copy_table_start__;
extern const copy_table_t __copy_table_end__;

void __initialize_hardware_early(void);
void _start(void) __attribute__((noreturn));

void Reset_Handler(void)
{
	__initialize_hardware_early();

	for (copy_table_t const* table = &__copy_table_start__; table < &__copy_table_end__; ++table) {
		memcpy(table->dest, table->src, table->wlen);
	}

	_start();
}

void __register_exitproc(void) {
	return;
}

void _exit(int code)
{
	(void) code;
	__asm__ ("BKPT");
	while (1);
}
