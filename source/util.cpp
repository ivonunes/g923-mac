//
//
//      flt
//      util.cpp
//

#include <util.hpp>

#include <cstdio>

#include <mach/mach_error.h>


namespace flt
{


void terminal_cmd ( int cmd ) { printf( "\033[%dm", cmd ) ; }

void terminal_reset  () { terminal_cmd(  0 ) ; }
void terminal_bold   () { terminal_cmd(  1 ) ; }
void terminal_faint  () { terminal_cmd(  2 ) ; }
void terminal_red    () { terminal_cmd( 31 ) ; }
void terminal_green  () { terminal_cmd( 32 ) ; }
void terminal_yellow () { terminal_cmd( 33 ) ; }

char const * terminal_reset_cstr  () { return "\033[0m"  ; }
char const * terminal_bold_cstr   () { return "\033[1m"  ; }
char const * terminal_faint_cstr  () { return "\033[2m"  ; }
char const * terminal_red_cstr    () { return "\033[31m" ; }
char const * terminal_green_cstr  () { return "\033[32m" ; }
char const * terminal_yellow_cstr () { return "\033[33m" ; }

bool _try ( char const * loc, IOReturn result ) noexcept
{
        if( result != kIOReturnSuccess )
        {
                terminal_bold() ;
                terminal_red() ;
                printf( "=== flt::error " ) ;
                terminal_reset() ;
                printf( ": %s failed with error code %x (%s)\n", loc, result, mach_error_string( result ) ) ;
                return false ;
        }
        return true ;
}

void print_info ( char const * message ) noexcept
{
        ( void ) message ;
//      printf( "%s=== flt::info : %s%s\n", terminal_faint_cstr(), message, terminal_reset_cstr() ) ;
}

void print_error ( char const * message ) noexcept
{
        printf( "%s%s=== flt::error %s: %s\n", flt::terminal_red_cstr(), flt::terminal_bold_cstr(), flt::terminal_reset_cstr(), message ) ;
}


} // namespace flt
