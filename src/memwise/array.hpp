#ifndef __ARRAY_HPP__
#define __ARRAY_HPP__

#ifndef __cplusplus
#error "This array version does not support other languages from C++"
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

template <typename type_t>
union array_t /* union instead of other type for make sure there is no vtable */
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
		friend array_t<type_t>;
    };
    
public: /* @region: Fields */
    struct
    {
		property<int>       count;
		property<int>       capacity;
		property<type_t*>   elements;
		property<membuf_t*> membuffer;
    };
    
public: /* @region: Constructors */
    inline array_t(membuf_t* membuf)
		: count(0)
		, capacity(0)
		, elements(0)
		, membuffer(membuf)
		{}
    
    inline ~array_t(void) 
	{
		membuf_collect(membuffer, elements);
		
		count     = 0;
		capacity  = 0;
		elements  = 0;
		membuffer = 0;
    }

public: /* @region: Operators */
    inline type_t& operator[](int index)
	{
		assert(index >= 0 && index < count);
		return elements[index];
    }
    
    inline const type_t& operator[](int index) const
	{
		assert(index >= 0 && index < count);
		return elements[index];
    }

    inline operator type_t*(void) 
	{
		return elements;
    }
    
    inline operator const type_t*(void) const 
	{
		return elements;
    }
};

namespace array
{
	template <typename type_t>
	inline int count(const array_t<type_t>& array)
	{
		return array.count;
	}

	template <typename type_t>
	inline int capacity(const array_t<type_t>& array)
	{
		return array.capacity;
	}

	template <typename type_t>
	inline const type_t* elements(const array_t<type_t>& array)
	{
		return array.elements;
	}

	template <typename type_t>
	inline void clear(array_t<type_t>& array)
	{
		*((int*)&array.count) = 0;
	}

    template <typename type_t>
    inline bool expand(array_t<type_t>& array, int capacity)
    {
		if (array.capacity < capacity)
		{
			int new_capacity = array.capacity > 0 ? (int)array.capacity : 64;
			while (new_capacity < capacity)
			{
				new_capacity *= 2;
			}

			*((int*)&array.capacity)   = new_capacity;
			*((void**)&array.elements) =
				membuf_resize(array.membuffer, array.elements, new_capacity * sizeof(type_t), 4);
			return array.elements != NULL;
		}
		else
		{
			return true;
		}
    }
    
    template <typename type_t>
    inline bool ensure(array_t<type_t>& array, int capacity)
    {
		if (array.capacity < capacity)
		{
			return expand(array, capacity);
		}
		else
		{
			return true;
		}
    }

    template <typename type_t>
    inline bool set(array_t<type_t>& array, int index, const type_t& value)
    {
		if (index >= array.count)
		{
			*((int*)&array.count) = index + 1;
		}
		
		if (ensure(array, array.count))
		{
			array[index] = value;
			return true;
		}
		else
		{
			return false;
		}
    }

	/**
	 * Add new slot to array
	 */
	template <typename type_t>
	inline type_t& add(array_t<type_t>& array)
	{
		if (ensure(array, array.count + 1))
		{
			*((int*)&array.count) = array.count + 1;
			return array.elements[array.count - 1];
		}
		else
		{
			assert(0 && "array::add: Out of memory");
			return *((type_t*)NULL); /* Will cause runtime error */ 
		}
	} 

    template <typename type_t>
    inline const type_t& get(const array_t<type_t>& array, int index)
    {
		assert(index >= 0 && index < array.count);
		return array.elements[index];
    }

    template <typename type_t>
    inline bool push(array_t<type_t>& array, const type_t& value)
    {
		return set(array, array.count, value);
    }

    template <typename type_t>
    inline const type_t& pop(array_t<type_t>& array)
    {
		assert(array.count > 0);
		*((int*)&array.count) = array.count - 1;
		return array.elements[array.count];
    }

	template <typename type_t>
	int find(const array_t<type_t>& array, const type_t& element)
	{
		int index, count;
		for (index = 0, count = array.count; index < count; index++)
		{
			if (array.elements[index] == element)
			{
				return index;
			}
		}

		return -1;
	}

	template <typename type_t>
	int rfind(const array_t<type_t>& array, const type_t& element)
	{
		int index;
		for (index = array.count - 1; index > -1; index--)
		{
			if (array.elements[index] == element)
			{
				return index;
			}
		}

		return -1;
	}

	template <typename type_t>
	inline bool has(const array_t<type_t>& array, const type_t& element)
	{
		return find(array, element) > -1;
	}

	template <typename type_t>
	bool insert(array_t<type_t>& array, int index, const type_t& element)
	{
		if (index >= array.count)
		{
			return set(array, index, element);
		}
		else
		{
			if (array_ensure(array, array.count + 1))
			{
				int   copy_cnt = array.count - index - 1;
				void* copy_src = array.elements + index;
				void* copy_dst = array.elements + index + 1;
				memmove(copy_dst, copy_src, copy_cnt * sizeof(type_t));
				
				array[index] = element; 
			}
			else
			{
				return false;
			}
		}
	}

	template <typename type_t>
	inline bool unshift(array_t<type_t>& array, const type_t& element)
	{
		return insert(array, 0, element);
	}

	template <typename type_t>
	inline type_t shift(array_t<type_t>& array)
	{
		assert(array.count > 0);
		
		type_t res = array.elements[0];

		erase(array, 0);
		
		return res;
	}

	template <typename type_t>
	inline bool erase(array_t<type_t>& array, int index)
	{
		if (index < 0 || index >= array.count)
		{
			return false;
		}
		else
		{
			*((int*)&array.count) = array.count - 1;
		
			if (index < array.count)
			{
				int   copy_cnt = array.count - index;
				void* copy_src = array.elements + index + 1;
				void* copy_dst = array.elements + index;
				memcpy(copy_dst, copy_src, copy_cnt * sizeof(type_t));
			}

			return true;
		}
	}

	template <typename type_t>
	inline bool remove(array_t<type_t>& array, const type_t& element)
	{
		int index = find(array, element);

		if (index > -1)
		{
			(void)erase(array, index); /* ignore return value */
			return true;
		}
		else
		{
			return false;
		}
	}

	template <typename type_t>
	inline bool right_remove(array_t<type_t>& array, const type_t& element)
	{
		int index = rfind(array, element);

		if (index > -1)
		{
			(void)erase(array, index); /* ignore return value */
			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif
