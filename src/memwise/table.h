#if defined(TABLE_CLEAN)
#undef TABLE_CLEAN

#undef table_t       
#undef table_init    
#undef table_free    
#undef table_has     
#undef table_get     
#undef table_try_get 
#undef table_set     
#undef table_remove  

#undef table_key_t   
#undef table_value_t
#undef TABLE_KEY_EQUAL

#undef TABLE_HASHS
#undef TABLE_HASHFUNC
#undef TABLE_CAPACITY

#undef TABLE_API
#undef TABLE_IMPL
#undef TABLE_PREFIX

#elif defined(TABLE_CONFIG)
#undef TABLE_CONFIG

#if defined(TABLE__SHOULD_CLEAN)
#undef  TABLE__SHOULD_CLEAN
#define TABLE_CLEAN
#include __FILE__
#endif

#undef TABLE__DEFINE
#undef TABLE__CONCAT
#undef TABLE__CONCAT_IN
#define TABLE__DEFINE(name) TABLE__CONCAT(TABLE_PREFIX, name)
#define TABLE__CONCAT(prefix, name) TABLE__CONCAT_IN(prefix, name)
#define TABLE__CONCAT_IN(prefix, name) prefix ## name

#define table_t       TABLE__DEFINE(_t)
#define table_init    TABLE__DEFINE(_init)
#define table_free    TABLE__DEFINE(_free)
#define table_has     TABLE__DEFINE(_has)
#define table_get     TABLE__DEFINE(_get)
#define table_try_get TABLE__DEFINE(_try_get)
#define table_set     TABLE__DEFINE(_set)
#define table_remove  TABLE__DEFINE(_remove)

#define TABLE__SHOULD_CLEAN

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

#ifndef TABLE_API
#define TABLE_API
#endif

#ifndef TABLE_HASHS
#define TABLE_HASHS 64
#endif

typedef struct
{
    int            count;
    int            capacity;
    int            hashs[TABLE_HASHS];
    int*           nexts;
    table_key_t*   keys;
    table_value_t* values;
    membuf_t*      membuffer;
} table_t;

TABLE_API void table_init(table_t* table, membuf_t* membuffer);
TABLE_API void table_free(table_t* table);

TABLE_API bool table_has(const table_t* table, table_key_t key);
TABLE_API bool table_try_get(const table_t* table, table_key_t key, table_value_t* value);
TABLE_API table_value_t table_get(const table_t* table, table_key_t key, table_value_t defvalue);
TABLE_API bool table_set(table_t* table, table_key_t key, table_value_t value);
TABLE_API bool table_remove(table_t* table, table_key_t key);

#if 0 || TABLE_EXTENSIONS
TABLE_API table_key_t table_get_key(const table_t* table, table_value_t value, table_key_t defkey);
TABLE_API bool table_has_value(const table_t* table, table_value_t value);
TABLE_API bool table_remove_value(table_t* table, table_value_t value);
#endif

#ifdef TABLE_IMPL

#ifndef TABLE_CAPACITY 
#define TABLE_CAPACITY 64
#endif

#ifndef TABLE_KEY_EQUAL
#define TABLE_KEY_EQUAL(a, b) ((a) == (b))
#endif

#ifndef table_assert
# include <assert.h>
# define  table_assert(t, e) assert(t != NULL); assert(e)
# endif
#endif

#include <string.h>

typedef struct
{
    int hash;
    int curr;
    int prev;
} table__findres_t;

void table_init(table_t* table, membuf_t* membuffer)
{
    table->count     = 0;
    table->capacity  = 0;
    table->nexts     = 0;
    table->keys      = 0;
    table->values    = 0;
    table->membuffer = membuffer;

    int i;
    for (i = 0; i < TABLE_HASHS; i++)
    {
		table->hashs[i] = -1;
    }
}

