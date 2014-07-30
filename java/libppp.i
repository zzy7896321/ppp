/* libppp.i */
%module libppp
%include "typemaps.i"
%apply float *OUTPUT { float * result };
%{
/* Put header files here or function declarations like below */
#include "libppp.h"
%}

%include "libppp.h"
