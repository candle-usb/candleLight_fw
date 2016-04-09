#pragma once
#include <stdbool.h>
#include <util.h>

typedef struct {
	unsigned max_elements;
	unsigned first;
	unsigned size;
	void **buf;
} queue_t;

queue_t *queue_create(unsigned max_elements);
void queue_free(queue_t *q);

unsigned queue_size(queue_t *q);
bool queue_is_empty(queue_t *q);
bool queue_push_back(queue_t *q, void *el);
void *queue_pop_front(queue_t *q);

unsigned queue_size_i(queue_t *q);
bool queue_is_empty_i(queue_t *q);
bool queue_push_back_i(queue_t *q, void *el);
void *queue_pop_front_i(queue_t *q);
