#ifndef ICI_CONF_H
#define ICI_CONF_H

#include <sys/param.h>
#undef isset

#ifndef BSD4_4
#define BAD_PRINTF_RETVAL
#endif

#undef  NOMATH          /* Trig and etc. */
#define NOTRACE         /* For debugging. */
#undef  NOWAITFOR       /* Requires select() or similar system primitive. */
#undef  NOSYSTEM        /* Command interpreter (shell) escape. */
#undef  NOPIPES         /* Requires popen(). */
#undef  NOSKT           /* BSD style network interface. */
#undef  NODIR           /* Directory reading function, dir(). */
#undef  NOPASSWD        /* UNIX password file access */
#undef  NODLOAD         /* Dynamic loading of native machine code modules. */
#undef  NOSTARTUPFILE   /* Parse a standard file of ICI code at init time. */
#undef  NODEBUGGING     /* Debugger interface and functions */
#define NOEVENTS        /* Event loop and associated processing. */
#define NOPROFILE       /* Profiler, see profile.c. */
#undef  NOSIGNALS       /* ICI level signal handling */

#ifdef _THREAD_SAFE
#define	ICI_USE_POSIX_THREADS
#endif

/*
 * Pre-3.0 FreeBSD uses a.out format objects that append a leading "_"
 * to symbol names. 3.0 and later are ELF that doesn't require this.
 */
#if defined(__FreeBSD__) && (__FreeBSD__ < 3)
#define NEED_UNDERSCORE_ON_SYMBOLS
#define CONFIG_STR      "FreeBSD 2.x configuration"
#else
#define CONFIG_STR      "FreeBSD 3.x configuration"
#endif

#endif /*ICI_CONF_H*/
