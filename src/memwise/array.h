/* @warning: no header guards, no #pragma once */
/*           'cause using like an template     */

#if defined(ARRAY_CLEAN)
#undef ARRAY_CLEAN

#undef array_t
#undef array_init         
#undef array_free         

#undef array_ensure       
#undef array_resize

#undef array_get          
#undef array_pop          
#undef array_shift        

#undef array_set          
#undef array_insert       
#undef array_push         
#undef array_unshift      

#undef array_has          
#undef array_find         
#undef array_rfind        

#undef array_erase       
#undef array_remove      
#undef array_right_remove

#undef array_element_t
#undef ARRAY_ELEMENT_EQUAL

#undef ARRAY_API
#undef ARRAY_IMPL
#undef ARRAY_PREFIX

#elif defined(ARRAY_CONFIG)
#undef ARRAY_CONFIG

#if defined(ARRAY__SHOULD_CLEAN)
#define ARRAY_CLEAN
#include __FILE__
#endif

#undef ARRAY__DEFINE
#undef ARRAY__CONCAT
#undef ARRAY__CONCAT_IN
#define ARRAY__DEFINE(name)            ARRAY__CONCAT(ARRAY_PREFIX, name)
#define ARRAY__CONCAT(prefix, name)    ARRAY__CONCAT_IN(prefix, name)
#define ARRAY__CONCAT_IN(prefix, name) prefix ## name        

#define array_t            ARRAY__DEFINE(_t)
#define array_init         ARRAY__DEFINE(_init)
#define array_free         ARRAY__DEFINE(_free)

#define array_ensure       ARRAY__DEFINE(_ensure)
#define array_resize       ARRAY__DEFINE(_resize)

#define array_get          ARRAY__DEFINE(_get)
#define array_pop          ARRAY__DEFINE(_pop)
#define array_shift        ARRAY__DEFINE(_shift)

#define array_set          ARRAY__DEFINE(_set)
#define array_insert       ARRAY__DEFINE(_insert)
#define array_push         ARRAY__DEFINE(_push)
#define array_unshift      ARRAY__DEFINE(_unshift)

#define array_has          ARRAY__DEFINE(_has)
#define array_find         ARRAY__DEFINE(_find)
#define array_rfind        ARRAY__DEFINE(_rfind)

#define array_erase        ARRAY__DEFINE(_erase)
#define array_remove       ARRAY__DEFINE(_remove)
#define array_right_remove ARRAY__DEFINE(_right_remove)

#define ARRAY__SHOULD_CLEAN

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

#ifndef ARRAY_API
#define ARRAY_API
#endif

#ifndef array_element_t
#error									\
    "array_element_t is undefined: "					\
    "You should define your array element's type."			
#endif

typedef struct
{
    int              count;
    int              capacity;
    array_element_t* elements;
    membuf_t*        membuffer;
} array_t;

ARRAY_API void array_init(array_t* array, membuf_t* membuffer);
ARRAY_API void array_free(array_t* array);
ARRAY_API bool array_ensure(array_t* array, int capacity);
ARRAY_API bool array_resize(array_t* array, int capacity);

ARRAY_API array_element_t array_get(const array_t* array, int index);
ARRAY_API array_element_t array_pop(array_t* array);
ARRAY_API array_element_t array_shift(array_t* array);

ARRAY_API bool array_set(array_t* array, int index, array_element_t element);
ARRAY_API bool array_insert(array_t* array, int index, array_element_t element);

ARRAY_API bool array_push(array_t* array, array_element_t element);
ARRAY_API bool array_unshift(array_t* array, array_element_t element);

ARRAY_API bool array_has(const array_t* array, array_element_t element);
ARRAY_API int  array_find(const array_t* array, array_element_t element);
ARRAY_API int  array_rfind(const array_t* array, array_element_t element);

ARRAY_API bool array_erase(array_t* array, int index);
ARRAY_API bool array_remove(array_t* array, array_element_t element);
ARRAY_API bool array_right_remove(array_t* array, array_element_t element);

#ifdef ARRAY_IMPL

#ifndef array_assert
#include <assert.h>
#define  array_assert(a, e) assert(a != NULL); assert(e)
#endif

#include <string.h>

#ifndef ARRAY_ELEMENT_EQUAL
#define ARRAY_ELEMENT_EQUAL(a, b) ((a) == (b))
#endif

