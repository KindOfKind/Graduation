
#ifndef RVO_EXPORT_H
#define RVO_EXPORT_H

#ifdef RVO_STATIC_DEFINE
#  define RVO_EXPORT
#  define RVO_NO_EXPORT
#else
#  ifndef RVO_EXPORT
#    ifdef RVO_EXPORTS
        /* We are building this library */
#      define RVO_EXPORT 
#    else
        /* We are using this library */
#      define RVO_EXPORT 
#    endif
#  endif

#  ifndef RVO_NO_EXPORT
#    define RVO_NO_EXPORT 
#  endif
#endif

#ifndef RVO_DEPRECATED
#  define RVO_DEPRECATED __declspec(deprecated)
#endif

#ifndef RVO_DEPRECATED_EXPORT
#  define RVO_DEPRECATED_EXPORT RVO_EXPORT RVO_DEPRECATED
#endif

#ifndef RVO_DEPRECATED_NO_EXPORT
#  define RVO_DEPRECATED_NO_EXPORT RVO_NO_EXPORT RVO_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef RVO_NO_DEPRECATED
#    define RVO_NO_DEPRECATED
#  endif
#endif

#endif /* RVO_EXPORT_H */
