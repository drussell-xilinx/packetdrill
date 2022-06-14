#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/mman.h>

#include "assert.h"
#include "onload.h"
#include "run.h"

#define HUGE_PAGE_SIZE (1 << 21)

struct zc_buffer *alloc_zc_buffer(void)
{
	struct zc_buffer *buf;
	int fd, rc;
	int map_flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB | MAP_HUGE_2MB;
	int prot = PROT_READ | PROT_WRITE;

	buf = calloc(1, sizeof(*buf));
	buf->buf_len = HUGE_PAGE_SIZE;


	buf->buf = mmap(NULL, buf->buf_len, prot, map_flags, -1, 0);
	if( buf->buf == MAP_FAILED )
		die("ERROR: Failed to allocate huge page buffer for ZC sends\n");
	memset(buf->buf, 0, buf->buf_len);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	rc = onload_zc_register_buffers(fd, EF_ADDRSPACE_LOCAL,
					(uint64_t) buf->buf,
					buf->buf_len,
					0, /* flags */
					&buf->handle);
	close(fd);
	if( rc != 0 )
		die("ERROR: onload_zc_register_buffers failed rc=%d [%s]\n",
		    rc, strerror(-rc));

	return buf;
}

int do_onload_zc_send(struct state *state, int fd, size_t len, int flags)
{
	struct onload_zc_mmsg mmsg = { 0 };
	struct onload_zc_iovec iov = { 0 };
	int rc;

	assert(len <= state->zc_buf->buf_len);

	iov.iov_base = state->zc_buf->buf;
	iov.iov_len = len;
	iov.buf = state->zc_buf->handle;
	iov.iov_flags = flags;

	mmsg.msg.msghdr.msg_iovlen = 1;
	mmsg.msg.iov = &iov;
	mmsg.fd = fd;

	rc = onload_zc_send(&mmsg,
			    1, /* msgcount */
			    0 /* flags */);

	return (rc == 1) ? mmsg.rc : rc;
}
