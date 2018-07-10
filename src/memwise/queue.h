#if defined(QUEUE_CLEAN)
#undef QUEUE_CLEAN

#undef queue_t
#undef queue_init
#undef queue_free
#undef queue_peek
#undef queue_enque
#undef queue_deque

#elif defined(QUEUE_CONFIG)
#undef QUEUE_CONFIG

#if defined(QUEUE__SHOULD_CLEAN)
#undef  QUEUE__SHOULD_CLEAN
#define QUEUE_CLEAN
#include __FILE__
#endif

#undef QUEUE__DEFINE
#undef QUEUE__CONCAT
#undef QUEUE__CONCAT_IN
#define QUEUE__DEFINE(name) QUEUE__CONCAT(QUEUE_PREFIX, name)
#define QUEUE__CONCAT(a, b) QUEUE__CONCAT_IN(a, b)
#define QUEUE__CONCAT_IN(a, b) a ## b

#define queue_t     QUEUE__DEFINE(_t)
#define queue_init  QUEUE__DEFINE(_init)
#define queue_free  QUEUE__DEFINE(_free)
#define queue_peek  QUEUE__DEFINE(_peek)
#define queue_enque QUEUE__DEFINE(_enque)
#define queue_deque QUEUE__DEFINE(_deque)

#define QUEUE__SHOULD_CLEAN

#else

/* COPY FROM membuf.h */
#ifndef HAS_MEMBUF_T
#define HAS_MEMBUF_T
/**
 * Memory buffer
 */
typedef struct
{
    void* data;

    void  (*clear)(void* data);
    void* (*resize)(void* data, void* ptr, int size, int align);
    void  (*collect)(void* data, void* pointer);
    void* (*extract)(void* data, int size, int align);
} membuf_t;

#ifndef __cplusplus
#  if __GNUC__
#    define inline __inline__
#  elif defined(_MSC_VER)
#    define inline __inline
#  else
#    define inline
#  endif
#endif 

static inline void membuf_clear(membuf_t* buf)
{
    buf->clear(buf->data);
}

static inline void* membuf_resize(membuf_t* buf, void* ptr, int size, int align)
{
    return buf->resize(buf->data, ptr, size, align);
}

static inline void membuf_collect(membuf_t* buf, void* ptr)
{
    buf->collect(buf->data, ptr);
}

static inline void* membuf_extract(membuf_t* buf, int size, int align)
{
    return buf->extract(buf->data, size, align);
}

/* END OF HAS_MEMBUF_T */
#endif

#ifndef __cplusplus
# if __STDC_VERSION__ == 201112L
#  include <stdbool.h>
# else
#  ifndef bool
#  define bool  unsigned char
#  endif
#  ifndef true
#  define true  0
#  endif
#  ifndef false
#  define false 1
#  endif
# endif
#endif

#ifndef QUEUE_API
#define QUEUE_API
#endif

typedef struct
{
    int              head;
    int              tail;
    int              capacity;
    queue_element_t* elements;
    membuf_t*        membuffer;
} queue_t;

QUEUE_API void queue_init(queue_t* queue, membuf_t* membuffer);
QUEUE_API void queue_free(queue_t* queue);

QUEUE_API queue_element_t queue_peek(const queue_t* queue);
QUEUE_API bool            queue_enque(queue_t* queue, queue_element_t element);
QUEUE_API queue_element_t queue_deque(queue_t* queue);

#if defined(QUEUE_IMPL)

#include <string.h>
#ifndef queue_assert
#include <assert.h>
#define queue_assert(q, e) assert(q != NULL); assert(e)
#endif

#ifndef QUEUE_CAPACITY
#define QUEUE_CAPACITY 64
#endif

void queue_init(queue_t* queue, membuf_t* membuffer)
{
    queue_assert(queue, membuffer != NULL);
    
    queue->head      = 0;
    queue->tail      = 0;
    queue->capacity  = 0;
    queue->elements  = 0;
    queue->membuffer = membuffer;
}

void queue_free(queue_t* queue)
{
    queue_assert(queue, queue->membuffer != NULL);

    membuf_collect(queue->membuffer, queue->elements);

    queue->head      = 0;
    queue->tail      = 0;
    queue->capacity  = 0;
    queue->elements  = 0;
}

queue_element_t queue_peek(const queue_t* queue)
{
    queue_assert(queue, queue->head < queue->tail);
    return queue->elements[queue->head];
}

bool queue_enque(queue_t* queue, queue_element_t element)
{
    queue_assert(queue, queue->membuffer != NULL);
    
    if (queue->tail >= queue->capacity)
    {
	if (queue->head > 0)
	{
	    const int count  = queue->tail - queue->head;
	    const int offset = queue->head;
	    memcpy(queue->elements,
		   queue->elements + offset,
		   count * sizeof(queue_element_t));
	    
	    queue->head = 0;
	    queue->tail = count;
	}
	else
	{
	    if (queue->capacity <= 0) queue->capacity  = QUEUE_CAPACITY;
	    else                      queue->capacity *= 2;
	    
	    const int new_size = queue->capacity * sizeof(queue_element_t);
	    void* new_elements = membuf_resize(queue->membuffer,
					       queue->elements, new_size, 4);
	    if (new_data)
	    {
		queue->elements = (queue_element_t*)new_data;
	    }
	    else
	    {
		return false;
	    }
	}
    }
    
    queue->elements[queue->tail++] = element;
    return true;
}

queue_element_t queue_deque(queue_t* queue)
{
    queue_assert(queue, queue->head < queue->tail);
    return queue->elements[queue->head++];
}

#endif

#endif
