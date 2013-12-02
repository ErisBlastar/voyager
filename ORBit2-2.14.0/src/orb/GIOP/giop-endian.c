#define GIOP_DO_NOT_INLINE_IIOP_BYTESWAP
#include <config.h>
#include <orbit/GIOP/giop.h>

void
giop_byteswap (guchar       *outdata,
	       const guchar *data,
	       gulong        datalen)
{
	const guchar *source_ptr = data;
	guchar       *dest_ptr = (guchar *) outdata + datalen - 1;

	while (dest_ptr >= outdata)
		*dest_ptr-- = *source_ptr++;
}
