#define ICI_CORE
#include "func.h"
#include "exec.h"
#include "ptr.h"
#include "struct.h"
#include "op.h"
#include "pc.h"
#include "str.h"
#include "catch.h"
#include "buf.h"
#include "mark.h"
#include "null.h"
#include "primes.h"
#ifndef NOPROFILE
#include "profile.h"
#endif

static void
objname_func(object_t *o, char p[ICI_OBJNAMEZ])
{
    string_t    *s;

    s = funcof(o)->f_name;
    if (s->s_nchars > ICI_OBJNAMEZ - 2 - 1)
        sprintf(p, "%.*s...()", ICI_OBJNAMEZ - 6, s->s_chars);
    else
        sprintf(p, "\"%s()\"", s->s_chars);
}

/*
 * Mark this and referenced unmarked objects, return memory costs.
 * See comments on t_mark() in object.h.
 */
static unsigned long
mark_func(object_t *o)
{
    long        mem;

    o->o_flags |= O_MARK;
    mem = sizeof(func_t);
    if (funcof(o)->f_code != NULL)
        mem += mark(objof(funcof(o)->f_code));
    if (funcof(o)->f_args != NULL)
        mem += mark(objof(funcof(o)->f_args));
    if (funcof(o)->f_autos != NULL)
        mem += mark(objof(funcof(o)->f_autos));
    if (funcof(o)->f_name != NULL)
        mem += mark(objof(funcof(o)->f_name));
    return mem;
}

/*
 * Returns 0 if these objects are equal, else non-zero.
 * See the comments on t_cmp() in object.h.
 */
static int
cmp_func(object_t *o1, object_t *o2)
{
    return funcof(o1)->f_code != funcof(o2)->f_code
        || funcof(o1)->f_autos != funcof(o2)->f_autos
        || funcof(o1)->f_args != funcof(o2)->f_args
        || funcof(o1)->f_name != funcof(o2)->f_name;
}

/*
 * Return a hash sensitive to the value of the object.
 * See the comment on t_hash() in object.h
 */
static unsigned long
hash_func(object_t *o)
{
    return (unsigned long)funcof(o)->f_code * FUNC_PRIME;
}

func_t *
new_func()
{
    register func_t     *f;

    if ((f = ici_talloc(func_t)) == NULL)
        return NULL;
    memset((char *)f, 0, sizeof(func_t));
    objof(f)->o_tcode = TC_FUNC;
    assert(ici_typeof(f) == &ici_func_type);
    objof(f)->o_nrefs = 1;
    rego(f);
    return f;
}

/*
 * Free this object and associated memory (but not other objects).
 * See the comments on t_free() in object.h.
 */
static void
free_func(object_t *o)
{
    ici_tfree(o, func_t);
}

/*
 * Return the object at key k of the obejct o, or NULL on error.
 * See the comment on t_fetch in object.h.
 */
static object_t *
fetch_func(object_t *o, object_t *k)
{
    object_t            *r;

    ici_error = NULL;
    r = NULL;
    if (k == SSO(autos))
        r = objof(funcof(o)->f_autos);
    else if (k == SSO(args))
        r = objof(funcof(o)->f_args);
    else if (k == SSO(name))
        r = objof(funcof(o)->f_name);
    if (r == NULL && ici_error == NULL)
        r = objof(&o_null);
    return r;
}

int
ici_op_return()
{
    register object_t   **x;

    x = ici_xs.a_top;
    while
    (
        --x >= ici_xs.a_base
        &&
        !ismark(*x)
        &&
        !(iscatch(*x) && isnull(catchof(*x)->c_catcher))
    )
        ;
    if (x < ici_xs.a_base || !ismark(*x))
    {
        ici_error = "return not in function";
        return 1;
    }
    ici_xs.a_top = x;
    --ici_vs.a_top;
#ifndef NOPROFILE
    if (ici_profile_active)
        ici_profile_return();
#endif
    return 0;
}

