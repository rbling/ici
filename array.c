/*
 * array.c - implementation of the intrinsic array type. See also array.h.
 */
#define ICI_CORE
#include "ptr.h"
#include "exec.h"
#include "op.h"
#include "int.h"
#include "buf.h"
#include "primes.h"

/*
 * Function to do the hard work for the macro ici_stk_push_chk().
 * See array.h. This reallocates the array buffer.
 */
int
ici_grow_stack(array_t *a, ptrdiff_t n)
{
    register object_t   **e;
    ptrdiff_t           oldz;

    /*
     * Users of arrays as stacks are supposed to know the origin and
     * history of the array. So we just assert it is not an atom.
     */
    assert((objof(a)->o_flags & O_ATOM) == 0);
    assert(a->a_bot == a->a_base);
    /*
     * We don't use realloc to ensure that memory exhaustion is
     * cleanly recovereable.
     */
    if ((oldz = a->a_limit - a->a_base) * 3 / 2 < a->a_limit - a->a_base + n)
        n += (a->a_limit - a->a_base) + 10;
    else
        n = (a->a_limit - a->a_base) * 3 / 2;
    if ((e = (object_t **)ici_nalloc(n * sizeof(object_t *))) == NULL)
        return 1;
    memcpy((char *)e, (char *)a->a_base,
        (a->a_limit - a->a_base) * sizeof(object_t *));
    a->a_top = e + (a->a_top - a->a_base);
    ici_nfree((char *)a->a_base, oldz * sizeof(object_t *));
    a->a_base = e;
    a->a_bot = e;
    a->a_limit = e + n;
    return 0;
}

/*
 * Function to do the hard work for the macro ici_stack_probe(). See array.h.
 */
int
ici_fault_stack(array_t *a, ptrdiff_t i)
{
    /*
     * Users of arrays as stacks are supposed to know the origin and
     * history of the array. So we just assert it is not an atom.
     */
    assert((objof(a)->o_flags & O_ATOM) == 0);
    ++i;
    i -= a->a_top - a->a_bot;
    if (ici_stk_push_chk(a, i))
        return 1;
    while (--i >= 0)
        *a->a_top++ = objof(&o_null);
    return 0;
}

/*
 * Return the number of elements in the array.
 */
ptrdiff_t
ici_array_nels(array_t *a)
{
    if (a->a_top >= a->a_bot)
        return a->a_top - a->a_bot;
    return (a->a_top - a->a_base) + (a->a_limit - a->a_bot);
}

/*
 * Return a pointer to the first object in the array at index i, and
 * reduce *np to the number of contiguous elements available from that
 * point onwards. np may be NULL if not required.
 *
 * This is the commonest routine for finding an element at a given
 * index in an array. It only works for valid indexes.
 */
static object_t **
ici_array_span(array_t *a, int i, ptrdiff_t *np)
{
    object_t            **e;
    ptrdiff_t           n;

    if (a->a_bot <= a->a_top)
    {
        e = &a->a_bot[i];
        n = a->a_top - e;
    }
    else if (a->a_bot + i < a->a_limit)
    {
        e = &a->a_bot[i];
        n = a->a_limit - e;
    }
    else
    {
        i -= a->a_limit - a->a_bot;
        e = &a->a_base[i];
        n = a->a_top - e;
    }
    assert(n >= 0);
    if (np != NULL && *np > n)
        *np = n;
    return e;
}

/*
 * Copy n objects of the given array, starting at index start, to b.
 * The span must cover existing elements of the array.
 */
void
ici_array_gather(object_t **b, array_t *a, ptrdiff_t start, ptrdiff_t n)
{
    object_t            **e;
    ptrdiff_t           i;
    ptrdiff_t           m;

    for (i = start; i < start + n; i += m, b += m)
    {
        m = n - (i - start);
        e = ici_array_span(a, i, &m);
        memcpy(b, e, m * sizeof(object_t *));
    }
}

/*
 * Grow the given array to have a larger allocation. Also ensure that
 * on return there is at least one empty slot after a_top and before
 * a_bot.
 */
