#ifndef _LINK_DEBUG_H
#define _LINK_DEBUG_H

/* Enables debug printout for the socket / connection */
#undef CONNECTION_DEBUG

/* If CONNECTION_DEBUG is defined, define this to make it selectable
 * at runtime by setting the LINK_CONNECTION_DEBUG env var.
 */
#define CONNECTION_DEBUG_FLAG

#ifndef CONNECTION_DEBUG
#  ifdef G_HAVE_ISO_VARARGS
#    define d_printf(...)
#  elif defined(G_HAVE_GNUC_VARARGS)
#    define d_printf(args...)
#  else
static inline void d_printf (const char *format, ...)
{
}
#  endif
#  define STATE_NAME(s) ""
#else
#  include <stdio.h>
#  define STATE_NAME(s) (((s) == LINK_CONNECTED) ? "Connected" : \
			 ((s) == LINK_CONNECTING) ? "Connecting" : \
			 ((s) == LINK_DISCONNECTED) ? "Disconnected" : \
			 "Invalid state")
#  ifdef CONNECTION_DEBUG_FLAG
extern gboolean link_connection_debug_flag;
#    define d_printf(format...) (link_connection_debug_flag ? (fprintf (stderr, format), fflush (stderr)) : 0)
#  else
#    define d_printf(format...) fprintf (stderr, format)
#  endif
#endif

#endif /* _LINK_DEBUG_H */
