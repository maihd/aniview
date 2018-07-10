/**************************************************************************
 * Memory buffer is a buffer that contains memory
 * and a manager for allocation/deallocation
 *
 * The term is also like allocator in C++
 * but membuf attempt user to think in C way than C++/OO approach
 *
 * So the algorithm (types and functions) like allocators, including:
 *      + pool
 *      + stack
 *      + double-ended stack
 *      + micro
 *      + one-frame, two-frame temporary
 *      + double-buffered I/O
 *      + general purpose (get memory directly from heap, membuf_heap())
 *************************************************************************/

#ifndef __MEMBUF_H__
#define __MEMBUF_H__

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

#ifndef MEMBUF_API
#define MEMBUF_API
#endif

MEMBUF_API membuf_t* membuf_heap(void);
MEMBUF_API membuf_t* membuf_pool(void* data, int size, int block);
MEMBUF_API membuf_t* membuf_stack(void* data, int size);
MEMBUF_API membuf_t* membuf_linear(void* data, int size);
MEMBUF_API membuf_t* membuf_destack(void* data, int size);

MEMBUF_API membuf_t* membuf_new_stack(int size);
MEMBUF_API membuf_t* membuf_new_linear(int size);
MEMBUF_API membuf_t* membuf_new_destack(int size);
MEMBUF_API membuf_t* membuf_new_pool(int count, int block);

MEMBUF_API void      membuf_free(membuf_t* buf);

#endif /* __MEMWISE_MEMBUF_H__ */

#ifdef MEMBUF_IMPL

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct
{
    int   size;
    int   count;
    int   block;
    void* first;
} poolbuf_t;

typedef struct
{
    int size;
    int tail;
} linearbuf_t;

typedef struct
{
    int size;
    int tail;
} stackbuf_t;

typedef struct
{
    int size;
    int head;
    int tail;
} destackbuf_t;

/*********************************
 * @region: Constructors
 *********************************/
membuf_t* membuf_new_linear(int size)
{
    int real_size = sizeof(membuf_t) + sizeof(linearbuf_t) + size;
    void*     mem = malloc(real_size);
    membuf_t* res = NULL;
    if (mem)
    {
	    return membuf_linear(mem, real_size);
    }
    else
    {
	    return NULL;
    }
}

membuf_t* membuf_new_stack(int size)
{
    int real_size = sizeof(membuf_t) + sizeof(stackbuf_t) + size;
    void*     mem = malloc(real_size);
    if (mem)
    {
	    return membuf_stack(mem, real_size);
    }
    else
    {
	    return NULL;
    }
}

membuf_t* membuf_new_destack(int size)
{
    int real_size = sizeof(membuf_t) + sizeof(destackbuf_t) + size;
    void*     mem = malloc(real_size);
    if (mem)
    {
	    return membuf_destack(mem, real_size);
    }
    else
    {
	    return NULL;
    }
}

membuf_t* membuf_new_pool(int count, int block)
{
    int pool_size = count * (sizeof(void*) + block);
    int real_size = sizeof(membuf_t) + sizeof(poolbuf_t) + pool_size;
    void*     mem = malloc(real_size);
    if (mem)
    {
	    return membuf_pool(mem, real_size, block);
    }
    else
    {
	    return NULL;
    }
}

void membuf_free(membuf_t* buf)
{
    free(buf);
}

/****************************************
 * @region: Functions implement
 ****************************************/
static void  membuf_heap_clear(void* data)
{
    (void)data;
}

static void* membuf_heap_extract(void* data, int size, int align);
static void* membuf_heap_resize(void* data, void* ptr, int size, int align)
{
    (void)data;

    assert(align > 1 && align <= 256);
    assert((align & (align - 1)) == 0);

    if (ptr)
    {
        int misalign = *((unsigned char*)ptr - 1);
        void* res = realloc((char*)ptr - misalign, size + align);
        if (res)
        {
            misalign = align - (int)((intptr_t)res & align);
            
            res = (char*)res + misalign;
            *((unsigned char*)res - 1) = misalign;
        }
        return res;
    }
    else
    {
	    return membuf_heap_extract(NULL, size, align);
    }
}

static void* membuf_heap_extract(void* data, int size, int align)
{
    (void)data;

    assert(align > 1 && align <= 256);
    assert((align & (align - 1)) == 0);
    
    void* res = malloc(size + align);
    if (res)
    {
	    int misalign = align - (int)((intptr_t)res & align);
	
	    res = (char*)res + misalign;
	    *((unsigned char*)res - 1) = (unsigned char)misalign;
    }
    return res;
}

