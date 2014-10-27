#ifndef MACRO_UTIL_H
#define MACRO_UTIL_H

#define CONCAT_HELPER(_1, _2) _1 ## _2 
#define CONCAT(_1, _2) CONCAT_HELPER(_1, _2)

#define CONCAT3_HELPER(_1, _2, _3) _1 ## _2 ## _3
#define CONCAT3(_1, _2, _3) CONCAT3_HELPER(_1, _2, _3)

#define STRINGIFY_HELPER(_1) #_1
#define STRINGIFY(_1) STRINGIFY_HELPER(_1)

#endif
