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

#include <queue.h>
#include <stdlib.h>

queue_t *queue_create(unsigned max_elements){
	queue_t *q = calloc(1, sizeof(queue_t));
	q->buf = calloc(max_elements, sizeof(void*));
	q->max_elements = max_elements;
	return q;
}

void queue_destroy(queue_t *q)
{
	free(q->buf);
	free(q);
}

unsigned queue_size(queue_t *q)
{
	int primask = disable_irq();
	unsigned retval = q->size;
	enable_irq(primask);
	return retval;
}

bool queue_is_empty(queue_t *q)
{
	return queue_size(q)==0;
}

bool queue_push_back(queue_t *q, void *el)
{
	bool retval = false;
	int primask = disable_irq();

	if (q->size < q->max_elements) {
		unsigned pos = (q->first + q->size) % q->max_elements;
		q->buf[pos] = el;
		q->size += 1;
		retval = true;
	}

	enable_irq(primask);
	return retval;
}

bool queue_push_front(queue_t *q, void *el)
{
	bool retval = false;
	int primask = disable_irq();
	if (q->size < q->max_elements) {
		if (q->first==0) {
			q->first = q->max_elements - 1;
		} else {
			q->first = q->first - 1;
		}
		q->buf[q->first] = el;
		q->size += 1;
		retval = true;
	}
	enable_irq(primask);
	return retval;
}

void *queue_pop_front(queue_t *q)
{
	int primask = disable_irq();
	void *el = 0;
	if (q->size > 0) {
		el = q->buf[q->first];
		q->first = (q->first + 1) % q->max_elements;
		q->size -= 1;
	}
	enable_irq(primask);
	return el;
}

unsigned queue_size_i(queue_t *q)
{
	return q->size;
}

bool queue_is_empty_i(queue_t *q)
{
	return queue_size_i(q)==0;
}

bool queue_push_back_i(queue_t *q, void *el)
{
	bool retval = false;

	if (q->size < q->max_elements) {
		unsigned pos = (q->first + q->size) % q->max_elements;
		q->buf[pos] = el;
		q->size += 1;
		retval = true;
	}

	return retval;
}

bool queue_push_front_i(queue_t *q, void *el)
{
	bool retval = false;
	if (q->size < q->max_elements) {
		if (q->first==0) {
			q->first = q->max_elements - 1;
		} else {
			q->first = q->first - 1;
		}
		q->buf[q->first] = el;
		q->size += 1;
		retval = true;
	}
	return retval;
}

void *queue_pop_front_i(queue_t *q)
{
	void *el = 0;
	if (q->size > 0) {
		el = q->buf[q->first];
		q->first = (q->first + 1) % q->max_elements;
		q->size -= 1;
	}
	return el;
}
