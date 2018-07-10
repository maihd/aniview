#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

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

#include <string.h>
#include <assert.h>

template <typename type_t>
union queue_t 
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
		friend queue_t<type_t>;
    };

public: /* @region: Fields */
    struct
    {
        property<int>       tail;
        property<int>       head;
        property<int>       capacity;
        property<type_t*>   elements;
        property<membuf_t*> membuffer;
    };

public: /* @region: Constructors */
    inline queue_t(membuf_t* membuf)
        : tail(0)
        , head(0)
        , capacity(0)
        , elements(0)
        , membuffer(membuf)
        {}

    inline ~queue_t(void)
    {
        membuf_collect(membuffer, elements);

        tail      = 0;
        head      = 0;
        capacity  = 0;
        elements  = 0;
        membuffer = 0;
    }
};

namespace queue
{
    template <typename type_t>
    inline int count(const queue_t<type_t>& queue)
    {
        return queue.tail - queue.head;
    }

    template <typename type_t>
    inline int capacity(const queue_t<type_t>& queue)
    {
        return queue.capacity;
    }

    template <typename type_t>
    inline const type_t* elements(const queue_t<type_t>& queue)
    {
        return queue.elements;
    }

    template <typename type_t>
    inline const type_t& peek(const queue_t<type_t>& queue)
    {
        assert(queue::count(queue) > 0);
        return queue.elements[queue.head];   
    }

    template <typename type_t>
    inline bool enque(queue_t<type_t>& queue, const type_t& value)
    {
        *((int*)&queue.tail) = queue.tail + 1;
        if (queue.tail > queue.capacity)
        {
            if (queue.head > 0)
            {
                memcpy(queue.elements, queue.elements + queue.head, queue::count(queue) * sizeof(type_t));

                *((int*)&queue.tail) = queue::count(queue);
                *((int*)&queue.head) = 0;
            }
            else
            {
                int capacity;
                if (queue.capacity > 0) capacity = queue.capacity * 2;
                else                    capacity = 64;

                void* elements = membuf_resize(queue.membuffer, queue.elements, capacity * sizeof(type_t), 4);

                if (elements) 
                {
                    *((int*)&queue.capacity)   = capacity;
                    *((void**)&queue.elements) = elements;
                }
                else
                {
                    return false;
                }
            }
        }
        
        ((type_t*)queue.elements)[queue.tail - 1] = value;
        return true;
    }

    template <typename type_t>
    inline const type_t& deque(queue_t<type_t>& queue)
    {
        assert(queue::count(queue) > 0);
        int index = queue.head;
        *((int*)&queue.head) = queue.head + 1;
        return queue.elements[index];
    }
}

#endif /* __QUEUE_HPP__ */