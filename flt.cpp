//
//
//      flt
//      flt.cpp
//

#include <flt/util.hpp>
#include <flt/device.hpp>
#include <flt/wheel.hpp>

#include <uti/string/string_view.hpp>

#include <cstdio>

#define FLT_CMD_QUIT        'q'
#define FLT_CMD_HELP        'h'
#define FLT_CMD_REINIT      'r'

#define FLT_CMD_AUTO        'a'
#define FLT_CMD_AUTO_ON     'y'
#define FLT_CMD_AUTO_OFF    'n'
#define FLT_CMD_AUTO_CONF   'c'

#define FLT_CMD_FORCE_OFF   'x'
#define FLT_CMD_SPRING_CONF 's'
#define FLT_CMD_DAMPER_CONF 'd'
#define FLT_CMD_CONST_CONF  'c'
#define FLT_CMD_TRAP_CONF   't'
#define FLT_CMD_LED_CONF    'l'


void print_help           () ;
void print_faint_prefix   () ;
void print_warning_prefix () ;
void print_error_prefix   () ;

void do_cmd_auto   ( flt::wheel & wheel ) ;
void do_cmd_stop   ( flt::wheel & wheel ) ;
void do_cmd_spring ( flt::wheel & wheel ) ;
void do_cmd_damper ( flt::wheel & wheel ) ;
void do_cmd_const  ( flt::wheel & wheel ) ;
void do_cmd_trap   ( flt::wheel & wheel ) ;
void do_cmd_led    ( flt::wheel & wheel ) ;

