#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "ringbuffer.h"

/**
 * @file
 * Implementation of ring buffer functions.
 *
 * 可动态分配队列大小版本，可兼容共享内存方式使用。
 * 王景鑫 2022/10/10
 */

void ring_buffer_init(ring_buffer_t *buffer)
{
    buffer->tail_index = 0;
    buffer->head_index = 0;
}

ring_buffer_size_t ring_buffer_calc_size(size_t buffer_length)
{
    int i;
    //长度检查
    if (buffer_length > RING_BUFFER_SIZE)
    {
        fprintf(stderr, "%s -- ring_buffer_size exceed max range of RING_BUFFER_SIZE(%d).\n", __func__, RING_BUFFER_SIZE);
        return 0;
    }

    for (i = 1; i <= RING_BUFFER_SIZE; i *= 2)
    {
        if (buffer_length <= i)
        {
            buffer_length = i;
            // printf("buffer_length = %lu\n",buffer_length);
            //返回地址块大小包含结构体占用的部分。
            return (buffer_length + (size_t)sizeof(ring_buffer_t));
        }
    }
}

ring_buffer_t *ring_buffer_new(ring_buffer_size_t buffer_length)
{

    ring_buffer_t *buffer = NULL;

    buffer_length = ring_buffer_calc_size(buffer_length);

    //使用一次malloc分配内存，大小是sizeof(ring_buffer_t) + sizeof(buffer_cap*sizeof(char))
    //然后把结构体首地址赋给结构体指针buffer，数组首地址赋给buffer->buffer_array。
    buffer = (ring_buffer_t *)malloc(buffer_length * sizeof(char));

    // printf("sizeof ring_buffer_t:%d, sizeof buffer_array:%d.\n",sizeof(ring_buffer_t),(ring_buffer_size * sizeof(char)));
    // printf("malloc, buffer addr %p -> %d (ring_buffer_t).\n",&buffer,buffer);
    // printf("tail addr of malloc:%d.\n",(int)buffer + sizeof(ring_buffer_t) + ring_buffer_size * sizeof(char));

    if (buffer == NULL)
    {
        fprintf(stderr, "%s -- malloc failed:%s\n", __func__, strerror(errno));
        free(buffer);
        return (ring_buffer_t *)NULL;
    }

    // printf("addr of buffer++:%d.\n",buffer+1);
    // buffer+1得到结构体后面的地址，将其类型转换为(char*)后赋值给buffer->array
    buffer->buffer_array = (char *)(buffer + 1);
    buffer->buffer_cap = buffer_length - sizeof(ring_buffer_t);

    //设置成员变量的值
    ring_buffer_init(buffer);

    return buffer;
}

void ring_buffer_destroy(ring_buffer_t **buffer)
{

    //检查buffer指针和buffer->buf_array指针是否有效
    if (*buffer == NULL)
    {
        fprintf(stderr, "%s -- ring_buffer_t ptr is NULL.\n", __func__);
        return;
    }

    free(*buffer);
    *buffer = NULL;
}

ring_buffer_t *ring_buffer_attach(void *addr, ring_buffer_size_t length)
{
    ring_buffer_t *buffer;
    //判断传入参数是否为空值。
    if (addr == NULL)
    {
        fprintf(stderr, "%s paramater *addr is NULL.\n", __func__);
        return NULL;
    }

    buffer = addr;
    buffer->buffer_array = (char *)(buffer + 1);
    buffer->buffer_cap = length - sizeof(ring_buffer_t);

    //设置成员变量的值
    ring_buffer_init(buffer);

    return buffer;
}

void ring_buffer_queue(ring_buffer_t *buffer, char data)
{
    /* Is buffer full? */
    if (ring_buffer_is_full(buffer))
    {
        /* Is going to overwrite the oldest byte */
        /* Increase tail index */
        buffer->tail_index = ((buffer->tail_index + 1) & (buffer->buffer_cap - 1));
    }

    /* Place data in buffer */
    buffer->buffer_array[buffer->head_index] = data;
    buffer->head_index = ((buffer->head_index + 1) & (buffer->buffer_cap - 1));
}

uint8_t ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size)
{
    /* Add bytes; one by one */
    /* 增加入队列数据大小的判断，超过队列容量则失败返回。 */
    if (size > buffer->buffer_cap)
    {
        fprintf(stderr, "%s -- queue array size exceed buffer size.\n", __func__);
        return 0;
    }

    ring_buffer_size_t i;
    for (i = 0; i < size; i++)
    {
        ring_buffer_queue(buffer, data[i]);
    }
    return 1;
}

uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data)
{
    if (ring_buffer_is_empty(buffer))
    {
        /* No items */
        return 0;
    }

    *data = buffer->buffer_array[buffer->tail_index];
    buffer->tail_index = ((buffer->tail_index + 1) & (buffer->buffer_cap - 1));
    return 1;
}

ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len)
{
    if (ring_buffer_is_empty(buffer))
    {
        /* No items */
        return 0;
    }

    char *data_ptr = data;
    ring_buffer_size_t cnt = 0;
    while ((cnt < len) && ring_buffer_dequeue(buffer, data_ptr))
    {
        cnt++;
        data_ptr++;
    }
    return cnt;
}

uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index)
{
    if (index >= ring_buffer_num_items(buffer))
    {
        /* No items at index */
        return 0;
    }

    /* Add index to pointer */
    ring_buffer_size_t data_index = ((buffer->tail_index + index) & (buffer->buffer_cap - 1));
    *data = buffer->buffer_array[data_index];
    return 1;
}

inline uint8_t ring_buffer_is_empty(ring_buffer_t *buffer)
{
    return (buffer->head_index == buffer->tail_index);
}

/**
 * Returns whether a ring buffer is full.
 * @param buffer The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
inline uint8_t ring_buffer_is_full(ring_buffer_t *buffer)
{
    return ((buffer->head_index - buffer->tail_index) & (buffer->buffer_cap - 1)) == (buffer->buffer_cap - 1);
}

/**
 * Returns the number of items in a ring buffer.
 * @param buffer The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
inline ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer)
{
    return ((buffer->head_index - buffer->tail_index) & (buffer->buffer_cap - 1));
}

/**
 * @brief 将共享内存结构体指针设置为NULL。如果该指针是最后一个指向该地址的，
 * 将导致这块地址无法被再次使用或释放（造成内存泄漏），所以应确保对分配的内存进行妥善管理。
 * 这个函数仅用于与ring_buffer_assign配对使用。如果使用了ring_buffer_new
 * 进行对象初始化，则需要使用ring_buffer_destroy对对象进行销毁。
 * @param buffer - 需要解除引用的指针。请注意这是二级指针，需要传递结构体指针的地址。
 */
inline void ring_buffer_detach(ring_buffer_t **buffer)
{
    *buffer = NULL;
}
