#include "sal.h"

#define NS_INADDRSZ 4   /* IPv4 T_A */
#define NS_IN6ADDRSZ    16  /* IPv6 T_AAAA */
#define NS_INT16SZ  2   /* #/bytes of data in a u_int16_t */

#ifndef SPRINTF
#define SPRINTF(x) ((int32)sal_sprintf x)
#endif

#define X_INF       0x7ffffff0
#define X_STORE(c) \
    {    \
        if (bp < be){    \
            * bp = (c); }  \
        bp ++;       \
    }

#define CHAR_BIT 8
#define STACK_SIZE	(CHAR_BIT * sizeof(size_t))
#define PUSH(low, high)	((top->lo = (low)), (top->hi = (high)), ++top)
#define	POP(low, high)	(--top, (low = top->lo), (high = top->hi))
#define	STACK_NOT_EMPTY	(stack < top)
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct {
	char *lo;
	char *hi;
} stack_node;

#define SWAP(a, b, size)		      \
  do {					      \
      size_t __size = (size);		      \
      char *__a = (a), *__b = (b);	      \
      do {				      \
	  char __tmp = *__a;		      \
	  *__a++ = *__b;		      \
	  *__b++ = __tmp;		      \
	} while (--__size > 0);		      \
    } while (0)


STATIC const char*
_inet_ntop4(const uint8* src, int8* dst, uint32 size)
{
    int8 tmp[sizeof "255.255.255.255"];

    if (SPRINTF((tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3])) > size)
    {
        return (NULL);
    }

    return sal_strcpy(dst, tmp);
}

STATIC const char*
_inet_ntop6(const uint8* src, int8* dst, uint32 size)
{
    int8 tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], * tp;

    struct { int base, len;
    } best, cur;
    uint32 words[NS_IN6ADDRSZ / NS_INT16SZ];
    int32 i;

    sal_memset(&best, 0, sizeof(best));
    sal_memset(&cur, 0, sizeof(cur));

    /*
     * Preprocess:
     *	Copy the input (bytewise) array into a wordwise array.
     *	Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    sal_memset(words, '\0', sizeof words);

    for (i = 0; i < NS_IN6ADDRSZ; i += 2)
    {
        words[i / 2] = (src[i] << 8) | src[i + 1];
    }

    best.base = -1;
    cur.base = -1;

    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        if (words[i] == 0)
        {
            if (cur.base == -1)
            {
                cur.base = i, cur.len = 1;
            }
            else
            {
                cur.len++;
            }
        }
        else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                {
                    best = cur;
                }

                cur.base = -1;
            }
        }
    }

    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
        {
            best = cur;
        }
    }

    if (best.base != -1 && best.len < 2)
    {
        best.base = -1;
    }

    /*
     * Format the result.
     */
    tp = tmp;

    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base &&
            i < (best.base + best.len))
        {
            if (i == best.base)
            {
                *tp++ = ':';
            }

            continue;
        }

        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
        {
            *tp++ = ':';
        }

        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 &&
            (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
        {
            if (!_inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp)))
            {
                return (NULL);
            }

            tp += sal_strlen(tp);
            break;
        }

        tp += SPRINTF((tp, "%x", words[i]));
    }

    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) ==
        (NS_IN6ADDRSZ / NS_INT16SZ))
    {
        *tp++ = ':';
    }

    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((uint32)(tp - tmp) > size)
    {
        return (NULL);
    }

    return sal_strcpy(dst, tmp);
}

