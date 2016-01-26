#pragma once

#if !defined(containerof)
#   include <stddef.h>
#   define containerof(ptr, type, member) \
        ((type *) ((char *)(ptr) - offsetof(type, member))) //(type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member))
#endif 

#if !defined(countof)
#   define countof(a) (sizeof(a) / sizeof(*a))
#endif
