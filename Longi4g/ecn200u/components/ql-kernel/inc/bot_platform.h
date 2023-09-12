/*
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
 */
#ifndef __BOT_PLATFORM_H
#define __BOT_PLATFORM_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


#include "ql_api_osi.h"

//log
#include "ql_log.h"
// fs
#include "ql_fs.h"
// Network
#include "sockets.h"
#include "lwipopts.h"

/* configuration about FS */
#define BOT_FS_STREAM_ENABLED       0       // support fs stream operation, like linux, windows.etc

typedef void  bot_statfs_t;     // TODO 
/***********************************************************************************************************************
 * Macro Definition
***********************************************************************************************************************/
#define BOT_FS_USER_DIR "UFS:bot"

/* micro defination about file system, used for bot_open()'s flag */
#define BOT_O_RDONLY        00              /* 以只读方式打开文件 */
#define BOT_O_WRONLY  	    01              /* 以只写方式打开文件 */
#define BOT_O_RDWR    		02              /* 以可读写方式打开文件. 上述三种旗标是互斥的, 也就是不可同时使用, 但可与下列的旗标利用OR(|)运算符组合. */
#define BOT_O_APPEND  		02000           /* 当读写文件时会从文件尾开始移动, 也就是所写入的数据会以附加的方式加入到文件后面. */
#define BOT_O_CREAT   		0100            /* 若欲打开的文件不存在则自动建立该文件. */
#define BOT_O_TRUNC   		01000           /* 若文件存在并且以可写的方式打开时, 此旗标会令文件长度清为0, 而原来存于该文件的资料也会消失. */
#define BOT_O_EXCL    		0200            /* 如果O_CREAT 也被设置, 此指令会去检查文件是否存在. 文件若不存在则建立该文件, 否则将导致打开文件错误. 此外, 若O_CREAT 与O_EXCL 同时设置, 并且欲打开的文件为符号连接, 则会打开文件失败. */
#define BOT_O_SYNC    		04010000        /* 以同步的方式打开文件. */
#define BOT_O_NOCTTY        0400            /* 如果欲打开的文件为终端机设备时, 则不会将该终端机当成进程控制终端机. */
#define BOT_O_NONBLOCK      04000           /* 以不可阻断的方式打开文件, 也就是无论有无数据读取或等待, 都会立即返回进程之中. */
#define BOT_O_NDELAY        04000           /* 同 O_NONBLOCK. */
// #define BOT_O_NOFOLLOW      O_NOFOLLOW      /* 如果参数 path 所指的文件为一符号连接, 则会令打开文件失败. */
// #define BOT_O_DIRECTORY     O_DIRECTORY     /* 如果参数 path 所指的文件并非为一目录, 则会令打开文件失败。注：此为Linux2. 2 以后特有的旗标, 以避免一些系统安全问题. */
/* micro defination about file system, used for bot_fseek()'s/bot_seek()'s whence */
#define BOT_SEEK_SET        QL_SEEK_SET
#define BOT_SEEK_CUR        QL_SEEK_CUR
#define BOT_SEEK_END        QL_SEEK_END


