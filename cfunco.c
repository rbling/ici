#define ICI_CORE
#include "cfunc.h"
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
objname_cfunc(object_t *o, char p[ICI_OBJNAMEZ])
{
    char    *n;

    n = cfuncof(o)->cf_name;
    if (strlen(n) > ICI_OBJNAMEZ - 2 - 1)
        sprintf(p, "%.*s...()", ICI_OBJNAMEZ - 6, n);
    else
        sprintf(p, "%.*s()", ICI_OBJNAMEZ - 3, n);
}

/*
 * Mark this and referenced unmarked objects, return memory costs.
 * See comments on t_mark() in object.h.
 */
static unsigned long
mark_cfunc(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof(cfunc_t);
}

/*
 * Return the object at key k of the obejct o, or NULL on error.
 * See the comment on t_fetch in object.h.
 */
static object_t *
fetch_cfunc(object_t *o, object_t *k)
{
    if (k == SSO(name))
        return objof(ici_str_get_nul_term(cfuncof(o)->cf_name));
    return objof(&o_null);
}

/*
 * ici_assign_cfuncs
 *
 * Assign the structure s all the intrinsic functions listed in the array
 * of cfunc_t structures pointed to by cf. The array must be terminated by
 * an entry with a cf_name of NULL. Typically, entries in the array are
 * formated as:
 *
 *  {CF_OBJ,    "func",     f_func},
 *
 * Where CF_OBJ is a convenience macro from func.h to take care of the
 * normal object header, "func" is the name your function will be assigned
 * to in the given struct, and f_func is a C function obeying the rules
 * of ICI intrinsic functions.
 *
 * Returns non-zero on error, in which case error is set, else zero.
 */
int
ici_assign_cfuncs(objwsup_t *s, cfunc_t *cf)
{
    string_t    *n;

    while (cf->cf_name != NULL)
    {
        assert(ici_typeof(cf) == &ici_cfunc_type);
        if (cf->cf_name[0] == TC_STRING)
        {
            /*
             * Temporary migration hack while we change over
             * to using static strings in these initialisations
             * in the ICI core.
             */
            n = (string_t *)cf->cf_name;
            cf->cf_name = n->s_chars;
            /* ### should be a decref here? ### */
        }
        else
        {
            if ((n = ici_str_new_nul_term(cf->cf_name)) == NULL)
                return 1;
        }
        if (assign_base(s, n, cf))
        {
            ici_decref(n);
            return 1;
        }
        ici_decref(n);
        ++cf;
    }
    return 0;
}

/*
 * def_cfuncs
 *
 * Define the given intrinsic functions in the current static scope.
 * See ici_assign_cfuncs() above for details.
 *
 * Returns non-zero on error, in which case error is set, else zero.
 */
int
def_cfuncs(cfunc_t *cf)
{
    return ici_assign_cfuncs(objwsupof(ici_vs.a_top[-1])->o_super, cf);
}

/*
 * Create a new class struct and assign the given cfuncs into it (as
 * in ici_new_struct_with_cfuncs() above). If super is NULL, the super
 * of the new struct is set to the outer-most writeable struct in the
 * current scope. Thus this is a new top-level class (not derived from
 * anything). If super is non-NULL, it is presumably the parent class
 * and is used directly as the super. Returns NULL on error, usual
 * conventions. The returned struct has an incref the caller owns.
 */
objwsup_t *
ici_class_new(cfunc_t *cf, objwsup_t *super)
{
    objwsup_t           *s;

    if ((s = objwsupof(ici_struct_new())) == NULL)
        return NULL;
    if (ici_assign_cfuncs(s, cf))
    {
        ici_decref(s);
        return NULL;
    }
    if (super == NULL && (super = ici_outermost_writeable_struct()) == NULL)
        return NULL;
    s->o_super = super;
    return s;
}

objwsup_t *
ici_module_new(cfunc_t *cf)
{
    return ici_class_new(cf, NULL);
}

static int
call_cfunc(object_t *o, object_t *subject)
{
    int                 result;

    result = (*cfuncof(o)->cf_cfunc)(subject);
#ifndef NOPROFILE
    if (ici_profile_active)
        ici_profile_return();
#endif
#ifndef NODEBUGGING
    if (ici_debug_enabled)
        ici_debug->idbg_fnresult(ici_os.a_top[-1]);
#endif
    return result;
}

type_t  ici_cfunc_type =
{
    mark_cfunc,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    ici_assign_fail,
    fetch_cfunc,
    "func",
    objname_cfunc,
    call_cfunc
};
