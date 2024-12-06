//
//
//      flt
//      wheel.hpp
//

#pragma once

#include <util.hpp>
#include <types.hpp>
#include <command.hpp>
#include <device.hpp>

#include <ctime>

#include <mach/mach_error.h>

#define FLT_G923_DEV_ID 0xc266046d


namespace flt
{


class wheel
{
public:
        constexpr wheel (                           ) noexcept : device_{ 0, 0, 0, nullptr } {}
        constexpr wheel ( hid_device const & device ) noexcept : device_(           device ) {}

        constexpr device_id_t  vendor_id () const noexcept { return device_. vendor_id_ ; }
        constexpr device_id_t product_id () const noexcept { return device_.product_id_ ; }
        constexpr device_id_t  device_id () const noexcept { return device_. device_id_ ; }

        constexpr hid_device_t       * device_ref ()       noexcept { return device_.hid_device_ ; }
        constexpr hid_device_t const * device_ref () const noexcept { return device_.hid_device_ ; }

        constexpr hid_device       & device ()       noexcept { return device_ ; }
        constexpr hid_device const & device () const noexcept { return device_ ; }

        constexpr operator bool () const noexcept { return device_.hid_device_ != nullptr ; }

        constexpr bool calibrate () noexcept
        {
                if( !set_led_pattern( 0 ) ) return false ;

                for( int i = 0; i < 32; ++i )
                {
                        usleep( 30 * 1000 ) ;
                        set_led_pattern( i ) ;
                }
                for( int i = 31; i >= 0; --i )
                {
                        usleep( 30 * 1000 ) ;
                        set_led_pattern( i ) ;
                }
                disable_autocenter() ;
                set_constant_force( 120 ) ;
                usleep( 500 * 1000 ) ;
                stop_forces() ;
                set_autocenter_spring( 2, 2, 48 ) ;

                if( !enable_autocenter() ) return false ;

                usleep( 500 * 1000 ) ;

                return true ;
        }

        constexpr bool disable_autocenter () noexcept
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF5 ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'disable autocenter' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool enable_autocenter () noexcept
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF4 ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'enable autocenter' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool set_autocenter_spring ( uti::u8_t k1, uti::u8_t k2, uti::u8_t clip ) noexcept
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xFE ;
                                rep.cmd[ 1 ] = 0x00 ;
                                rep.cmd[ 2 ] =   k1 ;
                                rep.cmd[ 3 ] =   k2 ;
                                rep.cmd[ 4 ] = clip ;
                                rep.cmd[ 5 ] = 0x00 ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'set autocenter spring' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool set_custom_spring ( uti::u8_t d1, uti::u8_t d2, uti::u8_t k1, uti::u8_t k2, uti::u8_t s1, uti::u8_t s2, uti::u8_t clip )
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF1 ;
                                rep.cmd[ 1 ] = 0x01 ;
                                rep.cmd[ 2 ] = d1 ;
                                rep.cmd[ 3 ] = d2 ;
                                rep.cmd[ 4 ] = ( k2 << 4 ) | k1 ;
                                rep.cmd[ 5 ] = ( s2 << 4 ) | s1 ;
                                rep.cmd[ 6 ] = clip ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'set custom spring' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool set_constant_force ( uti::u8_t force_level )
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF1 ;
                                rep.cmd[ 1 ] = 0x00 ;
                                rep.cmd[ 2 ] = force_level ;
                                rep.cmd[ 3 ] = force_level ;
                                rep.cmd[ 4 ] = force_level ;
                                rep.cmd[ 5 ] = force_level ;
                                rep.cmd[ 6 ] = 0x00 ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'set constant force' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool set_damper ( uti::u8_t k1, uti::u8_t k2, uti::u8_t s1, uti::u8_t s2 )
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF1 ;
                                rep.cmd[ 1 ] = 0x02 ;
                                rep.cmd[ 2 ] = k1 ;
                                rep.cmd[ 3 ] = s1 ;
                                rep.cmd[ 4 ] = k2 ;
                                rep.cmd[ 5 ] = s2 ;
                                rep.cmd[ 6 ] = 0x00 ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'set damper' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool set_trapezoid ( uti::u8_t l1, uti::u8_t l2, uti::u8_t t1, uti::u8_t t2, uti::u8_t t3, uti::u8_t s )
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF1 ;
                                rep.cmd[ 1 ] = 0x06 ;
                                rep.cmd[ 2 ] = l1 ;
                                rep.cmd[ 3 ] = l2 ;
                                rep.cmd[ 4 ] = t1 ;
                                rep.cmd[ 5 ] = t2 ;
                                rep.cmd[ 6 ] = ( t3 << 4 ) | s ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'set trapezoid' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool stop_forces ()
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF3 ;
                                rep.cmd[ 1 ] = 0x00 ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'stop forces' command..." ) ;

                return _send_report( rep ) ;
        }

        constexpr bool set_led_pattern ( uti::u8_t pattern )
        {
                report rep ;

                switch( device_.device_id_ )
                {
                        case FLT_G923_DEV_ID:
                                rep.cmd[ 0 ] = 0xF8 ;
                                rep.cmd[ 1 ] = 0x12 ;
                                rep.cmd[ 2 ] = pattern ;
                                break ;
                        default:
                                /// unreachable
                                break ;
                }
                print_info( "sending 'set led pattern' command..." ) ;

                return _send_report( rep ) ;
        }
private:
        hid_device device_ ;

        constexpr bool _send_report ( report const & rep ) noexcept
        {
                if( !_try( "_send_report::open_device", open_device( device_      ) ) ) {                           return false ; }
                if( !_try( "_send_report"             , send_report( device_, rep ) ) ) { close_device( device_ ) ; return false ; }

                close_device( device_ ) ;

                print_info( "_send_report successful" ) ;
                return true ;
        }
} ;


} // namespace flt