/* 参数mode 则有下列数种组合, 只有在建立新文件时才会生效, 此外真正建文件时的权限会受到umask 值所影响, 因此该文件权限应该为 (mode-umaks). */
#define BOT_S_IRWXU         00700         /* 00700 权限, 代表该文件所有者具有可读、可写及可执行的权限. */
#define BOT_S_IRUSR         00400         /* 或 S_IREAD, 00400 权限, 代表该文件所有者具有可读取的权限. */
#define BOT_S_IWUSR         00200         /* 或 S_IWRITE, 00200 权限, 代表该文件所有者具有可写入的权限. */
#define BOT_S_IXUSR         00100         /* 或 S_IEXEC, 00100 权限, 代表该文件所有者具有可执行的权限. */
#define BOT_S_IRWXG         00070         /* 00070 权限, 代表该文件用户组具有可读、可写及可执行的权限. */
#define BOT_S_IRGRP         00040         /* 00040 权限, 代表该文件用户组具有可读的权限. */
#define BOT_S_IWGRP         00020         /* 00020 权限, 代表该文件用户组具有可写入的权限. */
#define BOT_S_IXGRP         00010         /* 00010 权限, 代表该文件用户组具有可执行的权限. */
#define BOT_S_IRWXO         00007         /* 00007 权限, 代表其他用户具有可读、可写及可执行的权限. */
#define BOT_S_IROTH         00004         /* 00004 权限, 代表其他用户具有可读的权限 */
#define BOT_S_IWOTH         00002         /* 00002 权限, 代表其他用户具有可写入的权限. */
#define BOT_S_IXOTH         00001         /* 00001 权限, 代表其他用户具有可执行的权限. */


/* bot_exitdir() mode */
#define BOT_ACESS_R_OK      4               /* read OK */
#define BOT_ACESS_W_OK      2               /* write OK */
#define BOT_ACESS_X_OK      1               /* execute OK */
#define BOT_ACESS_F_OK      0               /* file exit: OK */

/* 返回值：若所有欲核查的权限都通过了检查则返回0 值, 表示成功, 只要有一个权限被禁止则返回-1. */
/* bot error code defination */

#define	BOT_E_EPERM         1	            /* 1, Operation not permitted */
#define	BOT_E_ENOENT        2		        /* 2, No such file or directory */
#define	BOT_E_ESRCH         3		        /* 3, No such process */
#define	BOT_E_EINTR         4		        /* 4, Interrupted system call */
#define	BOT_E_EIO           5		        /* 5, I/O error */
#define	BOT_E_ENXIO         6		        /* 6, No such device or address */
#define	BOT_E_E2BIG         7		        /* 7, Argument list too long */
#define	BOT_E_ENOEXEC       8	            /* 8, Exec format error */
#define	BOT_E_EBADF         9		        /* 9, Bad file number */
#define	BOT_E_ECHILD        10		        /* 10 No child processes */
#define	BOT_E_EAGAIN        11		        /* 11, Try again */
#define	BOT_E_ENOMEM        12 		        /* 12, Out of memory */
#define	BOT_E_EACCES        13		        /* 13, Permission denied */
#define	BOT_E_EFAULT        14		        /* 14, Bad address */
#define	BOT_E_ENOTBLK       15	            /* 15, Block device required */
#define	BOT_E_EBUSY         16		        /* 16, Device or resource busy */
#define	BOT_E_EEXIST        17		        /* 17, File exists */
#define	BOT_E_EXDEV         18		        /* 18, Cross-device link */
#define	BOT_E_ENODEV        19		        /* 19, No such device */
#define	BOT_E_ENOTDIR       20		        /* 20, Not a directory */
#define	BOT_E_EISDIR        21		        /* 21, Is a directory */
#define	BOT_E_EINVAL        22		        /* 22, Invalid argument */
#define	BOT_E_ENFILE        23		        /* 23, File table overflow */
#define	BOT_E_EMFILE        24		        /* 24, Too many open files */
#define	BOT_E_ENOTTY        25		        /* 25, Not a typewriter */
#define	BOT_E_ETXTBSY       26	            /* 26, Text file busy */
#define	BOT_E_EFBIG         27		        /* 27, File too large */
#define	BOT_E_ENOSPC        28		        /* 28, No space left on device */
#define	BOT_E_ESPIPE        29		        /* 29, Illegal seek */
#define	BOT_E_EROFS         30		        /* 30, Read-only file system */
#define	BOT_E_EMLINK        31		        /* 31, Too many links */
#define	BOT_E_EPIPE         32		        /* 32, Broken pipe */
#define	BOT_E_EDOM          33		        /* 33, Math argument out of domain of func */
#define	BOT_E_ERANGE        34		        /* 34, Math result not representable */


