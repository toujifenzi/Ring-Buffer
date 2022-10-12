/*************************************************************************
	> File Name: test_ringbuffer.c
	> Created Time: Sat 08 Oct 2022 07:31:22 PM CST
 ************************************************************************/

// compile command:
//  gcc -I/include -o test_rb test_ringbuffer.c src/ringbuffer.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"

void main()
{

	char a[100], b = 0, c[100];
	int i;
	int length = 10, calcu_length;
	void *data = NULL;

	for (i = 0; i < 100; i++)
	{
		a[i] = i;
		c[i] = 0;
	}

#if 1
	ring_buffer_t *rb1;
	rb1 = ring_buffer_new(128);
	printf("1. ring_buffer_t create test start.\n");
	printf("1. sizeof ring_buffer_t is %lu.\n", sizeof(ring_buffer_t));
	printf("1. sizeof ring_buffer_t->buf_cap is %lu.\n", sizeof(rb1->buffer_cap));
	printf("1. ring_buffer_t->buf_cap is %d.\n", rb1->buffer_cap);
	printf("1. sizeof ring_buffer_t->head is %lu.\n", sizeof(rb1->head_index));
	printf("1. sizeof buffer_array is %lu.\n", sizeof(rb1->buffer_array));
	printf("1. addr of ring_buffer_t is %lu.\n", rb1);
	printf("1. addr of buffer_array is %lu.\n", rb1->buffer_array);

	if (rb1 == NULL)
	{
		printf("1. ring_buffer_new() failed!\n");
		exit(-1);
	}

	printf("1. ring_buffer created.\n\n");

	printf("2. test for single queue & dequeue:\n");
	for (i = 0; i < 100; i++)
	{
		ring_buffer_queue(rb1, i);
	}

	for (i = 0; i < 100; i++)
	{
		ring_buffer_dequeue(rb1, &b);
		if (b != i)
		{
			printf("2. error queue/dequeue on %d\n", i);
			break;
		}
	}
	printf("2. testing done, no error arise means OK.\n\n");

	printf("2. test for arr queue & arr dequeue:\n");
	ring_buffer_queue_arr(rb1, a, 100);
	ring_buffer_dequeue_arr(rb1, c, 100);
	for (i = 0; i < 100; i++)
	{
		if (a[i] != c[i])
		{
			printf("2. failed!  a[%d] = %d  ----  c[%d] = %d\n", i, a[i], i, c[i]);
			break;
		}
	}
	printf("2. ...OK\n\n");

	printf("3. test for peek:\n");
	ring_buffer_queue(rb1, 'D');
	ring_buffer_queue(rb1, 'E');
	ring_buffer_queue(rb1, 'F');
	ring_buffer_queue(rb1, 'G');
	printf("3. D~G is queued, the item number is %d.\n", ring_buffer_num_items(rb1));
	ring_buffer_peek(rb1, &b, 0);
	printf("3. and peek of %d it is %c.\n", 0, b);

	ring_buffer_peek(rb1, &b, 1);
	printf("3. and peek of %d it is %c.\n", 1, b);

	ring_buffer_peek(rb1, &b, 2);
	printf("3. and peek of %d it is %c.\n", 2, b);

	ring_buffer_peek(rb1, &b, 3);
	printf("3. and peek of %d it is %c.\n", 3, b);

	ring_buffer_destroy(&rb1);
    if (rb1 == NULL)
		printf("ring_buffer_destroyed and set to NULL.\n\n");

#else   //动态绑定内存方式
	
	printf("===============================================\n");
	printf("4. test of ring_buffer_attach.\n");

	calcu_length = ring_buffer_calc_size(length);
	printf("4. buffer size is %d, calculated length is %d.\n", length, calcu_length);

	data = malloc(ring_buffer_calc_size(calcu_length));
	printf("4. malloc OK, data at %p->%lu.\n\n", &data, data);

    //绑定内存块
	rb1 = ring_buffer_attach(data, calcu_length);
	// printf("4. ring_buffer_t attach test start.\n");
	// printf("4. sizeof ring_buffer_t is %d.\n", sizeof(ring_buffer_t));
	// printf("4. sizeof ring_buffer_t->buf_cap is %d.\n", sizeof(rb1->buffer_cap));
	// printf("4. ring_buffer_t->buf_cap is %d.\n", rb1->buffer_cap);
	// printf("4. sizeof ring_buffer_t->head is %d.\n", sizeof(rb1->head_index));
	// printf("4. sizeof buffer_array is %d.\n", sizeof(rb1->buffer_array));
	// printf("4. addr of ring_buffer_t is %lu.\n", rb1);
	// printf("4. addr of buffer_array is %lu.\n", rb1->buffer_array);

	if (rb1 == NULL)
	{
		printf("4. ring_buffer_attach() failed!\n");
		exit(-1);
	}

	printf("4. ring_buffer attached.\n\n");

	printf("4.1. test for single queue & dequeue:\n");
	for (i = 0; i < 10; i++)
	{
		ring_buffer_queue(rb1, i);
		printf("after  queue:buffer_array[%d] \t-- %4d head -> %4d,tail -> %4d\n", i % 16, rb1->buffer_array[i % 16], rb1->head_index, rb1->tail_index);
	}

	for (i = 0; i < 10; i++)
	{
		ring_buffer_dequeue(rb1, &b);
		if (b != i)
		{
			printf("4.1. error queue/dequeue on %d\n", i);
			break;
		}
	}

	ring_buffer_init(rb1);

	printf("4.1. testing done, no error arise means OK.\n\n");

	////////////////////////////////////////////////////////////
	printf("4.1. test for arr queue & arr dequeue:\n");
	for (i = 0; i < 16; i++)
		printf("before arr queue:buffer_array[%d] \t-- %4d head -> %4d,tail -> %4d\n", i % 16, rb1->buffer_array[i % 16], rb1->head_index, rb1->tail_index);

	ring_buffer_queue_arr(rb1, a, 15);
	for (i = 0; i < 16; i++)
	{
		printf("after arr queue:buffer_array[%d] \t-- %4d head -> %4d,tail -> %4d\n", i % 16, rb1->buffer_array[i % 16], rb1->head_index, rb1->tail_index);
	}

	ring_buffer_dequeue_arr(rb1, c, 15);
	for (i = 0; i < 16; i++)
	{
		printf("after arr dequeue:buffer_array[%d] \t-- %4d head -> %4d,tail -> %4d\n", i % 16, rb1->buffer_array[i % 16], rb1->head_index, rb1->tail_index);
		printf("c[%d]:%d\n", i, c[i]);
		if (a[i] != c[i])
		{
			printf("4.1. failed!  a[%d] = %d  ----  c[%d] = %d\n", i, a[i], i, c[i]);
			break;
		}
	}
	printf("4.1. no error arise means OK\n");
	///////////////////////////////////////////////////////
	ring_buffer_detach(&rb1);
	free(data);
#endif

	exit(0);
}
