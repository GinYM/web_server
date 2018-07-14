#include "util.h"



void print_log(const char* format, ...){
    va_list args;
    va_start (args, format);
    vfprintf (f_log, format, args);
    va_end (args);
    return; 

}