#define	BOT_E_EDEADLK	    35		        /* 35, Resource deadlock would occur */
#define	BOT_E_ENAMETOOLONG	36	            /* 36, File name too long */
#define	BOT_E_ENOLCK	    37		        /* 37, No record locks available */

/*
 * This error code is special: arch syscall entry code will return
 * -ENOSYS if users try to call a syscall that doesn't exist.  To keep
 * failures of syscalls that really do exist distinguishable from
 * failures due to attempts to use a nonexistent syscall, syscall
 * implementations should refrain from returning -ENOSYS.
 */
#define	BOT_E_ENOSYS	        38		       /* 38, Invalid system call number */

#define	BOT_E_ENOTEMPTY	        39	            /* 39, Directory not empty */
#define	BOT_E_ELOOP	            40			    /* 40, Too many symbolic links encountered */
#define	BOT_E_EWOULDBLOCK	    11		        /* like, EAGAIN, Operation would block */
#define	BOT_E_ENOMSG	        42		        /* 42, No message of desired type */
#define	BOT_E_EIDRM	            43		        /* 43, Identifier removed */
#define	BOT_E_ECHRNG	        44		        /* 44, Channel number out of range */
#define	BOT_E_EL2NSYNC	        45	            /* 45, Level 2 not synchronized */
#define	BOT_E_EL3HLT	        46		        /* 46, Level 3 halted */
#define	BOT_E_EL3RST	        47		        /* 47, Level 3 reset */
#define	BOT_E_ELNRNG	        48		        /* 48, Link number out of range */
#define	BOT_E_EUNATCH	        49		        /* 49, Protocol driver not attached */
#define	BOT_E_ENOCSI	        50		        /* 50, No CSI structure available */
#define	BOT_E_EL2HLT	        51		        /* 51, Level 2 halted */
#define	BOT_E_EBADE	            52		        /* 52, Invalid exchange */
#define	BOT_E_EBADR	            53		        /* 53, Invalid request descriptor */
#define	BOT_E_EXFULL	        54		        /* 54, Exchange full */
#define	BOT_E_ENOANO	        55		        /* 55, No anode */
#define	BOT_E_EBADRQC	        56		        /* 56, Invalid request code */
#define	BOT_E_EBADSLT	        57		        /* 57, Invalid slot */

#define	BOT_E_EDEADLOCK	        35	            /* EDEADLK, Resource deadlock would occur */