STATIC int
_inet_pton4(const int8* src, uint8* dst)
{
    int32 saw_digit, octets, ch;
    uint8 tmp[NS_INADDRSZ], * tp;
    uint32 new_value = 0;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;

    while ((ch = *src++) != '\0')
    {

        if (ch >= '0' && ch <= '9')
        {
            new_value = *tp * 10 + (ch - '0');

            if (saw_digit && *tp == 0)
            {
                return (0);
            }

            if (new_value > 255)
            {
                return (0);
            }

            *tp = new_value;
            if (!saw_digit)
            {
                if (++octets > 4)
                {
                    return (0);
                }

                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
            {
                return (0);
            }

            *++tp = 0;
            saw_digit = 0;
        }
        else
        {
            return (0);
        }
    }

    if (octets < 4)
    {
        return (0);
    }

    sal_memcpy(dst, tmp, NS_INADDRSZ);
    return (1);
}

STATIC int
_inet_pton6(const int8* src, uint8* dst)
{
    static const int8 xdigits[] = "0123456789abcdef";
    uint8 tmp[NS_IN6ADDRSZ], * tp, * endp, * colonp;
    const int8* curtok;
    int32 ch, saw_xdigit;
    uint32 val;

    tp = sal_memset(tmp, '\0', NS_IN6ADDRSZ);
    endp = tp + NS_IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
    {
        if (*++src != ':')
        {
            return (0);
        }
    }

    curtok = src;
    saw_xdigit = 0;
    val = 0;

    while ((ch = sal_tolower(*src++)) != '\0')
    {
        const int8* pch;

        pch = sal_strchr(xdigits, ch);
        if (pch != NULL)
        {
            val <<= 4;
            val |= ((uintptr)pch - (uintptr)xdigits);
            if (val > 0xffff)
            {
                return (0);
            }

            saw_xdigit = 1;
            continue;
        }

        if (ch == ':')
        {
            curtok = src;
            if (!saw_xdigit)
            {
                if (colonp)
                {
                    return (0);
                }

                colonp = tp;
                continue;
            }
            else if (*src == '\0')
            {
                return (0);
            }

            if (tp + NS_INT16SZ > endp)
            {
                return (0);
            }

            *tp++ = (uint8)(val >> 8) & 0xff;
            *tp++ = (uint8)val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }

        if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
            _inet_pton4(curtok, tp) > 0)
        {
            tp += NS_INADDRSZ;
            saw_xdigit = 0;
            break;  /* '\0' was seen by inet_pton4(). */
        }

        return (0);
    }

    if (saw_xdigit)
    {
        if (tp + NS_INT16SZ > endp)
        {
            return (0);
        }

        *tp++ = (uint8)(val >> 8) & 0xff;
        *tp++ = (uint8)val & 0xff;
    }

    if (colonp != NULL)
    {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        if (tp == endp)
        {
            return (0);
        }

        for (i = 1; i <= n; i++)
        {
            endp[-i] = colonp[n - i];
            colonp[n - i] = 0;
        }

        tp = endp;
    }

    if (tp != endp)
    {
        return (0);
    }

    sal_memcpy(dst, tmp, NS_IN6ADDRSZ);
    return (1);
}

const char*
ctc_sal_inet_ntop(int32 af, void* src, char* dst, uint32 size)
{
    switch (af)
    {
    case AF_INET:
        return (_inet_ntop4((uint8*)src, dst, size));

    case AF_INET6:
        return (_inet_ntop6((uint8*)src, dst, size));

    default:
        return (NULL);
    }

    /* NOTREACHED */
}

int32
ctc_sal_inet_pton(int32 af, const char* src, void* dst)
{
    switch (af)
    {
    case AF_INET:
        return (_inet_pton4((int8*)src, dst));

    case AF_INET6:
        return (_inet_pton6((int8*)src, dst));

    default:
        return (-1);
    }

    /* NOTREACHED */
}

int32
ctc_sal_strcasecmp(const char* s1, const char* s2)
{
    const char* p1 = (const char*)s1;
    const char* p2 = (const char*)s2;
    int32 ret;

    if (p1 == p2)
    {
        return 0;
    }

    while ((ret = tolower(*p1) - tolower(*p2)) == 0)
    {
        if (*p1 == '\0')
        {
            break;
        }

        p1++;
        p2++;
    }

    return ret;
}

int32
ctc_sal_strncasecmp(const char* s1, const char* s2, int32 n)
{
    uint8 c1, c2;

    if (n == 0)
    {
        return 0;
    }

    do
    {
        c1 = tolower(*s1);
        c2 = tolower(*s2);
        s1++;
        s2++;
    }
    while ((--n) && (c1 == c2) && (c1 != 0));

    return (c1 - c2);
}

#if 0
int8*
sal_strdup(const int8* s)
{
    int8 len = sal_strlen(s);
    int8* rc = malloc(len + 1);

    if (rc != NULL)
    {
        sal_strcpy(rc, s);
    }

    return rc;
}

#endif