static int
ici_array_grow(array_t *a)
{
    ptrdiff_t           nel;    /* Number of elements. */
    ptrdiff_t           n;      /* Old allocation count. */
    ptrdiff_t           m;      /* New allocation count. */
    object_t            **e;    /* New allocation. */

    n = a->a_limit - a->a_base;
    if ((m = n * 3 / 2) < 8)
        m = 8;
    if ((e = (object_t **)ici_nalloc(m * sizeof(object_t *))) == NULL)
        return 1;
    nel = ici_array_nels(a);
    ici_array_gather(e + 1, a, 0, nel);
    ici_nfree(a->a_base, n * sizeof(object_t *));
    a->a_base = e;
    a->a_limit = e + m;
    a->a_bot = e + 1;
    a->a_top = e + 1 + nel;
    return 0;
}

/*
 * Push the object o onto the end of the array a. This is the general
 * case that works for any array whether it is a stack or a queue.
 * On return, o_top[-1] is the object pushed. Return 1 on error, else 0.
 */
int
ici_array_push(array_t *a, object_t *o)
{
    if (objof(a)->o_flags & O_ATOM)
    {
        ici_error = "attempt to push atomic array";
        return 1;
    }
    if (a->a_bot <= a->a_top)
    {
        /*
         *   ..........oooooooooooooooooooX............
         *   ^a_base   ^a_bot             ^a_top       ^a_limit
         */
        if (a->a_top == a->a_limit)
        {
            if (a->a_base + 1 >= a->a_bot || a->a_bot == a->a_limit)
            {
                if (ici_array_grow(a))
                    return 1;
            }
            else
            {
                a->a_top = a->a_base; /* Wrap from limit to base. */
            }
        }
    }
    else
    {
        /*
         *   ooooooooooooooX................ooooooooooo
         *   ^a_base       ^a_top           ^a_bot     ^a_limit
         */
        if (a->a_top + 1 >= a->a_bot)
        {
            if (ici_array_grow(a))
                return 1;
        }
    }
    assert(a->a_base <= a->a_top);
    assert(a->a_top <= a->a_limit);
    *a->a_top++ = o;
    return 0;
}

/*
 * Push the object o onto the front of the array a. Return 1 on failure,
 * else 0.
 */
int
ici_array_rpush(array_t *a, object_t *o)
{
    if (objof(a)->o_flags & O_ATOM)
    {
        ici_error = "attempt to rpush atomic array";
        return 1;
    }
    if (a->a_bot <= a->a_top)
    {
        /*
         *   ..........oooooooooooooooooooX............
         *   ^a_base   ^a_bot             ^a_top       ^a_limit
         */
        if (a->a_bot == a->a_base)
        {
            if (a->a_top >= a->a_limit - 1 || a->a_top == a->a_bot)
            {
                if (ici_array_grow(a))
                    return 1;
            }
            else
            {
                a->a_bot = a->a_limit; /* Wrap from base to limit. */
            }
        }
    }
    else
    {
        /*
         *   ooooooooooooooX................ooooooooooo
         *   ^a_base       ^a_top           ^a_bot     ^a_limit
         */
        if (a->a_top >= a->a_bot - 1)
        {
            if (ici_array_grow(a))
                return 1;
        }
    }
    assert(a->a_base <= a->a_bot);
    assert(a->a_bot <= a->a_limit);
    *--a->a_bot = o;
    return 0;
}

/*
 * Pop and return the top of the given array, or ICI NULL if it is empty.
 */
object_t *
ici_array_pop(array_t *a)
{
    if (objof(a)->o_flags & O_ATOM)
    {
        ici_error = "attempt to pop atomic array";
        return NULL;
    }
    if (a->a_bot <= a->a_top)
    {
        /*
         *   ..........oooooooooooooooooooX............
         *   ^a_base   ^a_bot             ^a_top       ^a_limit
         */
        if (a->a_bot < a->a_top)
            return *--a->a_top;
    }
    else
    {
        /*
         *   ooooooooooooooX................ooooooooooo
         *   ^a_base       ^a_top           ^a_bot     ^a_limit
         */
        if (a->a_top > a->a_base)
            return *--a->a_top;
        a->a_top = a->a_limit;
        if (a->a_top > a->a_bot)
            return *--a->a_top;
    }
    assert(a->a_base <= a->a_top);
    assert(a->a_top <= a->a_limit);
    return objof(&o_null);
}