#define	BOT_E_EBFONT	        59		        /* 59, Bad font file format */
#define	BOT_E_ENOSTR	        60		        /* 60, Device not a stream */
#define	BOT_E_ENODATA	        61		        /* No data available */
#define	BOT_E_ETIME	            62		        /* Timer expired */
#define	BOT_E_ENOSR	            63		        /* Out of streams resources */
#define	BOT_E_ENONET	        64		        /* Machine is not on the network */
#define	BOT_E_ENOPKG	        65		        /* Package not installed */
#define	BOT_E_EREMOTE	        66		        /* Object is remote */
#define	BOT_E_ENOLINK	        67		        /* Link has been severed */
#define	BOT_E_EADV	            68		        /* Advertise error */
#define	BOT_E_ESRMNT	        69		        /* Srmount error */
#define	BOT_E_ECOMM	            70		        /* Communication error on send */
#define	BOT_E_EPROTO	        71		        /* Protocol error */
#define	BOT_E_EMULTIHOP	        72	            /* Multihop attempted */
#define	BOT_E_EDOTDOT	        73		        /* RFS specific error */
#define	BOT_E_EBADMSG	        74		        /* Not a data message */
#define	BOT_E_EOVERFLOW	        75	            /* Value too large for defined data type */
#define	BOT_E_ENOTUNIQ	        76	            /* Name not unique on network */
#define	BOT_E_EBADFD	        77		        /* File descriptor in bad state */
#define	BOT_E_EREMCHG	        78		        /* Remote address changed */
#define	BOT_E_ELIBACC	        79		        /* Can not access a needed shared library */
#define	BOT_E_ELIBBAD	        80		        /* Accessing a corrupted shared library */
#define	BOT_E_ELIBSCN	        81		        /* .lib section in a.out corrupted */
#define	BOT_E_ELIBMAX	        82		        /* Attempting to link in too many shared libraries */
#define	BOT_E_ELIBEXEC  	    83	            /* Cannot exec a shared library directly */
#define	BOT_E_EILSEQ	        84		        /* Illegal byte sequence */
#define	BOT_E_ERESTART	        85	            /* Interrupted system call should be restarted */
#define	BOT_E_ESTRPIPE  	    86	            /* Streams pipe error */
#define	BOT_E_EUSERS	        87		        /* Too many users */
#define	BOT_E_ENOTSOCK  	    88	            /* Socket operation on non-socket */
#define	BOT_E_EDESTADDRREQ	    89	            /* Destination address required */
#define	BOT_E_EMSGSIZE	        90	            /* Message too long */
#define	BOT_E_EPROTOTYPE	    91	            /* Protocol wrong type for socket */
#define	BOT_E_ENOPROTOOPT	    92	            /* Protocol not available */
#define	BOT_E_EPROTONOSUPPORT	93	            /* Protocol not supported */
#define	BOT_E_ESOCKTNOSUPPORT	94		        /* Socket type not supported */
#define	BOT_E_EOPNOTSUPP	    95		        /* Operation not supported on transport endpoint */
#define	BOT_E_EPFNOSUPPORT  	96		        /* Protocol family not supported */
#define	BOT_E_EAFNOSUPPORT	    97		        /* Address family not supported by protocol */
#define	BOT_E_EADDRINUSE	    98		        /* Address already in use */
#define	BOT_E_EADDRNOTAVAIL	    99		        /* Cannot assign requested address */
#define	BOT_E_ENETDOWN	        100	    	    /* Network is down */
#define	BOT_E_ENETUNREACH	    101	    	    /* Network is unreachable */
#define	BOT_E_ENETRESET	        102	    	    /* Network dropped connection because of reset */
#define	BOT_E_ECONNABORTED	    103	            /* Software caused connection abort */
#define	BOT_E_ECONNRESET	    104	    	    /* Connection reset by peer */
#define	BOT_E_ENOBUFS	        105		    	/* No buffer space available */
#define	BOT_E_EISCONN	        106		    	/* Transport endpoint is already connected */
#define	BOT_E_ENOTCONN	        107	     	    /* Transport endpoint is not connected */
#define	BOT_E_ESHUTDOWN	        108		        /* Cannot send after transport endpoint shutdown */
#define	BOT_E_ETOOMANYREFS	    109	            /* Too many references: cannot splice */
#define	BOT_E_ETIMEDOUT	        110	    	    /* Connection timed out */
#define	BOT_E_ECONNREFUSED	    111	            /* Connection refused */
#define	BOT_E_EHOSTDOWN	        112	            /* Host is down */
#define	BOT_E_EHOSTUNREACH	    113	            /* No route to host */
#define	BOT_E_EALREADY	        114	    	    /* Operation already in progress */
#define	BOT_E_EINPROGRESS	    115	    	    /* Operation now in progress */
#define	BOT_E_ESTALE	        116		    	/* Stale file handle */
#define	BOT_E_EUCLEAN	        117		    	/* Structure needs cleaning */
#define	BOT_E_ENOTNAM	        118		    	/* Not a XENIX named type file */
#define	BOT_E_ENAVAIL	        119		    	/* No XENIX semaphores available */
#define	BOT_E_EISNAM	        120		    	/* Is a named type file */
#define	BOT_E_EREMOTEIO 	    121	    	    /* Remote I/O error */
#define	BOT_E_EDQUOT	        122		    	/* Quota exceeded */

