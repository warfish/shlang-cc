#pragma once

#if !defined(containerof)
#   define containerof(ptr, type, member) \
        ((type *) ((char *)(ptr) - offsetof(type, member))) //(type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member))
#endif 

