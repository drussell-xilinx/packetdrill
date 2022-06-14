#ifndef __ONLOAD_H__
#define __ONLOAD_H__

#include <onload/extensions_zc.h>
#include "run.h"

struct zc_buffer {
	void *buf;
	size_t buf_len;
	onload_zc_handle handle;
};

struct zc_buffer *alloc_zc_buffer(void);
int do_onload_zc_send(struct state *state, int fd, size_t send_len, int flags);

#endif /* __ONLOAD_H__ */