#define	BOT_E_ENOMEDIUM	        123	    	    /* No medium found */
#define	BOT_E_EMEDIUMTYPE	    124	    	    /* Wrong medium type */
#define	BOT_E_ECANCELED	        125	    	    /* Operation Canceled */
#define	BOT_E_ENOKEY	        126			    /* Required key not available */
#define	BOT_E_EKEYEXPIRED	    127	    	    /* Key has expired */
#define	BOT_E_EKEYREVOKED	    128	    	    /* Key has been revoked */
#define	BOT_E_EKEYREJECTED	    129	            /* Key was rejected by service */

/* for robust mutexes */
#define	BOT_E_EOWNERDEAD	    130	    	    /* Owner died */
#define	BOT_E_ENOTRECOVERABLE	131	            /* State not recoverable */

#define	BOT_E_ERFKILL           132		    	/* Operation not possible due to RF-kill */

#define	BOT_E_EHWPOISON         133	    	    /* 133, Memory page has hardware error */
//-----------------------------------------------------
// Socket Macro
#define BOT_AF_UNSPEC       AF_UNSPEC
#define BOT_SOCK_STREAM     SOCK_STREAM
#define BOT_SOCK_DGRAM      SOCK_DGRAM
#define BOT_IPPROTO_TCP     IPPROTO_TCP
#define BOT_IPPROTO_UDP     IPPROTO_UDP
#define BOT_SOL_SOCKET     SOL_SOCKET

