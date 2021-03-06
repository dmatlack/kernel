/**
 * @file kernel/errno.c
 */
#include <errno.h>

const char *strerr(int err) {
#define STRERR(errorcode) case (errorcode): return #errorcode;
  switch (err < 0 ? -err : err) {
    STRERR(EGENERIC);
    STRERR(EPERM);
    STRERR(ENOENT);
    STRERR(ESRCH);
    STRERR(EINTR);
    STRERR(EIO);
    STRERR(ENXIO);
    STRERR(E2BIG);
    STRERR(ENOEXEC);
    STRERR(EBADF);
    STRERR(ECHILD);
    STRERR(EAGAIN);
    STRERR(ENOMEM);
    STRERR(EACCES);
    STRERR(EFAULT);
    STRERR(ENOTBLK);
    STRERR(EBUSY);
    STRERR(EEXIST);
    STRERR(EXDEV);
    STRERR(ENODEV);
    STRERR(ENOTDIR);
    STRERR(EISDIR);
    STRERR(EINVAL);
    STRERR(ENFILE);
    STRERR(EMFILE);
    STRERR(ENOTTY);
    STRERR(ETXTBSY);
    STRERR(EFBIG);
    STRERR(ENOSPC);
    STRERR(ESPIPE);
    STRERR(EROFS);
    STRERR(EMLINK);
    STRERR(EPIPE);
    STRERR(EDOM);
    STRERR(ERANGE);
    STRERR(EDEADLK);
    STRERR(ENAMETOOLONG);
    STRERR(ENOLCK);
    STRERR(ENOSYS);
    STRERR(ENOTEMPTY);
    STRERR(ELOOP);
    STRERR(ERESTART);
    STRERR(EIDRM);
    STRERR(EILSEQ);
    
    /* The following errors relate to networking. */
    STRERR(EPACKSIZE);
    STRERR(ENOBUFS);
    STRERR(EBADIOCTL);
    STRERR(EBADMODE);
    STRERR(EWOULDBLOCK);
    STRERR(ENETUNREACH);
    STRERR(EHOSTUNREACH);
    STRERR(EISCONN);
    STRERR(EADDRINUSE);
    STRERR(ECONNREFUSED);
    STRERR(ECONNRESET);
    STRERR(ETIMEDOUT);
    STRERR(EURG);
    STRERR(ENOURG);
    STRERR(ENOTCONN);
    STRERR(ESHUTDOWN);
    STRERR(ENOCONN);
    STRERR(EAFNOSUPPORT);
    STRERR(EPROTONOSUPPORT);
    STRERR(EPROTOTYPE);
    STRERR(EINPROGRESS);
    STRERR(EADDRNOTAVAIL);
    STRERR(EALREADY);
    STRERR(EMSGSIZE);
    STRERR(ENOTSOCK);
    STRERR(ENOPROTOOPT);
    STRERR(EOPNOTSUPP);
    //STRERR(ENOTSUP);
    STRERR(ENETDOWN);
    
    /* The following are not POSIX errors, but they can still happen. 
     * All of these are generated by the kernel and relate to message passing.
     */
    STRERR(ELOCKED);
    STRERR(EBADCALL);
    STRERR(EBADSRCDST);
    STRERR(ECALLDENIED);
    STRERR(EDEADSRCDST);
    STRERR(ENOTREADY);
    STRERR(EBADREQUEST);
    STRERR(ESRCDIED);
    STRERR(EDSTDIED);
    STRERR(ETRAPDENIED);
    STRERR(EDONTREPLY);
  }
#undef STRERR
  return "unknown";
}