void table_free(table_t* table)
{
    membuf_collect(table->membuffer, table->values);
    membuf_collect(table->membuffer, table->keys);
    membuf_collect(table->membuffer, table->nexts);

    table->count    = 0;
    table->capacity = 0;
    table->nexts    = 0;
    table->keys     = 0;
    table->values   = 0;
    
    int i;
    for (i = 0; i < TABLE_HASHS; i++)
    {
		table->hashs[i] = -1;
    }
}

static table__findres_t table__find(const table_t* table, table_key_t key)
{
    table__findres_t res;

    res.hash = TABLE_HASHFUNC(key) % TABLE_HASHS;
    res.curr = table->hashs[res.hash];
    res.prev = -1;

    while (res.curr > -1)
    {
		if (TABLE_KEY_EQUAL(key, table->keys[res.curr]))
		{
			break;
		}

		res.prev = res.curr;
		res.curr = table->nexts[res.curr];
    }

    return res;
}

bool table_has(const table_t* table, table_key_t key)
{
    table__findres_t fs = table__find(table, key);
    return fs.curr > -1;
}

bool table_try_get(const table_t* table, table_key_t key, table_value_t* value)
{
    table__findres_t fs = table__find(table, key);

    if (fs.curr > -1)
    {
		*value = table->values[fs.curr];
		return true;
    }
    else
    {
		return false;
    }
}

table_value_t table_get(const table_t* table, table_key_t key, table_value_t def)
{
    table__findres_t fs = table__find(table, key);

    if (fs.curr > -1)
    {
		return table->values[fs.curr];
    }
    else
    {
		return def;
    }
}

static bool table__resize(table_t* table, void** dst, int new_size, void* old_data, int old_size)
{
    void* new_data = membuf_resize(table->membuffer, old_data, new_size, 4);
    if (new_data)
    {
		*dst = new_data;
		return true;
    }
    else
    {
		return false;
    }
}

bool table_set(table_t* table, table_key_t key, table_value_t value)
{
    table__findres_t fs = table__find(table, key);

    if (fs.curr > -1)
    {
		table->values[fs.curr] = value;
		return true;
    }
    else
    {
		/* find a hole in the containers */
		if (table->hashs[fs.hash] > -1)
		{
			int i;
			for (i = table->count - 2; i > -1; i--)
			{
				if (table->nexts[i] < -1)
				{
					fs.curr = i;
					goto __table_set_values;
				}
			}
		}

		/* append the value to the last position */
		fs.curr = table->count++;
		if (table->count <= table->capacity)
		{
			goto __table_set_values;
		}

		/* Resize the container to fit */
		const int old_cap = table->capacity;

		if (table->capacity <= 0) table->capacity  = TABLE_CAPACITY;
		else                      table->capacity *= 2;
		const int new_cap = table->capacity;

		bool status;
		/* resize table->nexts */
		status = table__resize(
			table, 
			(void**)&table->nexts,
			new_cap * sizeof(int),
			table->nexts, old_cap * sizeof(int));
		if (!status)
		{
	    	return false;
		}

		/* resize table->keys */
		status = table__resize(
			table,
			(void**)&table->keys,
			new_cap * sizeof(table_key_t),
			table->keys, old_cap * sizeof(table_key_t));
		if (!status)
		{
			return false;
		}

		/* resize table->values */
		status = table__resize(
			table,
			(void**)&table->values,
			new_cap * sizeof(table_value_t),
			table->values, old_cap * sizeof(table_value_t));
		if (!status)
		{
			return false;
		}
	
    __table_set_values:
		if (fs.prev > -1)
		{
			table->nexts[fs.prev] = fs.curr;
		}
		else
		{
			table->hashs[fs.hash] = fs.curr;
		}

		table->nexts[fs.curr]  = -1;
		table->keys[fs.curr]   = key;
		table->values[fs.curr] = value;
		return true;
    }
}

bool table_remove(table_t* table, table_key_t key)
{
    table__findres_t fs = table__find(table, key);
    if (fs.curr > -1)
    {
		if (fs.prev > -1)
		{
			table->nexts[fs.prev] = -1;
		}
		else
		{
			table->hashs[fs.hash] = -1;
		}
		return true;
    }
    else
    {
		return false;
    }
}
#endif