/*
 * Pop and return the front of the given array, or ICI NULL if it is empty.
 */
object_t *
ici_array_rpop(array_t *a)
{
    if (objof(a)->o_flags & O_ATOM)
    {
        ici_error = "attempt to rpop atomic array";
        return NULL;
    }
    if (a->a_bot <= a->a_top)
    {
        /*
         *   ..........oooooooooooooooooooX............
         *   ^a_base   ^a_bot             ^a_top       ^a_limit
         */
        if (a->a_bot < a->a_top)
            return *a->a_bot++;
    }
    else
    {
        /*
         *   ooooooooooooooX................ooooooooooo
         *   ^a_base       ^a_top           ^a_bot     ^a_limit
         */
        if (a->a_bot < a->a_limit)
            return *a->a_bot++;
        a->a_bot = a->a_base;
        if (a->a_bot < a->a_top)
            return *a->a_bot++;
    }
    assert(a->a_base <= a->a_bot);
    assert(a->a_bot <= a->a_limit);
    return objof(&o_null);
}

/*
 * Return a pointer to the slot that does, or should contain the index i.
 * This will grow and NULL fill the array as necessary. Only positive i.
 * Returns NULL on error, usual conventions.
 */
static object_t **
ici_array_find_slot(array_t *a, ptrdiff_t i)
{
    ptrdiff_t           n;

    n = ici_array_nels(a);
    if (i < n)
    {
        /*
         * Within the range of exisiting objects. Just use
         * ici_array_span to find the pointer to it.
         */
        return ici_array_span(a, i, NULL);
    }
    i = i - n + 1; /* Number of elements we need to add. */
    while (--i >= 0)
    {
        if (ici_array_push(a, objof(&o_null)))
            return NULL;
    }
    return &a->a_top[-1];
}

/*
 * Return the element at the given index, or ICI NULL if out of range.
 */
object_t *
ici_array_get(array_t *a, ptrdiff_t i)
{
    ptrdiff_t           n;

    n = ici_array_nels(a);
    if (i >= 0 && i < n)
        return *ici_array_span(a, i, NULL);
    return objof(&o_null);
}

/*
 * Return a new array.  It will have room for at least n elements.
 * If n is 0 an internal default will be used. Returned array has
 * ref count 1. Returns NULL on failure, usual conventions.
 */
array_t *
ici_array_new(ptrdiff_t n)
{
    register array_t    *a;

    if ((a = ici_talloc(array_t)) == NULL)
        return NULL;
    objof(a)->o_tcode = TC_ARRAY;
    assert(ici_typeof(a) == &ici_array_type);
    objof(a)->o_flags = 0;
    objof(a)->o_nrefs = 1;
    a->a_base = NULL;
    a->a_top = NULL;
    a->a_limit = NULL;
    a->a_bot = NULL;
    if (n == 0)
        n = 4;
    if ((a->a_base = (object_t **)ici_nalloc(n * sizeof(object_t *))) == NULL)
    {
        ici_tfree(a, array_t);
        return NULL;
    }
    a->a_top = a->a_base;
    a->a_bot = a->a_base;
    a->a_limit = a->a_base + n;
    rego(a);
    return a;
}

/*
 * Mark this and referenced unmarked objects, return memory costs.
 * See comments on t_mark() in object.h.
 */
static unsigned long
mark_array(object_t *o)
{
    object_t            **e;
    unsigned long       mem;

    o->o_flags |= O_MARK;
    if (arrayof(o)->a_base == NULL)
        return sizeof(array_t);
    mem = sizeof(array_t)
        + (arrayof(o)->a_limit - arrayof(o)->a_base) * sizeof(object_t *);
    if (arrayof(o)->a_bot <= arrayof(o)->a_top)
    {
        for (e = arrayof(o)->a_bot; e < arrayof(o)->a_top; ++e)
            mem += ici_mark(*e);
    }
    else
    {
        for (e = arrayof(o)->a_base; e < arrayof(o)->a_top; ++e)
            mem += ici_mark(*e);
        for (e = arrayof(o)->a_bot; e < arrayof(o)->a_limit; ++e)
            mem += ici_mark(*e);
    }
    return mem;
}