int main ()
{
        flt::terminal_bold() ;
        printf( "   //////////////////////\n" ) ;
        printf( "  ///   flt v0.0.1   ///\n"  ) ;
        printf( " //////////////////////\n"   ) ;
        flt::terminal_reset() ;

init:
        flt::terminal_faint() ;
        printf( "/// flt : looking for steering wheels...\n" ) ;
        flt::terminal_reset() ;

        flt::wheel wheel ;
        {
                flt::device_manager manager ;

                auto wheels = manager.find_known_wheels() ;

                for( flt::hid_device & device : wheels )
                {
                        flt::terminal_faint() ;
                        printf( "/// flt : trying device %x...\n", device.device_id_ ) ;
                        flt::terminal_reset() ;

                        if( flt::wheel( device ).calibrate() )
                        {
                                wheel = flt::wheel( device ) ;
                                print_faint_prefix() ;
                                printf( "steering wheel calibrated\n" ) ;
                                break ;
                        }
                        else
                        {
                                print_warning_prefix() ;
                                printf( "calibration failed\n" ) ;
                        }
                }
        }
        if( !wheel )
        {
                print_error_prefix() ;
                printf( "failed connecting to a steering wheel\n" ) ;
                return 1 ;
        }
        char cmd { ' ' } ;

        print_faint_prefix() ;
        printf( "'h' for help\n" ) ;
        print_faint_prefix() ;

        while( cmd != FLT_CMD_QUIT )
        {
                bool reinit = false ;
                scanf( "%c", &cmd ) ;

                switch( cmd )
                {
                        case '\n':
                                break ;
                        case FLT_CMD_QUIT:
                                break ;
                        case FLT_CMD_HELP:
                                print_help() ;
                                break ;
                        case FLT_CMD_REINIT:
                                reinit = true ;
                                break ;
                        case FLT_CMD_AUTO:
                                do_cmd_auto( wheel ) ;
                                break ;
                        case FLT_CMD_FORCE_OFF:
                                do_cmd_stop( wheel ) ;
                                break ;
                        case FLT_CMD_SPRING_CONF:
                                do_cmd_spring( wheel ) ;
                                break ;
                        case FLT_CMD_DAMPER_CONF:
                                do_cmd_damper( wheel ) ;
                                break ;
                        case FLT_CMD_CONST_CONF:
                                do_cmd_const( wheel ) ;
                                break ;
                        case FLT_CMD_TRAP_CONF:
                                do_cmd_trap( wheel ) ;
                                break ;
                        case FLT_CMD_LED_CONF:
                                do_cmd_led( wheel ) ;
                                break ;
                        default:
                                break ;
                }
                if( reinit ) goto init ;
                if( cmd != '\n' && cmd != FLT_CMD_QUIT )
                {
                        print_faint_prefix() ;
                        printf( "" ) ;
                }
        }
        printf( "%s///%s\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        print_faint_prefix() ;
        printf( "quitting...\n" ) ;
        printf( "%s//%s\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s/%s\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;


        return 0 ;
}

void do_cmd_auto ( flt::wheel & wheel )
{
        print_faint_prefix() ;
        printf( "y to enable, n to disable, c to configure : " ) ;

        char cmd { 'n' } ;

        scanf( "%c", &cmd ) ;
        scanf( "%c", &cmd ) ;

        if( cmd == FLT_CMD_AUTO_ON )
        {
                if( !wheel.enable_autocenter() )
                {
                        print_error_prefix() ;
                        printf( "failed enabling autocenter spring\n" ) ;
                        return ;
                }
                print_faint_prefix() ;
                printf( "autocentering spring enabled\n" ) ;
        }
        else if( cmd == FLT_CMD_AUTO_OFF )
        {
                if( !wheel.disable_autocenter() )
                {
                        print_error_prefix() ;
                        printf( "failed disabling autocenter spring\n" ) ;
                        return ;
                }
                print_faint_prefix() ;
                printf( "autocentering spring disabled\n" ) ;
        }
        else if( cmd == FLT_CMD_AUTO_CONF )
        {
                uti::i32_t k1, k2, clip ;

                print_faint_prefix() ;
                printf( "slope left (0..7) : " ) ;
                scanf( "%d", &k1 ) ;

                print_faint_prefix() ;
                printf( "slope right (0..7) : " ) ;
                scanf( "%d", &k2 ) ;

                print_faint_prefix() ;
                printf( "max force (0..255) : " ) ;
                scanf( "%d", &clip ) ;

                if( !wheel.set_autocenter_spring( k1, k2, clip ) )
                {
                        print_error_prefix() ;
                        printf( "failed setting autocenter spring\n" ) ;
                        return ;
                }
                print_faint_prefix() ;
                printf( "autocentering spring set\n" ) ;
        }
}

void do_cmd_stop ( flt::wheel & wheel )
{
        if( !wheel.stop_forces() )
        {
                print_error_prefix() ;
                printf( "failed stopping forces\n" ) ;
                return ;
        }
        print_faint_prefix() ;
        printf( "stopped forces\n" ) ;
}

void do_cmd_spring ( flt::wheel & wheel )
{
        uti::i32_t k1, k2, d1, d2, s1, s2, clip ;

        print_faint_prefix() ;
        printf( "dead band left (0..255) : " ) ;
        scanf( "%d", &d1 ) ;

        print_faint_prefix() ;
        printf( "dead band right (0..255) : " ) ;
        scanf( "%d", &d2 ) ;

        print_faint_prefix() ;
        printf( "slope left (0..7) : " ) ;
        scanf( "%d", &k1 ) ;

        print_faint_prefix() ;
        printf( "slope right (0..7) : " ) ;
        scanf( "%d", &k2 ) ;

        print_faint_prefix() ;
        printf( "slope invert left (0..1) : " ) ;
        scanf( "%d", &s1 ) ;

        print_faint_prefix() ;
        printf( "slope invert right (0..1) : " ) ;
        scanf( "%d", &s2 ) ;

        print_faint_prefix() ;
        printf( "max force (0..255) : " ) ;
        scanf( "%d", &clip ) ;

        if( !wheel.set_custom_spring( d1, d2, k1, k2, s1, s2, clip ) )
        {
                print_error_prefix() ;
                printf( "failed setting custom spring\n" ) ;
                return ;
        }
        print_faint_prefix() ;
        printf( "custom spring set\n" ) ;
}

void do_cmd_damper ( flt::wheel & wheel )
{
        uti::i32_t k1, k2, s1, s2 ;

        print_faint_prefix() ;
        printf( "push left (0..7) : " ) ;
        scanf( "%d", &k1 ) ;

        print_faint_prefix() ;
        printf( "invert left (0..1) : " ) ;
        scanf( "%d", &s1 ) ;

        print_faint_prefix() ;
        printf( "push right (0..7) : " ) ;
        scanf( "%d", &k2 ) ;

        print_faint_prefix() ;
        printf( "invert right (0..1) : " ) ;
        scanf( "%d", &s2 ) ;

        if( !wheel.set_damper( k1, k2, s1, s2 ) )
        {
                print_error_prefix() ;
                printf( "failed setting damper\n" ) ;
                return ;
        }
        print_faint_prefix() ;
        printf( "damper set\n" ) ;
}

void do_cmd_const ( flt::wheel & wheel )
{
        uti::i32_t force ;

        print_faint_prefix() ;
        printf( "force (0..255) : " ) ;
        scanf( "%d", &force ) ;

        if( !wheel.set_constant_force( force ) )
        {
                print_error_prefix() ;
                printf( "failed setting constant force\n" ) ;
                return ;
        }
        print_faint_prefix() ;
        printf( "constant force set\n" ) ;
}

void do_cmd_trap ( flt::wheel & wheel )
{
        uti::i32_t force_max, force_min, t_max, t_min, diff_x, diff_y ;

        print_faint_prefix() ;
        printf( "max force (0..255) : " ) ;
        scanf( "%d", &force_max ) ;

        print_faint_prefix() ;
        printf( "min force (0..255) : " ) ;
        scanf( "%d", &force_min ) ;

        print_faint_prefix() ;
        printf( "time at max force (0..255) : " ) ;
        scanf( "%d", &t_max ) ;

        print_faint_prefix() ;
        printf( "time at min force (0..255) : " ) ;
        scanf( "%d", &t_min ) ;

        print_faint_prefix() ;
        printf( "slope step x (0..15) : " ) ;
        scanf( "%d", &diff_x ) ;

        print_faint_prefix() ;
        printf( "slope step y (0..15) : " ) ;
        scanf( "%d", &diff_y ) ;

        if( !wheel.set_trapezoid( force_max, force_min, t_max, t_min, diff_x, diff_y ) )
        {
                print_error_prefix() ;
                printf( "failed setting trapezoid force\n" ) ;
                return ;
        }
        print_faint_prefix() ;
        printf( "trapezoid force set\n" ) ;
}

void do_cmd_led ( flt::wheel & wheel )
{
        uti::i32_t led_pattern ;

        print_faint_prefix() ;
        printf( "led pattern (0..31) : " ) ;
        scanf( "%d", &led_pattern ) ;

        if( !wheel.set_led_pattern( led_pattern ) )
        {
                print_error_prefix() ;
                printf( "failed setting led pattern\n" ) ;
                return ;
        }
        print_faint_prefix() ;
        printf( "led pattern set\n" ) ;
}

void print_help ()
{
        printf( "%s///%s\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s flt - fuck logitech - v0.0.1\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s commands :\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    q - quit\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    h - help\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    r - reinitialize wheel\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    a - configure autocentering\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    x - stop forces\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    s - configure spring force\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    d - configure damper force\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    c - configure constant force\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    t - configure trapezoid force\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s    l - configure leds\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
        printf( "%s///%s\n", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
}

void print_faint_prefix ()
{
        printf( "%s/// flt %s: ", flt::terminal_faint_cstr(), flt::terminal_reset_cstr() ) ;
}

void print_warning_prefix ()
{
        printf( "%s/// flt::warning %s: ", flt::terminal_yellow_cstr(), flt::terminal_reset_cstr() ) ;
}

void print_error_prefix ()
{
        printf( "%s/// flt::error %s: ", flt::terminal_red_cstr(), flt::terminal_reset_cstr() ) ;
}