void
ctc_sal_qsort(void *const pbase, size_t total_elems, size_t size,
      int (*cmp)(const void*, const void*))
{
	char *base_ptr = (char *)pbase;

	const size_t max_thresh = MAX_THRESH * size;

	/* Avoid lossage with unsigned arithmetic below.  */
	if (total_elems == 0) {
		return;
	}

	if (total_elems > MAX_THRESH) {
		char *lo = base_ptr;
		char *hi = &lo[size * (total_elems - 1)];
		stack_node stack[STACK_SIZE];
		stack_node *top = stack + 1;

		while (STACK_NOT_EMPTY) {
			char *left_ptr;
			char *right_ptr;

			/* Select median value from among LO, MID, and
			   HI. Rearrange LO and HI so the three values
			   are sorted. This lowers the probability of
			   picking a pathological pivot value and
			   skips a comparison for both the LEFT_PTR
			   and RIGHT_PTR in the while loops. */

			char *mid = lo + size * ((hi - lo) / size >> 1);

			if ((*cmp)((void*)mid, (void*)lo) < 0)
				SWAP(mid, lo, size);
			if ((*cmp)((void*)hi, (void*)mid) < 0)
				SWAP(mid, hi, size);
			else
				goto jump_over;
			if ((*cmp)((void*)mid, (void*)lo) < 0)
				SWAP(mid, lo, size);
		jump_over:

			left_ptr = lo + size;
			right_ptr = hi - size;

			/* Here's the famous ``collapse the walls''
			   section of quicksort.  Gotta like those
			   tight inner loops!  They are the main
			   reason that this algorithm runs much faster
			   than others. */
			do {
				while ((*cmp)((void*)left_ptr, (void*)mid) < 0)
					left_ptr += size;

				while ((*cmp)((void*)mid, (void*)right_ptr) < 0)
					right_ptr -= size;

				if (left_ptr < right_ptr) {
					SWAP(left_ptr, right_ptr, size);
					if (mid == left_ptr)
						mid = right_ptr;
					else if (mid == right_ptr)
						mid = left_ptr;
					left_ptr += size;
					right_ptr -= size;
				} else if (left_ptr == right_ptr) {
					left_ptr += size;
					right_ptr -= size;
					break;
				}
			}
			while (left_ptr <= right_ptr);

			/* Set up pointers for next iteration.  First
			   determine whether left and right partitions
			   are below the threshold size.  If so,
			   ignore one or both.  Otherwise, push the
			   larger partition's bounds on the stack and
			   continue sorting the smaller one. */

			if ((size_t) (right_ptr - lo) <= max_thresh) {
				if ((size_t) (hi - left_ptr) <= max_thresh)
					/* Ignore both small partitions. */
					POP(lo, hi);
				else
					/* Ignore small left partition. */
					lo = left_ptr;
			} else if ((size_t) (hi - left_ptr) <= max_thresh)
				/* Ignore small right partition. */
				hi = right_ptr;
			else if ((right_ptr - lo) > (hi - left_ptr)) {
				/* Push larger left partition indices. */
				PUSH(lo, right_ptr);
				lo = left_ptr;
			} else {
				/* Push larger right partition indices. */
				PUSH(left_ptr, hi);
				hi = right_ptr;
			}
		}
	}

	/* Once the BASE_PTR array is partially sorted by quicksort
	   the rest is completely sorted using insertion sort, since
	   this is efficient for partitions below MAX_THRESH
	   size. BASE_PTR points to the beginning of the array to
	   sort, and END_PTR points at the very last element in the
	   array (*not* one beyond it!). */

	{
		char *end_ptr = &base_ptr[size * (total_elems - 1)];
		char *tmp_ptr = base_ptr;
		char *thresh = min(end_ptr, base_ptr + max_thresh);
		char *run_ptr;

		/* Find smallest element in first threshold and place
		   it at the array's beginning.  This is the smallest
		   array element, and the operation speeds up
		   insertion sort's inner loop. */

		for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
			if ((*cmp)((void*)run_ptr, (void*)tmp_ptr) < 0)
				tmp_ptr = run_ptr;

		if (tmp_ptr != base_ptr)
			SWAP(tmp_ptr, base_ptr, size);

		/* Insertion sort, running from left-hand-side up to
		 * right-hand-side.  */

		run_ptr = base_ptr + size;
		while ((run_ptr += size) <= end_ptr) {
			tmp_ptr = run_ptr - size;
			while ((*cmp)((void*)run_ptr, (void*)tmp_ptr) < 0)
				tmp_ptr -= size;

			tmp_ptr += size;
			if (tmp_ptr != run_ptr) {
				char *trav;

				trav = run_ptr + size;
				while (--trav >= run_ptr) {
					char c = *trav;
					char *hi, *lo;

					for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
						*hi = *lo;
					*hi = c;
				}
			}
		}
	}
}

