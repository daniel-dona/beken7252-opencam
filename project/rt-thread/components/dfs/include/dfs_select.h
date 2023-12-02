#ifndef DFS_SELECT_H__
#define DFS_SELECT_H__

#include <libc/libc_fdset.h>
#include <sys/select.h>

#ifndef fd_set

#  ifndef	FD_SETSIZE
#	define	FD_SETSIZE	64
#  endif

typedef	unsigned long	fd_mask;
#  define	NFDBITS	(sizeof (fd_mask) * 8)	/* bits per mask */
#  ifndef	_howmany
#	define	_howmany(x,y)	(((x)+((y)-1))/(y))
#  endif

typedef	struct _types_fd_set {
    fd_mask	fds_bits[_howmany(FD_SETSIZE, NFDBITS)];
} _types_fd_set;
#define fd_set _types_fd_set

#  define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#  define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#  define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#  define	FD_ZERO(p)	(__extension__ (void)({ \
     size_t __i; \
     char *__tmp = (char *)p; \
     for (__i = 0; __i < sizeof (*(p)); ++__i) \
       *__tmp++ = 0; \
}))

#endif


#ifdef __cplusplus
extern "C" {
#endif

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

#ifdef __cplusplus
}
#endif

#endif