#define BOT_ERROK          ERROK           /* err ok set, no err happen */
#define BOT_EPERM          EPERM           /* Operation not permitted */
#define BOT_ENOENT         ENOENT          /* No such file or directory */
#define BOT_ESRCH          ESRCH           /* No such process */
#define BOT_EINTR          EINTR           /* Interrupted system call */
#define BOT_EIO            EIO             /* I/O error */
#define BOT_ENXIO          ENXIO           /* No such device or address */
#define BOT_E2BIG          E2BIG           /* Arg list too long */
#define BOT_ENOEXEC        ENOEXEC         /* Exec format error */
#define BOT_EBADF          EBADF           /* Bad file number */
#define BOT_ECHILD         ECHILD          /* No child processes */
#define BOT_EAGAIN         EAGAIN          /* Try again */
#define BOT_ENOMEM         ENOMEM          /* Out of memory */
#define BOT_EACCES         EACCES          /* Permission denied */
#define BOT_EFAULT         EFAULT          /* Bad address */
#define BOT_ENOTBLK        ENOTBLK         /* Block device required */
#define BOT_EBUSY          EBUSY           /* Device or resource busy */
#define BOT_EEXIST         EEXIST          /* File exists */
#define BOT_EXDEV          EXDEV           /* Cross-device link */
#define BOT_ENODEV         ENODEV          /* No such device */
#define BOT_ENOTDIR        ENOTDIR         /* Not a directory */
#define BOT_EISDIR         EISDIR          /* Is a directory */
#define BOT_EINVAL         EINVAL          /* Invalid argument */
#define BOT_ENFILE         ENFILE          /* File table overflow */
#define BOT_EMFILE         EMFILE          /* Too many open files */
#define BOT_ENOTTY         ENOTTY          /* Not a typewriter */
#define BOT_ETXTBSY        ETXTBSY         /* Text file busy */
#define BOT_EFBIG          EFBIG           /* File too large */
#define BOT_ENOSPC         ENOSPC          /* No space left on device */
#define BOT_ESPIPE         ESPIPE          /* Illegal seek */
#define BOT_EROFS          EROFS           /* Read-only file system */
#define BOT_EMLINK         EMLINK          /* Too many links */
#define BOT_EPIPE          EPIPE           /* Broken pipe */
#define BOT_LWIPEDOM       LWIPEDOM        /* Math argument out of domain of func */
#define BOT_LWIPERANGE     LWIPERANGE      /* Math result not representable */
#define BOT_EDEADLK        EDEADLK         /* Resource deadlock would occur */
#define BOT_ENAMETOOLONG   ENAMETOOLONG    /* File name too long */
#define BOT_ENOLCK         ENOLCK          /* No record locks available */
#define BOT_ENOSYS         ENOSYS          /* Function not implemented */
#define BOT_ENOTEMPTY      ENOTEMPTY       /* Directory not empty */
#define BOT_ELOOP          ELOOP           /* Too many symbolic links encountered */
#define BOT_EWOULDBLOCK    EWOULDBLOCK     /* Operation would block */
#define BOT_ENOMSG         ENOMSG          /* No message of desired type */
#define BOT_EIDRM          EIDRM           /* Identifier removed */
#define BOT_ECHRNG         ECHRNG          /* Channel number out of range */
#define BOT_EL2NSYNC       EL2NSYNC        /* Level 2 not synchronized */
#define BOT_EL3HLT         EL3HLT          /* Level 3 halted */
#define BOT_EL3RST         EL3RST          /* Level 3 reset */
#define BOT_ELNRNG         ELNRNG          /* Link number out of range */
#define BOT_EUNATCH        EUNATCH         /* Protocol driver not attached */
#define BOT_ENOCSI         ENOCSI          /* No CSI structure available */
#define BOT_EL2HLT         EL2HLT          /* Level 2 halted */
#define BOT_EBADE          EBADE           /* Invalid exchange */
#define BOT_EBADR          EBADR           /* Invalid request descriptor */
#define BOT_EXFULL         EXFULL          /* Exchange full */
#define BOT_ENOANO         ENOANO          /* No anode */
#define BOT_EBADRQC        EBADRQC         /* Invalid request code */
#define BOT_EBADSLT        EBADSLT         /* Invalid slot */
#define BOT_EDEADLOCK      EDEADLOCK
#define BOT_EBFONT         EBFONT          /* Bad font file format */
#define BOT_ENOSTR         ENOSTR          /* Device not a stream */
#define BOT_ENODATA        ENODATA         /* No data available */
#define BOT_ETIME          ETIME           /* Timer expired */
#define BOT_ENOSR          ENOSR           /* Out of streams resources */
#define BOT_ENONET         ENONET          /* Machine is not on the network */
#define BOT_ENOPKG         ENOPKG          /* Package not installed */
#define BOT_EREMOTE        EREMOTE         /* Object is remote */
#define BOT_ENOLINK        ENOLINK         /* Link has been severed */
#define BOT_EADV           EADV            /* Advertise error */
#define BOT_ESRMNT         ESRMNT          /* Srmount error */
#define BOT_ECOMM          ECOMM           /* Communication error on send */
#define BOT_EPROTO         EPROTO          /* Protocol error */
#define BOT_EMULTIHOP      EMULTIHOP       /* Multihop attempted */
#define BOT_EDOTDOT        EDOTDOT         /* RFS specific error */
#define BOT_EBADMSG        EBADMSG         /* Not a data message */
#define BOT_EOVERFLOW      EOVERFLOW       /* Value too large for defined data type */
#define BOT_ENOTUNIQ       ENOTUNIQ        /* Name not unique on network */
#define BOT_EBADFD         EBADFD          /* File descriptor in bad state */
#define BOT_EREMCHG        EREMCHG         /* Remote address changed */
#define BOT_ELIBACC        ELIBACC         /* Can not access a needed shared library */
#define BOT_ELIBBAD        ELIBBAD         /* Accessing a corrupted shared library */
#define BOT_ELIBSCN        ELIBSCN         /* .lib section in a.out corrupted */
#define BOT_ELIBMAX        ELIBMAX         /* Attempting to link in too many shared libraries */
#define BOT_ELIBEXEC       ELIBEXEC        /* Cannot exec a shared library directly */
#define BOT_LWIPEILSEQ         LWIPEILSEQ      /* Illegal byte sequence */
#define BOT_ERESTART           ERESTART        /* Interrupted system call should be restarted */
#define BOT_ESTRPIPE           ESTRPIPE        /* Streams pipe error */
#define BOT_EUSERS             EUSERS          /* Too many users */
#define BOT_ENOTSOCK           ENOTSOCK        /* Socket operation on non-socket */
#define BOT_EDESTADDRREQ       EDESTADDRREQ    /* Destination address required */
#define BOT_EMSGSIZE           EMSGSIZE        /* Message too long */
#define BOT_EPROTOTYPE         EPROTOTYPE      /* Protocol wrong type for socket */
#define BOT_ENOPROTOOPT        ENOPROTOOPT     /* Protocol not available */
#define BOT_EPROTONOSUPPORT    EPROTONOSUPPORT /* Protocol not supported */
#define BOT_ESOCKTNOSUPPORT    ESOCKTNOSUPPORT /* Socket type not supported */
#define BOT_EOPNOTSUPP         EOPNOTSUPP      /* Operation not supported on transport endpoint */
#define BOT_EPFNOSUPPORT       EPFNOSUPPORT    /* Protocol family not supported */
#define BOT_EAFNOSUPPORT       EAFNOSUPPORT    /* Address family not supported by protocol */
#define BOT_EADDRINUSE         EADDRINUSE      /* Address already in use */
#define BOT_EADDRNOTAVAIL      EADDRNOTAVAIL   /* Cannot assign requested address */
#define BOT_ENETDOWN           ENETDOWN        /* Network is down */
#define BOT_ENETUNREACH        ENETUNREACH     /* Network is unreachable */
#define BOT_ENETRESET          ENETRESET       /* Network dropped connection because of reset */
#define BOT_ECONNABORTED       ECONNABORTED    /* Software caused connection abort */
#define BOT_ECONNRESET         ECONNRESET      /* Connection reset by peer */
#define BOT_ENOBUFS            ENOBUFS         /* No buffer space available */
#define BOT_EISCONN            EISCONN         /* Transport endpoint is already connected */
#define BOT_ENOTCONN           ENOTCONN        /* Transport endpoint is not connected */
#define BOT_ESHUTDOWN          ESHUTDOWN       /* Cannot send after transport endpoint shutdown */
#define BOT_ETOOMANYREFS       ETOOMANYREFS    /* Too many references: cannot splice */
#define BOT_ETIMEDOUT          ETIMEDOUT       /* Connection timed out */
#define BOT_ECONNREFUSED       ECONNREFUSED    /* Connection refused */
#define BOT_EHOSTDOWN          EHOSTDOWN       /* Host is down */
#define BOT_EHOSTUNREACH       EHOSTUNREACH    /* No route to host */
#define BOT_EALREADY           EALREADY        /* Operation already in progress */
#define BOT_EINPROGRESS        EINPROGRESS     /* Operation now in progress */
#define BOT_ESTALE             ESTALE          /* Stale NFS file handle */
#define BOT_EUCLEAN            EUCLEAN         /* Structure needs cleaning */
#define BOT_ENOTNAM            ENOTNAM         /* Not a XENIX named type file */
#define BOT_ENAVAIL            ENAVAIL         /* No XENIX semaphores available */
#define BOT_EISNAM             EISNAM          /* Is a named type file */
#define BOT_EREMOTEIO          EREMOTEIO       /* Remote I/O error */
#define BOT_EDQUOT             EDQUOT          /* Quota exceeded */
#define BOT_ENOMEDIUM          ENOMEDIUM       /* No medium found */
#define BOT_EMEDIUMTYPE        EMEDIUMTYPE     /* Wrong medium type */
#endif /* __BOT_PLATFORM_H */
