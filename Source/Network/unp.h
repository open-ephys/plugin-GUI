// Unix network programming unc.h standard header
#ifndef __unp_h
#define __unp_h

// Skipping this line until we have a good makefile
// include "../config.h" 

#include <sys/types.h>   // basic system datatypes
#include <sys/socket.h>  // basic socket definitions
#include <sys/time.h>    // timeval{} for select()
#include <time.h>        // timespec{} for pselect()
#include <netinet/in.h>  // sockaddy_in{} and other Internet defns
#include <arpa/inet.h>   // inet(3) functions
#include <errno.h>
#include <fcntl.h>       // for nonblocking
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>    // for S_xxx file mode constants
#include <sys/uio.h>     // for iovec{} and readv/writev
#include <unistd.h>
#include <sys/wait.h>    // for unix domain sockets
#include <sys/un.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>  // for convenience
#endif

#ifdef HAVE_POLL_H
#include <poll.h>        // for convenience
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>     // for convenience
#endif

// Three headers are normally neeed for socket/file ioctl's:
// <sys/ioctl.h>, <sys/filio.h>, and <sys/sockio.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

// 110skipping this because nidaqmx is finicky about pthread
// ifdef HAVE_PTHREAD_H
// include <pthread.h>
// endif



 #ifdef __osf__ 
 #undef recv 
 #udef send 
 #define recv(a,b,c,d)   recvfrom(a,b,c,d,0,0) 
 #define send(a,b,c,d)   sendto(a,b,c,d,0,0) 
 #endif 

 #ifndef INADDR_NONE 
 #define INADDR_NONE 0xffffffff // should have been in <netinet/in.h> 
 #endif 

 #ifndef SHUT_RD                // these three Posix.1g names are quite new 
 #define SHUT_RD    0           // shutdown for reading 
 #define SHUT_WR    1           // shutdown for writing 
 #define SHUT_RDWR  2           // shutdown for reading and writing 
 #endif 
 #ifndef INET_ADDRSTRLEN 
 #define INET_ADDRSTRLEN    16   // "ddd.ddd.ddd.ddd\0" 
 #endif 

 // define following even if IPv6 not supported, so we can 
 // always allocate an adequately-sized buffer,  
 // without #ifdefs in the code 
 #ifndef INET6_ADDRSTRLEN 
 #define INET6_ADDRSTRLEN    46 
 #endif 

 // define bzero() as a macro if it's not in standard C library 
 #ifndef HAVE_BZERO 
 #define bzero(ptr,n)    memset(ptr,0,n) 
 #endif 

 // older resolvers don't have gethostbyname2() 
 #ifndef HAVE_GETHOSTBYNAME2 
 #define gethostbyname2(host,family)    gethostbyname((host)) 
 #endif 

 // The structure returned by recvfrom_flags() 
 //struct in_pktinfo{ 
 //  struct in_addr ipi_addr;    // dst IPv4 address 
 //  int            ipi_ifindex; // received interface index 
 //}; 

 // We need the newer CMSG_LEN() and CMSG_SPACE() macros, but few 
 // implementations support them today.  These two macros really 
 // need an ASIGN() macro, but each implementation does this differently 
 #ifndef CMSG_LEN 
 #define CMSG_LEN(size)   (sizeof(struct cmsghdr) + (size)) 
 #endif 
 #ifndef CMSG_SPACE 
 #define CMSG_SPACE(size) (sizeof(struct cmsghdr) + (size)) 
 #endif 

 // Posix.1g requires the SUN_LEN() macro but not all implementations define 
 // it (yet). Note that this 4.4BSD macro works regardless whether there is 
 // a length field or not 
 #ifndef SUN_LEN 
 #define SUN_LEN(su)  (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path)) 
 #endif 

 // Posix.1g renames "Unix domain" as "local IPC" 
 // But not all systems define AF_LOCAL and PF_LOCAL (yet) 
 #ifndef AF_LOCAL 
 #define AF_LOCAL    AF_UNIX 
 #endif 
 #ifndef PF_LOCAL 
 #define PF_LOCAL    PF_UNIX 
 #endif 




 // Posix.1g requires that an #include of <poll.h> define INFTIM, but many 
 // systems still define it in <sys/stropts.h>  We don't want to include 
 // all the streams stuff if it's not needed, so we just define INFTIM here. 
 // This is the standard value, but there's no guarentee it is -1 
 #ifndef INFTIM 
 #define INFTIM    (-1)    // infinite poll timeout 
 #ifdef HAVE_POLL_H 
 #define INFTIM_UNPH       // tell unpxti.h we defined it 
 #endif 
 #endif 

 // Following could be derived from SOMAXCONN in  <sys/socket.h>, but many 
 // kernels still #define it as 5, while actually supporting many more 
 #define LISTENQ    1024 

 // Miscillaneous constants 
 #define MAXLINE       4096   // max text line length 
 #define MAXSOCKADDR    128   // max socket address structure size 
 #define BUFFSIZE      8192   // buffer size for reads and writes 




 // Define some port number that can be used for client-servers 
 #define SERV_PORT      9877   // TCP and UDP client-servers 
 #define SERV_PORT_STR "9877"  // TCP and UDP client-servers 
 #define UNIXSTR_PATH  "/tmp/unix.str"  // Unix domain stream cli-serv 
 #define UNIXDG_PATH   "/tmp/unix.dg"   // Unix domain datagram cli-serv 

 // Following shortens all the type casts of pointer arguments 
 #define SA struct sockaddr 

 #define FILE_MODE    (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) 
 //                    default file access permissions for new files 
 #define DIR_MODE     (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)  

 typedef void Sigfunc (int);   // for signal handlers 

// Commented b/c these guys break the compile
 #define MIN(a,b)   ((a) < (b) ? (a) : (b)) 
 #define MAX(a,b)   ((a) > (b) ? (a) : (b)) 

/* // will this work? */
/* //#ifndef HAVE_ADDRINFO_STRUCT */
/* //#include "../lib/addrinfo.h" */
/* //#endif */

 #ifndef HAVE_IF_NAMEINDEX_STRUCT 
 struct if_nameindex { 
   unsigned int if_index;   // 1, 2, ... 
   char *if_name;           // sull terminated name: "1e0", ... 
 }; 
 #endif 




//#ifndef HAVE_TIMESPEC_STRUCT
//struct timespec{
//  time_t tv_sec;   // seconds
//  long   nv_nset;  // and nanoseconds
//};
//#endif

#endif
