#ifndef __TEMPO_H__
#define __TEMPO_H__

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#include <malloc.h>
#undef  alloca
#define alloca _alloca
#else
#include <alloca.h>
#endif

#define __MCONCAT(x, y) x ## y
#define _MCONCAT(x, y) __MCONCAT(x, y)
#define _MGENSYM(x)    _MCONCAT(x, __LINE__)

#define MEMWISE_ERROR_INDEX  0
#define MEMWISE_ERROR_EMPTY  1
#define MEMWISE_ERROR_MEMORY 2

#define MEMWISE_ERROR(func, code)  func ": " MEMWISE_ERROR_STRING(code)
#define MEMWISE_ERROR_STRING(code) _MCONCAT(MEMWISE_ERROR_STRING, code)
#define MEMWISE_ERROR_STRING0      "Index out of range"
#define MEMWISE_ERROR_STRING1      "No elements"
#define MEMWISE_ERROR_STRING2      "Out of memory"

#define MEMWISE_ASSERT(exp, f, e)  assert(exp && MEMWISE_ERROR(f, e))

/* ------------------------------------------------- */
/* ----------------- MEMORY BUFFER ----------------- */
/* ------------------------------------------------- */

/* NO MEMORY BUFFER NEEDED, tempo_t doesnot use dynamic memory allocation */

/* --------------------------------------------------- */
/* ------------------- CONTAINERS -------------------- */
/* --------------------------------------------------- */

#define MEMWISE_DEFCMP(a, b) ((a) == (b))

/**************************
 * Temporary array
 **************************/
#define memwise__tempo_t(type_t)    \
    int     count;		            \
    int     capacity;               \
    type_t* elements	    

#define memwise__tempo_init(t, c)                                                   \
    do {                                                                            \
        (t).count    = 0;                                                           \
        (t).capacity = c;                                                           \
        *((void**)&(t).elements) = alloca(c * sizeof((t).elements[0]));             \
    } while (0)

#define memwise__tempo_set(t, i, e, res)				\
    do {                                                \
        if (i < 0) break;                               \
        if (i >= (t).count) {						    \
            if (i + 1 <= (t).capacity) {                \
                (t).count = i + 1;						\
            } else {                                    \
                res = 0;                                \
                break;                                  \
            }                                           \
        }                                               \
        (t).elements[i] = e;                            \
        res = 1;                                        \
    } while (0)

#define memwise__tempo_get(t, i)              \
    (MEMWISE_ASSERT(i > -1 && i < (t).count,  \
		    "tempo_get",                      \
		    MEMWISE_ERROR_INDEX),             \
     (t).elements[i])

#define memwise__tempo_ref(t, i)                               \
    (MEMWISE_ASSERT(i > -1 && i < capacity(t).capacity,        \
		    "tempo_ref",                                       \
		    MEMWISE_ERROR_INDEX),                              \
     (t).elements + i)

#define memwise__tempo_pop(t)                \
    (MEMWISE_ASSERT((t).count > 0,           \
		    "tempo_pop",                     \
		    MEMWISE_ERROR_EMPTY),            \
     (t).elements[--(t).count])

#define memwise__tempo_push(t, e)       \
    do {                                \
	    int _i_ = (t).count;            \
	    tempo_set(t, _i_, e);           \
    } while (0)

#define memwise__tempo_find(t, e, i)                 \
    memwise__tempo_find_cmp(t, e, i, MEMWISE_DEFCMP)

#define memwise__tempo_find_cmp(t, e, i, cmp)     \
    do {                                          \
        int _j_; int _m_ = (t).count;             \
        for (_j_ = 0; _j_ < _m_; _j_++)	{         \
            if (cmp(e, (t).elements[_j_])) {      \
                (i) = _j_; break;                 \
            }                                     \
        }                                         \
    } while (0)

#define memwise__tempo_rfind(t, e, i)                 \
    memwise__tempo_rfind_cmp(t, e, i, MEMWISE_DEFCMP)

#define memwise__tempo_rfind_cmp(t, e, i, cmp)      \
    do {                                            \
        int _j_; int _m_ = (t).count;               \
        for (_j_ = _m_ - 1; _j_ > 0; _j_--)	{       \
            if (cmp(e, (t).elements[_j_])) {        \
                (i) = _j_; break;                   \
            }                                       \
        }                                           \
    } while (0)

#define memwise__tempo_erase(t, i)                      \
    do {                                                \
        MEMWISE_ASSERT(!(i < 0 || i >= (t).count),      \
                "tempo_erase", MEMWISE_ERROR_INDEX);    \
        int _i_; int _n_ = (t).count;                   \
        for (_i_ = i; _i_ < _n_; _i_++) {               \
            (t).elements[_i_] = (t).elements[_i_ + 1];  \
        }                                               \
    } while (0)

#define memwise__tempo_remove(t, e, res)	            \
    memwise__tempo_remove_cmp(t, e, res, MEMWISE_DEFCMP)

#define memwise__tempo_remove_cmp(t, e, res, cmp)   \
    do {						                    \
        int _k_;					                \
        memwise__tempo_find_cmp(t, e, _k_, cmp);	\
        memwise__tempo_erase(t, _k_);			    \
        res = _k_ > -1;                             \
    } while (0)