/*
 * Free this object and associated memory (but not other objects).
 * See the comments on t_free() in object.h.
 */
static void
free_array(object_t *o)
{
    if (arrayof(o)->a_base != NULL)
    {
        ici_nfree
        (
            arrayof(o)->a_base,
            (arrayof(o)->a_limit - arrayof(o)->a_base) * sizeof(object_t *)
        );
    }
    ici_tfree(o, array_t);
}

/*
 * Returns 0 if these objects are equal, else non-zero.
 * See the comments on t_cmp() in object.h.
 */
static int
cmp_array(object_t *o1, object_t *o2)
{
    ptrdiff_t           i;
    ptrdiff_t           n1;
    ptrdiff_t           n2;
    object_t            **e1;
    object_t            **e2;

    if (o1 == o2)
        return 0;
    n1 = ici_array_nels(arrayof(o1));
    n2 = ici_array_nels(arrayof(o2));
    if (n1 != n2)
        return 1;
    for (i = 0; i < n1; i += n2)
    {
        n2 = n1;
        e1 = ici_array_span(arrayof(o1), i, &n2);
        e2 = ici_array_span(arrayof(o2), i, &n2);
        if (memcmp(e1, e2, n2 * sizeof(object_t *)))
            return 1;
    }
    return 0;
}

/*
 * Return a copy of the given object, or NULL on error.
 * See the comment on t_copy() in object.h.
 */
static object_t *
copy_array(object_t *o)
{
    array_t             *na;
    ptrdiff_t           n;

    n = ici_array_nels(arrayof(o));
    if ((na = ici_array_new(n)) == NULL)
        return NULL;
    ici_array_gather(na->a_top, arrayof(o), 0, n);
    na->a_top += n;
    return objof(na);
}

/*
 * Return a hash sensitive to the value of the object.
 * See the comment on t_hash() in object.h
 */
static unsigned long
hash_array(object_t *o)
{
    unsigned long       h;
    object_t            **e;
    ptrdiff_t           n;
    ptrdiff_t           m;
    ptrdiff_t           i;

    h = ARRAY_PRIME;
    n = ici_array_nels(arrayof(o));
    for (i = 0; i < n; )
    {
        m = n;
        e = ici_array_span(arrayof(o), i, &m);
        i += m;
        while (--m >= 0)
        {
            h += ICI_PTR_HASH_BITS(*e++);
            h >>= 1;
        }
    }
    return h;
}

/*
 * Assign to key k of the object o the value v. Return 1 on error, else 0.
 * See the comment on t_assign() in object.h.
 *
 * The key k must be a positive integer. The array will attempt to grow
 * to accomodate the new index as necessary.
 */
static int
assign_array(object_t *o, object_t *k, object_t *v)
{
    long        i;
    object_t    **e;

    if (!isint(k))
        return ici_assign_fail(o, k, v);
    i = intof(k)->i_value;
    if (i < 0)
    {
        ici_error = "attempt to assign to negative array index";
        return 1;
    }
    if ((e = ici_array_find_slot(arrayof(o), i)) == NULL)
        return 1;
    *e = v;
    return 0;
}

/*
 * Return the object at key k of the obejct o, or NULL on error.
 * See the comment on t_fetch in object.h.
 *
 * The key k must be an integer. Reading non existing keys results in
 * an NULL value (not an error).
 */
static object_t *
fetch_array(object_t *o, object_t *k)
{
    if (!isint(k))
        return ici_fetch_fail(o, k);
    return ici_array_get(arrayof(o), intof(k)->i_value);
}

/*
 * obj => array 0 (the array contains the obj)
 */
int
ici_op_mklvalue()
{
    array_t     *a;

    if ((a = ici_array_new(1)) == NULL)
        return 1;
    *a->a_top++ = ici_os.a_top[-1];
    ici_os.a_top[-1] = objof(a);
    *ici_os.a_top++ = objof(ici_zero);
    ici_decref(a);
    --ici_xs.a_top;
    return 0;
}

type_t  ici_array_type =
{
    mark_array,
    free_array,
    hash_array,
    cmp_array,
    copy_array,
    assign_array,
    fetch_array,
    "array"
};

op_t    o_mklvalue      = {OBJ(TC_OP), ici_op_mklvalue};
