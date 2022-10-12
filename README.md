Modified Ring-Buffer
====================
原项目：https://github.com/AndersKaloer/Ring-Buffer

对原项目做了扩展，当前支持动态申请内存，也可以支持将申请好的内存绑定到队列中。将队列容量做了扩展，目前可以支持到32767 byte容量。
在test_ring_buffer.c文件中提供的简单的使用案例。
如果需要改变队列大小的上限，可以在.h文件中改变RING_BUFFER_SIZE的定义和 typedef uint16_t ring_buffer_size_t;具体方法请看注释。

提供的函数说明请见.h文件。

下附原说明文件。

Ring-Buffer
===========

A simple ring buffer (circular buffer) designed for embedded systems.

An example is given in [examples/simple.c](examples/simple.c).

The size of the buffer can be configured by changing the value of `RING_BUFFER_SIZE` in `ring_buffer.h`. Note that `RING_BUFFER_SIZE` must be a power-of-two.

A new ring buffer is created using the `ring_buffer_init(buffer)` function:
```c
ring_buffer_t ring_buffer;
ring_buffer_init(&ring_buffer);
```

The module provides the following functions for accessing the ring buffer (documentation can be found in [ringbuffer.h](ringbuffer.h)):
```c
void ring_buffer_queue(ring_buffer_t *buffer, char data);
void ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size);
uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data);
ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len);
uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index);
uint8_t ring_buffer_is_empty(ring_buffer_t *buffer);
uint8_t ring_buffer_is_full(ring_buffer_t *buffer);
ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer);
```
