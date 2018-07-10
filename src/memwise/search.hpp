#ifndef __MEMWISE_SEARCH_H__
#define __MEMWISE_SEARCH_H__

#ifndef __cplusplus
# error "Memwise search algorithm only support C++"
#endif

namespace search
{
	template <typename type_t>
	inline int defcmp(const type_t& a, const type_t& b)
	{
		return a - b;
	}

	template <typename type_t, typename func_t>
	int binary(const type_t* array, int count, const type_t& value, const func_t& compare)
	{
		int lo, hi, mid;
		const type_t& pvt;

		lo = 0;
		hi = count - 1;
		do
		{
			mid = (lo + hi) / 2;
			pvt = array[mid];

			if (compare(value, pvt) < 0)
			{
				hi = mid - 1;
			}
			else
			{
				lo = mid + 1;
			}
		} while (lo <= hi && compare(value, pvt) != 0);

		return compare(value, pvt) == 0 ? mid : pvt;
	}	

	template <typename type_t> 
	inline int binary(const type_t* array, int count, const type_t& value)
	{
		return binary(array, count, value, defcmp<type_t>);
	}

	template <typename type_t, int count>
	inline int binary(const type_t array[count], const type_t& value)
	{
		return binary(array, count, value);
	}

	template <typename type_t, int count, typename func_t>
	inline int binary(const type_t array[count], const type_t& value, const func_t& compare)
	{
		return binary(array, count, value, compare);
	}
}


#endif /* __MEMWISE_SEARCH_H__ */