/*
 * arg(N-1) .. arg1 arg0 nargs func     => (os) OR
 * arg(N-1) .. arg1 arg0 nargs ptr      => (os) OR
 * arg(N-1) .. arg1 arg0 nargs aggr key => (os) iff OP_AGGR_KEY_CALL
 *                                => auto-struct  (vs)
 *                      call(N)   => mark pc      (xs)
 *
 * Calling a function pushes a structure for auto variables on the
 * variable stack. It then pushes a mark and a pc starting at the first
 * element of the code array on the execution stack. Any arguments are
 * assigned to the corresponding formal argument names in the auto var
 * structure.
 */
static int
call_func(object_t *o, object_t *subject)
{
    register func_t     *f;
    register struct_t   *d;     /* The local variable structure. */
    register object_t   **ap;   /* Actual parameter. */
    register object_t   **fp;   /* Formal parameter. */
    slot_t              *sl;
    array_t             *va;
    int                 n;

    f = funcof(o);
#ifndef NOPROFILE
    if (ici_profile_active)
        ici_profile_call(f);
#endif

    if ((d = structof(copy(objof(f->f_autos)))) == NULL)
        goto fail;
    if (subject != NULL)
    {
        /*
         * This is a method call, that is, it has a subject object that
         * becomes the scope.
         */
        if (!hassuper(subject))
        {
            char        n1[30];

            sprintf(buf, "attempt to call method on %s", objname(n1, subject));
            ici_error = buf;
            goto fail;
        }
        objwsupof(d)->o_super = objwsupof(subject);
        /*
         * Set the special instantiation variables.
         */
        if (assign_base(d, SSO(this), subject))
            goto fail;
        if
        (
            objwsupof(f->f_autos)->o_super != NULL
            &&
            assign_base(d, SSO(class), objwsupof(f->f_autos)->o_super)
        )
            goto fail;
    }
    n = NARGS(); /* Number of actual args. */
    ap = ARGS();
    if (f->f_args != NULL)
    {
        /*
         * There are explicit formal parameters.
         */
        fp = f->f_args->a_base;
        /*
         * Assign the actuals to the formals.
         */
        while (fp < f->f_args->a_top && n > 0)
        {
            assert(isstring(*fp));
            if (stringof(*fp)->s_struct == d && stringof(*fp)->s_vsver == ici_vsver)
            {
                stringof(*fp)->s_slot->sl_value = *ap;
            }
            else
            {
                if (assign(d, *fp, *ap))
                    goto fail;
            }
            ++fp;
            --ap;
            --n;
        }
    }
    va = NULL;
    if
    (
        n > 0
        &&
        (sl = find_raw_slot(d, SSO(vargs))) != NULL
        &&
        (va = new_array(n)) != NULL
    )
    {
        /*
         * There are left-over actual parameters and a "vargs"
         * auto to put them in, and everything else looks good.
         */
        while (--n >= 0)
            *va->a_top++ = *ap--;
        sl->sl_value = objof(va);
    }
    if (va != NULL)
        decref(va);

    ici_xs.a_top[-1] = objof(&o_mark);
    if (new_pc(f->f_code, ici_xs.a_top))
        goto fail;
    ++ici_xs.a_top;
    *ici_vs.a_top++ = objof(d);
    decref(d);
    ici_os.a_top -= NARGS() + 2;
    return 0;

fail:
    if (d != NULL)
        decref(d);
    return 1;
}

type_t  ici_func_type =
{
    mark_func,
    free_func,
    hash_func,
    cmp_func,
    copy_simple,
    assign_simple,
    fetch_func,
    "func",
    objname_func,
    call_func
};

op_t    o_return        = {OBJ(TC_OP), ici_op_return};
op_t    o_call          = {OBJ(TC_OP), NULL, OP_CALL};
op_t    o_method_call   = {OBJ(TC_OP), NULL, OP_METHOD_CALL};
op_t    o_super_call    = {OBJ(TC_OP), NULL, OP_SUPER_CALL};
