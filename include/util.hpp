//
//
//      flt
//      util.hpp
//

#pragma once

#include <IOKit/IOReturn.h>


namespace flt
{


void terminal_reset  () ;
void terminal_bold   () ;
void terminal_faint  () ;
void terminal_red    () ;
void terminal_green  () ;
void terminal_yellow () ;

char const * terminal_reset_cstr  () ;
char const * terminal_bold_cstr   () ;
char const * terminal_faint_cstr  () ;
char const * terminal_red_cstr    () ;
char const * terminal_green_cstr  () ;
char const * terminal_yellow_cstr () ;

bool _try ( char const * loc, IOReturn result ) noexcept ;

void print_info  ( char const * message ) noexcept ;
void print_error ( char const * message ) noexcept ;


} // namespace flt
