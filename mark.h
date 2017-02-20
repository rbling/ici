#ifndef ICI_MARK_H
#define ICI_MARK_H

#ifndef ICI_OBJECT_H
#include "object.h"
#endif

/*
 * The following portion of this file exports to ici.h. --ici.h-start--
 */

/*
 * Mark objects are used in a few placed in the interpreter when we need
 * an object that we can guarantee is distinct from any object a user
 * could give us. One use is as the 'label' on the default clause of
 * the struct that represents a switch statement.
 */
struct marks
{
    object_t    o_head;
};
#define markof(o)       ((mark_t *)o)
#define ismark(o)       ((o) == objof(&o_mark))
/*
 * End of ici.h export. --ici.h-end--
 */

#endif /* ICI_MARK_H */
