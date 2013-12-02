#ifndef GIOP_ENDIAN_H
#define GIOP_ENDIAN_H 1

#include <orbit/GIOP/giop-types.h>

G_BEGIN_DECLS

#ifdef ORBIT2_INTERNAL_API

/* This is also defined in IIOP-types.c */
void giop_byteswap(guchar *outdata,
		   const guchar *data,
		   gulong datalen);

#if defined(G_CAN_INLINE) && !defined(GIOP_DO_NOT_INLINE_IIOP_BYTESWAP)
G_INLINE_FUNC void giop_byteswap(guchar *outdata,
				 const guchar *data,
				 gulong datalen)
{
  const guchar *source_ptr = data;
  guchar *dest_ptr = outdata + datalen - 1;
  while(dest_ptr >= outdata)
    *dest_ptr-- = *source_ptr++;
}
#endif

#endif /* ORBIT2_INTERNAL_API */

G_END_DECLS

#endif