static void  membuf_heap_collect(void* data, void* pointer)
{
    (void)data;

    if (pointer)
    {
	    int misalign = *((unsigned char*)pointer - 1);
        void* target = (char*)pointer - misalign;
	    free(target);
    }
}

static membuf_t membuf__heap =
{
    NULL,
    membuf_heap_clear,
    membuf_heap_resize,
    membuf_heap_collect,
    membuf_heap_extract,
};

membuf_t* membuf_heap(void)
{
    return &membuf__heap;
}

static void membuf_linear_clear(void* userdata)
{
    linearbuf_t* buf = (linearbuf_t*)userdata;
    buf->tail = 0;
}

static void* membuf_linear_extract(void* userdata, int size, int align);
static void* membuf_linear_resize(void* userdata, void* ptr, int size, int align)
{
    linearbuf_t* buf = (linearbuf_t*)userdata;
    intptr_t address = (intptr_t)ptr - (intptr_t)userdata;
    if (address > 0 && address + size <= buf->tail)
    {
	    buf->tail = buf->tail > address + size ? buf->tail : address + size;
	    return ptr;
    }
    else
    {
	    void* res = membuf_linear_extract(userdata, size, align);
	    if (ptr && res)
	    {
	        memcpy(res, ptr, size);
	    }
	    return res;
    }
}

static void* membuf_linear_extract(void* userdata, int size, int align)
{
    linearbuf_t* buf = (linearbuf_t*)userdata;

    if (buf->tail + size <= buf->size)
    {
	    char* mem = (char*)userdata + sizeof(linearbuf_t) + buf->tail;
	    buf->tail += size;
	    return mem;
    }
    else
    {
	    return NULL;
    }
}

static void  membuf_linear_collect(void* userdata, void* ptr)
{
    linearbuf_t* buf = (linearbuf_t*)userdata;
    intptr_t address = (intptr_t)ptr - (intptr_t)userdata;
    if (address > 0 && address < buf->tail)
    {
	    buf->tail = 0;
    }
}

membuf_t* membuf_linear(void* data, int size)
{
    if (size <= sizeof(membuf_t) + sizeof(linearbuf_t))
    {
	    return NULL;
    }
    else
    {
        membuf_t* buf = (membuf_t*)data;
        buf->data     = (void*)((char*)data + sizeof(membuf_t));
        buf->clear    = membuf_linear_clear;
        buf->resize   = membuf_linear_resize;
        buf->extract  = membuf_linear_extract;
        buf->collect  = membuf_linear_collect;

        linearbuf_t* ldata = (linearbuf_t*)buf->data;
        ldata->tail = 0;
        ldata->size = size - sizeof(membuf_t) - sizeof(linearbuf_t);
        
        return buf;
    }
}

static void  membuf_stack_clear(void* userdata)
{
    stackbuf_t* buf = (stackbuf_t*)userdata;

    buf->tail = 0;
}

static void* membuf_stack_extract(void* userdata, int size, int align);
static void* membuf_stack_resize(void* userdata, void* ptr, int size, int align)
{
    stackbuf_t* buf = (stackbuf_t*)userdata;
    intptr_t address = (intptr_t)ptr - (intptr_t)userdata;
    if (buf->tail > -1 && address == buf->tail + sizeof(int))
    {
        int* next = (int*)((char*)ptr - sizeof(int));
        *next = size;
        return ptr;
    }
    else
    {
	    return membuf_stack_extract(userdata, size, align);
    }
}

