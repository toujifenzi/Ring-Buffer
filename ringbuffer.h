#include <inttypes.h>
#include <stddef.h>

/**
 * @file
 * 对环形队列进行了改写，使它可以支持可变长度的队列。
 * 由于算法原理限制，队列长度仍然需要保持是2的整数次幂。
 * 增加了ring_buffer_new函数，用于环形队列对象的分配。
 * 相对应的，增加了ring_buffer_destroy函数用于队列对象的释放。
 * 为了适用于共享内存工作环境，增加了三个函数，分别是
 * ring_buffer_calc_size
 * ring_buffer_attach
 * ring_buffer_detach
 * 目前队列最大可设置长度为32768个char。超过该长度将发生异常。
 * 队列的实际可利用长度为设置长度-1，需要予以注意。
 * 王景鑫   2022/10/10
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

// 最大允许队列长度
#define RING_BUFFER_SIZE 32768 // 2^15

/**
 * The type which is used to hold the size
 * and the indicies of the buffer.
 * Must be able to fit \c RING_BUFFER_SIZE .
 * **将其定义为uint32_t可以兼容更大的长度，需要同时修改RING_BUFFER_SIZE的宏定义为2^31次幂**
 */
typedef uint16_t ring_buffer_size_t;

/**
 * Simplifies the use of <tt>struct ring_buffer_t</tt>.
 */
typedef struct ring_buffer_t ring_buffer_t;

/**
 * Structure which holds a ring buffer.
 * The buffer contains a buffer array
 * as well as metadata for the ring buffer.
 */
struct ring_buffer_t
{
    /**
     * Buffer memory.
     * 用指针方式实现，在ring_buffer_new函数或者ring_buffer_attach中进行地址绑定。
     * */
    char *buffer_array;
    /* 队列缓冲的大小级别，2的整数幂次 */
    /**
     * 同时，这个变量也作为mask使用。mask = buffer_cap - 1
     */
    ring_buffer_size_t buffer_cap;
    /** Index of tail. */
    ring_buffer_size_t tail_index;
    /** Index of head. */
    ring_buffer_size_t head_index;
};

/**
 * Initializes the ring buffer pointed to by <em>buffer</em>.
 * This function can also be used to empty/reset the buffer.
 * @param buffer The ring buffer to initialize.
 */
void ring_buffer_init(ring_buffer_t *buffer);

/**
 * @brief 计算需要分配给ring_buffer_t结构体的内存大小，以sizeof(char)为单位。
 * 返回值是可以容纳length个char的最小2的整数次幂 + ring_buffer_t结构体大小。
 * @param length - 需要ring_buffer环形队列容纳的容量大小,以sizeof(char)为单位。。
 * @return 需要分配的内存大小，以sizeof(char)为单位。
 *
 */
ring_buffer_size_t ring_buffer_calc_size(size_t length);

/**
 * 初始化ring_buffer结构体，并返回该结构体对象的地址。
 * 如果初始化失败，则返回NULL。
 * @param buffer_length 申请分配队列的大小，其大小应不超过宏定义RING_BUFFER_SIZE。实际分配的队列大小为大于buffer_length的最小2的整数次幂。
 * @return 初始化完成的ring_buffer_t结构体对象。
 * 王景鑫 2022/10/10
 *
 */
ring_buffer_t *ring_buffer_new(ring_buffer_size_t buffer_length);

/**
 * @brief 销毁不再使用的队列对象。
 * @param buffer 将要销毁的ring_buffer_t对象指针的地址，请注意这是一个二级指针，需要传递结构体指针的地址。
 * 传递二级指针的目的是，在释放内存对象的同时将该指针置为NULL，以防止后续程序误用。
 *
 * 王景鑫 2002/10/10
 *
 */
void ring_buffer_destroy(ring_buffer_t **buffer);

/**
 * @brief 使用传入的内存块初始化ring_buffer_t并返回该结构体实例地址。
 * 此函数需传入已经分配好的内存块。 * 该内存块大小应使用ring_buffer_alloc_calculate函数计算获得。
 * 此函数的典型使用场景是进程间使用共享内存方式通信，并在分配好的共享内存中放置环形队列。
 * @param addr - 已经分配好的地址空间首地址，无类型指针。
 * @param length - 环形队列的长度。
 * @return 返回初始化好的ring_buffer对象地址。
 */
ring_buffer_t *ring_buffer_attach(void *addr, ring_buffer_size_t length);

/**
 * Adds a byte to a ring buffer.
 * @param buffer The buffer in which the data should be placed.
 * @param data The byte to place.
 */
void ring_buffer_queue(ring_buffer_t *buffer, char data);

/**
 * Adds an array of bytes to a ring buffer.
 * @param buffer The buffer in which the data should be placed.
 * @param data A pointer to the array of bytes to place in the queue.
 * @param size The size of the array.
 * @return 1 - success, 0 - fail, exceed buffer size.
 */
uint8_t ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size);

/**
 * Returns the oldest byte in a ring buffer.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data);

/**
 * Returns the <em>len</em> oldest bytes in a ring buffer.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the array at which the data should be placed.
 * @param len The maximum number of bytes to return.
 * @return The number of bytes returned.
 */
ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len);
/**
 * Peeks a ring buffer, i.e. returns an element without removing it.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @param index The index to peek.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index);

/**
 * Returns whether a ring buffer is empty.
 * @param buffer The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */

inline uint8_t ring_buffer_is_empty(ring_buffer_t *buffer);
inline uint8_t ring_buffer_is_full(ring_buffer_t *buffer);
inline ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer);
inline void ring_buffer_detach(ring_buffer_t **buffer);

#endif /* RINGBUFFER_H */
