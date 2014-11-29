#ifndef CONFIG_H
#define CONFIG_H

#ifdef PPP_INLINE
#undef PPP_INLINE
#undef PPP_HAS_INLINE
#endif

#ifdef PPP_BOOL
#undef PPP_BOOL
#undef PPP_TRUE
#undef PPP_FALSE
#endif

#if __STDC_VERSION__ >= 199901L
#	define PPP_INLINE inline
#	define PPP_HAS_INLINE 1
#elif (defined(__GNUC__))
#	define PPP_INLINE __inline__
#	define PPP_HAS_INLINE 1
#else	/* __STDC_VERSION < 199901L and !defined(__GNUC__) */
#	define PPP_INLINE 
#	define PPP_HAS_INLINE 0
#endif


#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else

#define bool int
#define false 0
#define true 1
#define __bool_true_false_are_defined 1

#endif


#endif	/* CONFIG_H */

