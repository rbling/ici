#define ICI_CORE
#include "exec.h"
#include "ptr.h"
#include "struct.h"
#include "int.h"
#include "op.h"
#include "buf.h"
#include "primes.h"
#include "cfunc.h"

/*
 * Mark this and referenced unmarked objects, return memory costs.
 * See comments on t_mark() in object.h.
 */
static unsigned long
mark_ptr(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof(ptr_t) + ici_mark(ptrof(o)->p_aggr) + ici_mark(ptrof(o)->p_key);
}

/*
 * Returns 0 if these objects are equal, else non-zero.
 * See the comments on t_cmp() in object.h.
 */
static int
cmp_ptr(object_t *o1, object_t *o2)
{
    return ptrof(o1)->p_aggr != ptrof(o2)->p_aggr
        || ptrof(o1)->p_key != ptrof(o2)->p_key;
}

/*
 * Return a hash sensitive to the value of the object.
 * See the comment on t_hash() in object.h
 */
static unsigned long
hash_ptr(object_t *o)
{
    return (unsigned long)ptrof(o)->p_aggr * PTR_PRIME_0
        + (unsigned long)ptrof(o)->p_key * PTR_PRIME_1;
}

/*
 * Return the object at key k of the obejct o, or NULL on error.
 * See the comment on t_fetch in object.h.
 *
 * Fetching a "sub element" of a pointer is the operation of pointer indexing.
 * It works for any pointer whoes key into its aggregate is an integer, and
 * of course the key must be an integer too.  The final key is the sum of the
 * two keys.  But if the key is zero, just do *ptr.
 */
static object_t *
fetch_ptr(object_t *o, object_t *k)
{
    if (k == objof(ici_zero))
        return ici_fetch(ptrof(o)->p_aggr, ptrof(o)->p_key);
    if (!isint(k) || !isint(ptrof(o)->p_key))
        return ici_fetch_fail(o, k);
    if (ptrof(o)->p_key == objof(ici_zero))
        ici_incref(k);
    else if ((k = objof(ici_int_new(intof(k)->i_value + intof(ptrof(o)->p_key)->i_value)))
            == NULL)
        return NULL;
    o = ici_fetch(ptrof(o)->p_aggr, k);
    ici_decref(k);
    return o;
}

/*
 * Assign to key k of the object o the value v. Return 1 on error, else 0.
 * See the comment on t_assign() in object.h.
 *
 * See above comment.
 */
static int
assign_ptr(object_t *o, object_t *k, object_t *v)
{
    if (k == objof(ici_zero))
        return ici_assign(ptrof(o)->p_aggr, ptrof(o)->p_key, v);
    if (!isint(k) || !isint(ptrof(o)->p_key))
        return ici_assign_fail(o, k, v);
    if (ptrof(o)->p_key == objof(ici_zero))
        ici_incref(k);
    else if ((k = objof(ici_int_new(intof(k)->i_value + intof(ptrof(o)->p_key)->i_value)))
            == NULL)
        return 1;
    if (ici_assign(ptrof(o)->p_aggr, k, v))
    {
        ici_decref(k);
        return 1;
    }
    ici_decref(k);
    return 0;
}

static int
call_ptr(object_t *o, object_t *subject)
{
    object_t    *f;

    f = ici_fetch(ptrof(o)->p_aggr, ptrof(o)->p_key);
    if (ici_typeof(f)->t_call == NULL)
    {
        char    n1[30];

        sprintf(buf, "attempt to call a ptr pointing to %s", objname(n1, o));
        ici_error = buf;
        return 1;
    }
    /*
     * Replace ourselves on the operand stack with 'self' (our aggr) and
     * push on the new object being called.
     */
    ici_os.a_top[-2] = ptrof(o)->p_aggr;
    if ((ici_os.a_top[-1] = objof(ici_int_new(NARGS() + 1))) == NULL)
        return 1;
    ici_decref(ici_os.a_top[-1]);
    if (ici_stk_push_chk(&ici_os, 1))
        return 1;
    *ici_os.a_top++ = f;
    ici_xs.a_top[-1] = objof(&o_call);
    /*
     * Then behave as if the target had been called. Should this do the
     * debug hooks? Assume not for now.
     */
    return (*ici_typeof(f)->t_call)(f, NULL);
}

ptr_t *
ici_ptr_new(object_t *a, object_t *k)
{
    register ptr_t      *p;

    if ((p = ici_talloc(ptr_t)) == NULL)
        return NULL;
    objof(p)->o_tcode = TC_PTR;
    assert(ici_typeof(p) == &ptr_type);
    objof(p)->o_flags = 0;
    objof(p)->o_nrefs = 1;
    rego(p);
    p->p_aggr = a;
    p->p_key = k;
    return ptrof(ici_atom(objof(p), 1));
}

/*
 * Free this object and associated memory (but not other objects).
 * See the comments on t_free() in object.h.
 */
static void
free_ptr(object_t *o)
{
    ici_tfree(o, ptr_t);
}

/*
 * aggr key => ptr
 */
int
ici_op_mkptr()
{
    register object_t   *o;

    if ((o = objof(ici_ptr_new(ici_os.a_top[-2], ici_os.a_top[-1]))) == NULL)
        return 1;
    ici_os.a_top[-2] = o;
    ici_decref(o);
    --ici_os.a_top;
    --ici_xs.a_top;
    return 0;
}

/*
 * ptr => aggr key
 */
int
ici_op_openptr()
{
    register ptr_t      *p;
    char                n[30];

    if (!isptr(objof(p = ptrof(ici_os.a_top[-1]))))
    {
        sprintf(buf, "pointer required, but %s given", objname(n, ici_os.a_top[-1]));
        ici_error = buf;
        return 1;
    }
    ici_os.a_top[-1] = p->p_aggr;
    *ici_os.a_top++ = p->p_key;
    --ici_xs.a_top;
    return 0;
}

/*
 * ptr => obj
 */
int
ici_op_fetch()
{
    register ptr_t      *p;
    register object_t   *o;
    char                n[30];

    if (!isptr(objof(p = ptrof(ici_os.a_top[-1]))))
    {
        sprintf(buf, "pointer required, but %s given", objname(n, ici_os.a_top[-1]));
        ici_error = buf;
        return 1;
    }
    if ((o = ici_fetch(p->p_aggr, p->p_key)) == NULL)
        return 1;
    ici_os.a_top[-1] = o;
    --ici_xs.a_top;
    return 0;
}

type_t  ptr_type =
{
    mark_ptr,
    free_ptr,
    hash_ptr,
    cmp_ptr,
    copy_simple,
    assign_ptr,
    fetch_ptr,
    "ptr",
    NULL,
    call_ptr,
};

op_t    o_mkptr         = {OBJ(TC_OP), ici_op_mkptr};
op_t    o_openptr       = {OBJ(TC_OP), ici_op_openptr};
op_t    o_fetch         = {OBJ(TC_OP), ici_op_fetch};