static void* membuf_stack_extract(void* userdata, int size, int align)
{
    stackbuf_t* buf = (stackbuf_t*)userdata;

    if (buf->tail < 0)
    {
        if (buf->size >= size)
        {
            buf->tail = 0;
            int* next = (int*)((char*)userdata + sizeof(stackbuf_t));
            void* mem = (char*)next + sizeof(int);
            *next = size;
            return mem;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        int* next = (int*)((char*)userdata + sizeof(stackbuf_t) + buf->tail);

        if (buf->tail + (*next) + size <= buf->size)
        {
            buf->tail += *next;
            next = (int*)((char*)next + (*next));
            void* mem = (char*)next + sizeof(int);
            *next = size;
            return mem;
        }
        else
        {
            return NULL;
        }
    }
}

static void  membuf_stack_collect(void* userdata, void* ptr)
{
    stackbuf_t* buf = (stackbuf_t*)userdata;
    intptr_t address = (intptr_t)ptr - (intptr_t)userdata;
    if (buf->tail > -1 && address == buf->tail + sizeof(int))
    {
        if (buf->tail > 0)
        {
            int size = *(int*)((char*)ptr - sizeof(int));
            buf->tail -= size;
        }
        else
        {
            buf->tail = -1;
        }
    }
}

membuf_t* membuf_stack(void* data, int size)
{
    if (size <= sizeof(membuf_t) + sizeof(stackbuf_t))
    {
	    return NULL;
    }
    else
    {
        membuf_t* buf = (membuf_t*)data;
        buf->data     = (void*)((char*)data + sizeof(membuf_t));
        buf->clear    = membuf_stack_clear;
        buf->resize   = membuf_stack_resize;
        buf->extract  = membuf_stack_extract;
        buf->collect  = membuf_stack_collect;

        stackbuf_t* sdata = (stackbuf_t*)buf->data;
        sdata->tail = -1;
        sdata->size = size - sizeof(membuf_t) - sizeof(stackbuf_t);
        
        return buf;
    }
}

static void  membuf_destack_clear(void* userdata)
{
    destackbuf_t* buf = (destackbuf_t*)userdata;

    buf->head = 0;
    buf->tail = 0;
}

static void* membuf_destack_resize(void* userdata, void* ptr, int size, int align)
{
    destackbuf_t* buf = (destackbuf_t*)userdata;

    (void)ptr;
    (void)size;
    (void)align;

    return NULL;
}

static void* membuf_destack_extract(void* userdata, int size, int align)
{
    destackbuf_t* buf = (destackbuf_t*)userdata;

    (void)size;
    (void)align;
    
    return NULL;
}

static void  membuf_destack_collect(void* userdata, void* ptr)
{
    destackbuf_t* buf = (destackbuf_t*)userdata;
    intptr_t address  = (intptr_t)ptr - (intptr_t)userdata;
    if (buf->tail > -1 && address == buf->tail + sizeof(int))
    {
    }
    else
    {
    }
}

membuf_t* membuf_destack(void* data, int size)
{
    if (size <= sizeof(membuf_t) + sizeof(destackbuf_t))
    {
	    return NULL;
    }
    else
    {
        membuf_t* buf = (membuf_t*)data;
        buf->data     = (void*)((char*)data + sizeof(membuf_t));
        buf->clear    = membuf_destack_clear;
        buf->resize   = membuf_destack_resize;
        buf->extract  = membuf_destack_extract;
        buf->collect  = membuf_destack_collect;

        destackbuf_t* userdata = (destackbuf_t*)buf->data;
        userdata->tail = 0;
        userdata->head = 0;
        userdata->size = size - sizeof(membuf_t) - sizeof(destackbuf_t);
        
        return buf;
    }
}

static void  membuf_pool_clear(void* userdata)
{
    (void)userdata;
}

static void* membuf_pool_resize(void* userdata, void* ptr, int size, int align)
{
    (void)userdata;
    (void)ptr;
    (void)size;
    (void)align;

    return NULL;
}

static void* membuf_pool_extract(void* userdata, int size, int align)
{
    poolbuf_t* buf = (poolbuf_t*)userdata;

    if (buf->block == size && buf->first)
    {
        void* mem  = (char*)buf->first + sizeof(void*);
        buf->first = *((void**)buf->first);
        buf->count = buf->count - 1;
        return mem;
    }
    else
    {
	    return NULL;
    }
}

static void  membuf_pool_collect(void* userdata, void* ptr)
{
    poolbuf_t* buf = (poolbuf_t*)userdata;
    
    intptr_t address = (intptr_t)ptr - (intptr_t)userdata;
    if (address > 0 && address < buf->size)
    {
        void** block = (void**)((char*)ptr - sizeof(void*));
        *block = buf->first;
        buf->first = block;
        buf->count = buf->count + 1;
    }
}

membuf_t* membuf_pool(void* data, int size, int block)
{
    if (size <= sizeof(membuf_t) + sizeof(poolbuf_t))
    {
	    return NULL;
    }
    else
    {
        membuf_t* buf = (membuf_t*)data;
        buf->data     = (void*)((char*)data + sizeof(membuf_t));
        buf->clear    = membuf_pool_clear;
        buf->resize   = membuf_pool_resize;
        buf->extract  = membuf_pool_extract;
        buf->collect  = membuf_pool_collect;

        poolbuf_t* userdata = (poolbuf_t*)buf->data;
        userdata->size  = size - sizeof(membuf_t) - sizeof(poolbuf_t);
        userdata->count = userdata->size / (sizeof(void*) + block);
        userdata->block = block;
        userdata->first = (char*)userdata + sizeof(poolbuf_t);

        int i;
        char** ptr = (char**)userdata->first;
        for (i = 0; i < userdata->count - 1; i++)
        {
            ptr = (char**)(*ptr = (char*)ptr + (sizeof(void*) + block));
        }
        *ptr = NULL;
        
        return buf;
    }
}
#endif
