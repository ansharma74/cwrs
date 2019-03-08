/* portability defines */
#if defined(__FreeBSD__)
# define OS_FreeBSD
# define ARCH "FreeBSD"
#elif defined(__OpenBSD__)
# define OS_OpenBSD
# define ARCH "OpenBSD"
#elif defined(__NetBSD__)
# define OS_NetBSD
# define ARCH "NetBSD"
#elif defined(linux)
# define OS_Linux
# define ARCH "Linux"
#elif defined(sun)
# define OS_Solaris
# define ARCH "Solaris"
#elif defined(__svr4__)
# define OS_SysV
# define ARCH "SysV"
#else
# define OS_UNKNOWN
# define ARCH "UNKNOWN"
#endif
