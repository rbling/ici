#define ICI_CORE
#include "fwd.h"
#ifndef NODEBUGGING

#include "exec.h"

/*
 * Stub functions for the debugging interface. These are linked by
 * default with the interpreter and may be overriden when linking
 * against the ICI library. Debuggers simply implement whatever user
 * interface they wish and link against the ICI library. This ensures
 * that all debuggers match the interpreter being used (as long as they
 * are all built at the same time).
 */

/*
 * ici_debug_error - called when the program raises an error.
 *
 * Parameters:
 *
 *      err     the error being set.
 *      src     the last source marker encountered.
 */
static void
ici_debug_error(char *err, src_t *src)
{
}

/*
 * ici_debug_fncall - called prior to a function call.
 *
 * Parameters:
 *
 *      o       The function being called.
 *      ap      The parameters to function, a (C) array of objects.
 *      nargs   The number of parameters in that array.
 */
static void
ici_debug_fncall(object_t *o, object_t **ap, int nargs)
{
}

/*
 * ici_debug_fnresult - called upon function return.
 *
 * Parameters:
 *
 *      o       The result of the function.
 */
static void
ici_debug_fnresult(object_t *o)
{
}

/*
 * ici_debug_src - called when a source line marker is encountered.
 *
 * Parameters:
 *
 *      src     The source marker encountered.
 */
static void
ici_debug_src(src_t *src)
{
}

/*
 * ici_debug_watch - called upon each assignment.
 *
 * Parameters:
 *
 *      o       The object being assigned into. For normal variable
 *              assignments this will be a struct, part of the scope.
 *      k       The key being used, typically a string, the name of a
 *              variable.
 *      v       The value being assigned to the object.
 */
static void
ici_debug_watch(object_t *o, object_t *k, object_t *v)
{
}

/*
 * The default debugging interface is the stub functions.  Debuggers
 * will assign ici_debug to point to a more useful set of functions.
 */
debug_t ici_debug_stubs =
{
    ici_debug_error,
    ici_debug_fncall,
    ici_debug_fnresult,
    ici_debug_src,
    ici_debug_watch
};

debug_t *ici_debug = &ici_debug_stubs;

#endif
