/*

The MIT License (MIT)

Copyright (c) 2016 Hubert Denkmair

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

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
bool queue_push_front(queue_t *q, void *el);
void *queue_pop_front(queue_t *q);

unsigned queue_size_i(queue_t *q);
bool queue_is_empty_i(queue_t *q);
bool queue_push_back_i(queue_t *q, void *el);
bool queue_push_front_i(queue_t *q, void *el);
void *queue_pop_front_i(queue_t *q);
