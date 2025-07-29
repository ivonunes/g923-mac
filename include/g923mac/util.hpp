//
//
//      g923mac
//      util.hpp
//

#pragma once

#include <cstdio>

#include <IOKit/IOReturn.h>

#include <mach/mach_error.h>

#define G923MAC_HELPER_VERSION "0.0.1"


namespace g923mac
{


constexpr void terminal_cmd ( int cmd ) { printf( "\033[%dm", cmd ) ; }

constexpr void terminal_reset  () { terminal_cmd(  0 ) ; }
constexpr void terminal_bold   () { terminal_cmd(  1 ) ; }
constexpr void terminal_faint  () { terminal_cmd(  2 ) ; }
constexpr void terminal_red    () { terminal_cmd( 31 ) ; }
constexpr void terminal_green  () { terminal_cmd( 32 ) ; }
constexpr void terminal_yellow () { terminal_cmd( 33 ) ; }

constexpr char const * terminal_reset_cstr  () { return "\033[0m"  ; }
constexpr char const * terminal_bold_cstr   () { return "\033[1m"  ; }
constexpr char const * terminal_faint_cstr  () { return "\033[2m"  ; }
constexpr char const * terminal_red_cstr    () { return "\033[31m" ; }
constexpr char const * terminal_green_cstr  () { return "\033[32m" ; }
constexpr char const * terminal_yellow_cstr () { return "\033[33m" ; }

constexpr bool _try ( char const * loc, IOReturn result ) noexcept
{
        if( result != kIOReturnSuccess )
        {
                terminal_bold() ;
                terminal_red() ;
                printf( "=== g923mac::error " ) ;
                terminal_reset() ;
                printf( ": %s failed with error code %x (%s)\n", loc, result, mach_error_string( result ) ) ;
                return false ;
        }
        return true ;
}

constexpr void print_info ( char const * message ) noexcept
{
        ( void ) message ;
//      printf( "%s=== g923mac::info : %s%s\n", terminal_faint_cstr(), message, terminal_reset_cstr() ) ;
}

constexpr void print_error ( char const * message ) noexcept
{
        printf( "%s%s=== g923mac::error %s: %s\n", g923mac::terminal_red_cstr(), g923mac::terminal_bold_cstr(), g923mac::terminal_reset_cstr(), message ) ;
}


} // namespace g923mac