void array_init(array_t* array, membuf_t* membuffer)
{
    array->count     = 0;
    array->capacity  = 0;
    array->elements  = 0;
    array->membuffer = membuffer;
}

void array_free(array_t* array)
{
    membuf_collect(array->membuffer, array->elements);

    array->count    = 0;
    array->capacity = 0;
    array->elements = 0;
}

bool array_ensure(array_t* array, int capacity)
{
    if (array->capacity < capacity)
    {
	printf("array_ensure: resize now\n");
	return array_resize(array, capacity);
    }
    else
    {
	return true;
    }
}

bool array_resize(array_t* array, int capacity)
{
    /* calculate new array capacity to fit expected capacity */
    if (array->capacity <= 0) array->capacity = capacity;
    while (array->capacity < capacity)
    {
	array->capacity *= 2;
    }
    
    const int new_size = array->capacity * sizeof(array_element_t);
    void* new_elements = membuf_resize(array->membuffer,
				       array->elements, new_size, 4);

    if (!new_elements)
    {
	return false;
    }
    else
    {
	array->elements = (array_element_t*)new_elements;
	return true;
    }
}

array_element_t array_get(const array_t* array, int index)
{
    array_assert(array, index > -1 && index < array->count);
    return array->elements[index];
}

bool array_set(array_t* array, int index, array_element_t element)
{
    int new_count;
    if (index >= array->count)
    {
        new_count = index + 1;
    }
    else
    {
	new_count = array->count;
    }

    if (array_ensure(array, new_count))
    {
	array->count = new_count;
	array->elements[index] = element;
	return true;
    }
    else
    {
	return false;
    }
}

bool array_push(array_t* array, array_element_t element)
{
    return array_set(array, array->count, element);
}

array_element_t array_pop(array_t* array)
{
    array_assert(array, array->count > 0);
    return array->elements[--array->count];
}

int array_find(const array_t* array, array_element_t element)
{
    int index, count;
    for (index = 0, count = array->count; index < count; index++)
    {
	if (ARRAY_ELEMENT_EQUAL(array->elements[index], element))
	{
	    return index;
	}
    }

    return -1;
}

int array_rfind(const array_t* array, array_element_t element)
{
    int index;
    for (index = array->count - 1; index > -1; index--)
    {
	if (ARRAY_ELEMENT_EQUAL(array->elements[index], element))
	{
	    return index;
	}
    }

    return -1;
}

bool array_has(const array_t* array, array_element_t element)
{
    return array_find(array, element) > -1;
}

bool array_insert(array_t* array, int index, array_element_t element)
{
    if (index >= array->count)
    {
	return array_set(array, index, element);
    }
    else
    {
	if (array_ensure(array, array->count + 1))
	{
	    int   copy_cnt = array->count - index - 1;
	    void* copy_src = array->elements + index;
	    void* copy_dst = array->elements + index + 1;
	    memmove(copy_dst, copy_src, copy_cnt * sizeof(array_element_t));
	    
	    array->elements[index] = element; 
	}
	else
	{
	    return false;
	}
    }
}

bool array_unshift(array_t* array, array_element_t element)
{
    return array_insert(array, 0, element);
}

array_element_t array_shift(array_t* array)
{
    array_assert(array, array->count > 0);
    
    array_element_t res = array->elements[0];

    array_erase(array, 0);
    
    return res;
}

bool array_erase(array_t* array, int index)
{
    if (index < 0 || index >= array->count)
    {
	return false;
    }
    else
    {
	array->count--;
	
	if (index < array->count)
	{
	    int   copy_cnt = array->count - index;
	    void* copy_src = array->elements + index + 1;
	    void* copy_dst = array->elements + index;
	    memcpy(copy_dst, copy_src, copy_cnt * sizeof(array_element_t));
	}

	return true;
    }
}

bool array_remove(array_t* array, array_element_t element)
{
    int index = array_find(array, element);

    if (index > -1)
    {
	(void)array_erase(array, index); /* ignore return value */
	return true;
    }
    else
    {
	return false;
    }
}

bool array_right_remove(array_t* array, array_element_t element)
{
    int index = array_rfind(array, element);

    if (index > -1)
    {
	(void)array_erase(array, index); /* ignore return value */
	return true;
    }
    else
    {
	return false;
    }
}
#endif

#endif /* ARRAY_CONFIG */