#define memwise__tempo_insert(t, i, e, res)                        \
    do {                                                           \
        if ((t).count < (t).capacity) {                            \
            res = 0;                                               \
            break;                                                 \
        }                                                          \
        int _i_; int _n_ = (t).count++, _e_ = i;                   \
        for (_i_ = _n_; _i_ < _e_; _i_--)                          \
            (t).elements[_i_] = (t).elements[_i_ - 1];             \
        (t).elements[_e_] = e;                                     \
        res = 1;                                                   \
    } while (0)


#define memwise__tempo__foreach(t, type_t, v, i, n, elm)   \
    type_t v; type_t* elm = (t).elements;                  \
    int i = 0; int n = (t).count;                          \
    for (v = elm[i]; i < n; v = elm[++i])

#define tempo_foreach(t, type_t, v)                 \
    memwise__tempo__foreach(t, type_t, v,           \
			    _MGENSYM(_i_),                      \
			    _MGENSYM(_n_),                      \
			    _MGENSYM(_a_))


#if defined(MEMWISE_C_VERSION) || !defined(__cplusplus)
/* BEGIN OF C VERSIONS */


#define MEMWISE_DEFCMP(a, b) ((a) == (b))

/**************************
 * Temporary array
 **************************/
#define tempo_t(type_t, c) struct { memwise__tempo_(type_t, c); }     

#define tempo_init(t) memwise__tempo_init(t)

#define tempo_capacity(t) memwise__tempo_capacity(t)

#define tempo_set(t, i, e, res) memwise__tempo_set(t, i, e, res)

#define tempo_get(t, i) memwise__tempo_get(t, i)

#define tempo_ref(t, i) memwise__tempo_ref(t, i)

#define tempo_pop(t) memwise__tempo_pop(t)

#define tempo_push(t, e) memwise__tempo_push(t, e)

#define tempo_find(t, e, i) memwise__tempo_find(t, e, i)

#define tempo_find_cmp(t, e, i, cmp) memwise__tempo_find_cmp(t, e, i, cmp)

#define tempo_erase(t, i) memwise__tempo_erase(t, i) 

#define tempo_remove(t, e, res) memwise__tempo_remove(t, e, res)

#define tempo_remove_cmp(t, e, res, cmp) memwise__tempo_remove_cmd(t, e, res, cmp)

#define tempo_insert(t, i, e, res) memwise__tempo_insert(t, i, e)

#else

/**
 * Temporary array
 */
template <typename type_t>
union tempo_t
{
public: /* @region: Fields */
    struct
    {
        memwise__tempo_t(type_t);
    };

public: /* @region: Constructors */
    inline tempo_t(int capacity)
    {
        memwise__tempo_init(*this, capacity);
    }

public: /* @region: Operators */
    inline type_t& operator[](int index)
    {
	    return elements[index];
    }

    inline const type_t& operator[](int index) const
    {
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

namespace tempo
{
    template <typename type_t>
    inline bool set(tempo_t<type_t>& tempo, int index, const type_t& value)
    {
        bool res;
        memwise__tempo_set(tempo, index, value, res);
        return res;
    }

    template <typename type_t>
    inline const type_t& get(const tempo_t<type_t>& tempo, int index)
    {
        return memwise__tempo_get(tempo, index);
    }

    template <typename type_t>
    inline type_t* ref(tempo_t<type_t>& tempo, int index)
    {
        return memwise__tempo_ref(tempo, index);
    }

    template <typename type_t>
    inline const type_t* ref(const tempo_t<type_t>& tempo, int index)
    {
        return memwise__tempo_ref(tempo, index);
    }

    template <typename type_t>
    inline const type_t& pop(tempo_t<type_t>& tempo)
    {
        return memwise__tempo_pop(tempo);
    }

    template <typename type_t>
    inline bool push(tempo_t<type_t>& tempo, const type_t& value)
    {
        return set(tempo, tempo.count, value);
    }

    template <typename type_t>
    int find(const tempo_t<type_t>& tempo, const type_t& value)
    {
        int res;
        memwise__tempo_find(tempo, value, res);
        return res;
    }

    template <typename type_t>
    int rfind(const tempo_t<type_t>& tempo, const type_t& value)
    {
        int res;
        memwise__tempo_rfind(tempo, value, res);
        return res;
    }

    template <typename type_t>
    inline void erase(tempo_t<type_t>& tempo, int index)
    {
        memwise__tempo_erase(tempo, index);
    }

    template <typename type_t>
    inline bool remove(tempo_t<type_t>& tempo, const type_t& value)
    {
        bool res;
        memwise__tempo_remove(tempo, value, res);
        return res;
    }


    template <typename type_t>
    inline bool insert(tempo_t<type_t>& tempo, int index, const type_t& value)
    {
        bool res;
        memwise__tempo_insert(tempo, index, value, res);
        return res;
    }
}
#endif /* MEMWISE_C_VERSIONS */

#endif /* __TEMPO_H__ */
