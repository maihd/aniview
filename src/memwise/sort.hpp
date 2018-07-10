#ifndef __MEMWISE_SORT_H__
#define __MEMWISE_SORT_H__

#ifndef __cplusplus
# error "Generics sorting functions is not support non-C++ languages"
#endif

namespace sort
{
	/**
	 * Swap value between two variables
	 */
	template <typename type_t>
	inline void swap(type_t& a, type_t& b)
	{
		type_t c = a;
		a = b;
		b = c;
	}

	/** 
	 * Default compare function
	 */
	template <typename type_t>
	inline int defcmp(const type_t& a, const type_t& b)
	{
		return a - b;
	}

	/**
	 * Insertion sorting with custom compare function
	 */
	template <typename type_t, typename func_t>
	void insert(type_t* array, int count, const func_t& compare)
	{
		/* Nothing for sorting */
		if (count <= 1) return;

		type_t* lo = array;
		type_t* hi = array + count - 1;
		while (lo < hi)
		{
			type_t* max = lo;
			for (type_t* p = lo + 1; p <= hi; p++)
			{
				if (compare(*max, *p) < 0)
				{
					max = p;
				}
			}
			swap(*max, *hi--);
		}
	}

	/**
	 * Insertion sorting
	 */
	template <typename type_t>
	inline void insert(type_t* array, int count)
	{
		insert(array, count, defcmp<type_t>);
	}

	/**
	 * Insertion sorting
	 */
	template <typename type_t, int count>
	inline void insert(type_t array[count])
	{
		insert(array, count);
	}

	/**
	 * Insertion sorting with custom compare function
	 */
	template <typename type_t, int count, typename func_t>
	inline void insert(type_t array[count], const func_t& compare)
	{
		insert(array, count, compare);
	}

	/**
	 * Quick sort with custom compare functions
	 */
	template <typename type_t, typename func_t>
	void quick(type_t* array, int count, const func_t& compare)
	{
		if (count <= 2) return;

		const int maxlevels = 128;
		int lo[maxlevels] = { 0 };
		int hi[maxlevels] = { count - 1 };
		int lv = 0;

		while (lv > -1)
		{
			int i = lo[lv];
			int j = hi[lv];
			if ((j - i) >= 4 && (lv < maxlevels - 1))
			{
				int idx = (i + j) / 2;

				type_t pivot = array[idx];
				swap(array[idx], array[j--]);

				do
				{
					while (compare(array[i], pivot) < 0 && ++i >= j) break;
					while (compare(array[j], pivot) && --j <= i) break;

					swap(array[i], array[j]);
					i += 1;
					j -= 1;
				} while (i < j);

				while (compare(array[i], pivot) <= 0 && i < hi[lv]) i++;
				while (compare(array[j], pivot) >= 0 && j > lo[lv]) j--;

				swap(array[i], array[hi[lv]]);

				lo[lv + 1] = i;
				hi[lv + 1] = hi[lv];
				hi[lv] = j;
				lv += 1;
			}
			else
			{
				for (; j > i; j--)
				{
					int m = i;
					for (int k = i + 1; k <= j; k++)
					{
						if (compare(array[k], array[m]) > 0)
						{
							m = k;
						}
					}
					swap(array[m], array[j]);
				}
				
				lv -= 1;
			}
		}
	}

	/**
	 * Quick sort
	 */
	template <typename type_t>
	inline void quick(type_t* array, int count)
	{
		quick(array, count, defcmp<type_t>);
	}

	/**
	 * 
	 */
	template <typename type_t, int count>
	inline void quick(type_t array[count])
	{
		quick(array, count);
	}

	/**
	 * 
	 */
	template <typename type_t, int count, typename func_t>
	inline void quick(type_t array[count], const func_t& compare)
	{
		quick(array, count, compare);
	}

	/**
	 * 
	 */
	template <typename type_t, typename func_t>
	void shell(type_t* array, int count, const func_t& compare)
	{
		if (count < 2) return;

		int gap, i, j;
		for (gap = count / 2; gap > 0; gap /= 2)
		{
			for (i = gap; i < count; i++)
			{
				for (j = i - gap; j >= 0 && compare(array[j], array[j + gap]) > 0; j -= gap)
				{
					swap(array[i], array[j]);
				}
			}
		}
	}

	/**
	 * 
	 */
	template <typename type_t>
	inline void shell(type_t* array, int count)
	{
		shell(array, count, defcmp<type_t>);
	}
}

#endif /* __SORT_HPP__ */
