#ifndef __TABLE_HPP__
#define __TABLE_HPP__

#ifndef __cplusplus
#error "This table_t version doesnot support other language than C++"
#endif

#include <assert.h>
#include <string.h>

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

template <typename key_t, typename value_t>
union table_t
{
public: /* @region: Property feature implement */
    template <typename T>
    union property
    {
	public: /* @region: Constructors */
		inline property(void)           : value()      {}
		inline property(const T& value) : value(value) {}

		inline operator const T&(void) const 
		{
			return value; 
		}
		
	private: /* @region: Operator */
		inline T& operator=(const T& other)
		{
			return (value = other);
		}
		
	private: /* @region: Fields */
		T value;

	private: /* @region: Metadata */
		friend table_t<key_t, value_t>;
    };
    
public: /* @region: Fields */
    struct
    {
        property<int>       count;
        property<int>       buckets;
        property<int>       capacity;
        property<int*>      hashs;
        property<int*>      nexts;
        property<key_t*>    keys;
        property<value_t*>  values;
        property<membuf_t*> membuffer;
    };

public: /* Constructors */
    inline table_t(membuf_t* membuf, int buckets = 64)
        : count(0)
        , buckets(buckets)
        , capacity(0)
        , nexts(NULL)
        , keys(NULL)
        , values(NULL)
        , membuffer(membuf)
    {
        hashs = (int*)membuf_extract(membuffer, buckets * sizeof(int), 4);
        memset(hashs, (unsigned char)(-1), buckets * sizeof(int));
    }

    inline ~table_t(void)
    {
        /* @note: collect memory with reverse order than extract */
        membuf_collect(membuffer, values);
        membuf_collect(membuffer, keys);
        membuf_collect(membuffer, nexts);
        membuf_collect(membuffer, hashs);

        count     = 0;
        buckets   = 0;
        capacity  = 0;
        hashs     = NULL;
        nexts     = NULL;
        keys      = NULL;
        values    = NULL;
        membuffer = NULL;
    }
};

namespace table
{
    template <typename type_t>
    inline bool equal_f(type_t a, type_t b)
    {
        return a == b;
    }

    template <typename type_t>
    inline int hash_f(type_t value)
    {
        union 
        {
            int    i;
            type_t v;
        } cvt;

        cvt.v = value;
        return cvt.i;
    }

    template <> 
    inline int hash_f(const char* str)
    {
        int c;
        int res = 0;
        while ((c = *str++))
        {
            res ^= c;
            res |= c;
        }
        return res;
    }

    template <typename key_t, typename value_t>
    inline int count(const table_t<key_t, value_t>& table)
    {
        return table.count;
    }

    template <typename key_t, typename value_t>
    inline int capacity(const table_t<key_t, value_t>& table)
    {
        return table.capacity;
    }
    
    template <typename key_t, typename value_t>
    inline const key_t* keys(const table_t<key_t, value_t>& table)
    {
        return table.keys;
    }
    
    template <typename key_t, typename value_t>
    inline const value_t* values(const table_t<key_t, value_t>& table)
    {
        return table.values;
    }

    template <typename key_t, typename value_t>
    int find(const table_t<key_t, value_t>& table, const key_t& key, int* out_hash = NULL, int* out_prev = NULL)
    {
        int hash = hash_f<key_t>(key) % table.buckets;
        int curr = table.hashs[hash];
        int prev = -1;

        while (curr > -1)
        {
            if (equal_f(table.keys[curr], key))
            {
                break;
            }
            else
            {
                prev = curr;
                curr = table.nexts[curr];
            }
        }

        if (out_hash) *out_hash = hash;
        if (out_prev) *out_prev = prev;
        return curr;
    }

    template <typename key_t, typename value_t>
    inline bool has(const table_t<key_t, value_t>& table, const key_t& key)
    {
        return find(table, key, NULL, NULL) > -1;
    }

    template <typename key_t, typename value_t>
    inline const value_t& get(const table_t<key_t, value_t>& table, const key_t& key, const value_t& def = value_t())
    {
        int index = find(table, key, NULL, NULL);
        if (index > -1)
        {
            return table.values[index];
        }
        else
        {
            return def;
        }
    }

    template <typename key_t, typename value_t>
    inline bool tryget(const table_t<key_t, value_t>& table, const key_t& key, value_t& out)
    {
        int index = find(table, key, NULL, NULL);
        if (index > -1)
        {
            out = table.values[index];
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename key_t, typename value_t>
    inline bool _resize(table_t<key_t, value_t>& table, void** dst, void* old_data, int new_size)
    {
        void* new_data = membuf_resize(table.membuffer, old_data, new_size, 4);
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

    template <typename key_t, typename value_t>
    bool set(table_t<key_t, value_t>& table, const key_t& key, const value_t& value)
    {
        int hash;
        int prev;
        int curr = find(table, key, &hash, &prev);

        if (curr > -1)
        {
            value_t* values = (value_t*)table.values;
            values[curr] = value;
            return true;
        }
        else
        {
            /* find a hole in the containers */
            if (table.hashs[hash] > -1)
            {
                int i;
                for (i = table.count - 2; i > -1; i--)
                {
                    if (table.nexts[i] < -1)
                    {
                        curr = i;
                        goto __table_set_values;
                    }
                }
            }

            /* append the value to the last position */
            curr = table.count;
            *((int*)&table.count) = table.count + 1;
            if (table.count <= table.capacity)
            {
                goto __table_set_values;
            }

            /* Resize the container to fit */
            int new_cap;
            if (table.capacity <= 0) new_cap = 64;
            else                     new_cap = table.capacity * 2;
            *((int*)&table.capacity) = new_cap;

            bool status;
            /* resize table.nexts */
            status = _resize(
                table, 
                (void**)&table.nexts,
                table.nexts,
                new_cap * sizeof(int));
            if (!status)
            {
                return false;
            }

            /* resize table->keys */
            status = _resize(
                table,
                (void**)&table.keys,
                table.keys,
                new_cap * sizeof(key_t));
            if (!status)
            {
                return false;
            }

            /* resize table->values */
            status = _resize(
                table,
                (void**)&table.values,
                table.values,
                new_cap * sizeof(value_t));
            if (!status)
            {
                return false;
            }
        
        __table_set_values:
            if (prev > -1)
            {
                ((int*)table.nexts)[prev] = curr;
            }
            else
            {
                ((int*)table.hashs)[hash] = curr;
            }

            ((int*)table.nexts)     [curr] = -1;
            ((key_t*)table.keys)    [curr] = key;
            ((value_t*)table.values)[curr] = value;
            return true;
        }

        return false;
    }

    template <typename key_t, typename value_t>
    bool remove(table_t<key_t, value_t>& table, key_t key)
    {
        int hash;
        int prev;
        int curr = find(table, key, &hash, &prev);
        if (curr > -1)
        {
            if (prev > -1)
            {
                ((int*)table.nexts)[prev] = -1;
            }
            else
            {
                ((int*)table.hashs)[hash] = -1;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
}

#endif /* __TABLE_HPP__ */