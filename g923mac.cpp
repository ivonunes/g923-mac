//
//
//      g923mac
//      g923mac.cpp
//

#include <g923mac/util.hpp>
#include <g923mac/device.hpp>
#include <g923mac/wheel.hpp>

#include <string_view>

#include <cstdio>

#define G923MAC_HELPER_CMD_QUIT        'q'
#define G923MAC_HELPER_CMD_HELP        'h'
#define G923MAC_HELPER_CMD_REINIT      'r'

#define G923MAC_HELPER_CMD_AUTO        'a'
#define G923MAC_HELPER_CMD_AUTO_ON     'y'
#define G923MAC_HELPER_CMD_AUTO_OFF    'n'
#define G923MAC_HELPER_CMD_AUTO_CONF   'c'

#define G923MAC_HELPER_CMD_FORCE_OFF   'x'
#define G923MAC_HELPER_CMD_SPRING_CONF 's'
#define G923MAC_HELPER_CMD_DAMPER_CONF 'd'
#define G923MAC_HELPER_CMD_CONST_CONF  'c'
#define G923MAC_HELPER_CMD_TRAP_CONF   't'
#define G923MAC_HELPER_CMD_LED_CONF    'l'


void print_help           () ;
void print_faint_prefix   () ;
void print_warning_prefix () ;
void print_error_prefix   () ;

void do_cmd_auto   ( g923mac::wheel & wheel ) ;
void do_cmd_stop   ( g923mac::wheel & wheel ) ;
void do_cmd_spring ( g923mac::wheel & wheel ) ;
void do_cmd_damper ( g923mac::wheel & wheel ) ;
void do_cmd_const  ( g923mac::wheel & wheel ) ;
void do_cmd_trap   ( g923mac::wheel & wheel ) ;
void do_cmd_led    ( g923mac::wheel & wheel ) ;

int main ()
{
        g923mac::terminal_bold() ;
        printf( "   //////////////////////\n" ) ;
        printf( "  ///   g923mac v" G923MAC_HELPER_VERSION "   ///\n"  ) ;
        printf( " //////////////////////\n"   ) ;
        g923mac::terminal_reset() ;

init:
        g923mac::terminal_faint() ;
        printf( "/// g923mac : looking for steering wheels...\n" ) ;
        g923mac::terminal_reset() ;

        g923mac::wheel wheel ;
        {
                g923mac::device_manager manager ;

                auto wheels = manager.find_known_wheels() ;

                for( g923mac::hid_device & device : wheels )
                {
                        g923mac::terminal_faint() ;
                        printf( "/// g923mac : trying device %x...\n", device.device_id_ ) ;
                        g923mac::terminal_reset() ;

                        if( g923mac::wheel( device ).calibrate() )
                        {
                                wheel = g923mac::wheel( device ) ;
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

        while( cmd != G923MAC_HELPER_CMD_QUIT )
        {
                bool reinit = false ;
                scanf( "%c", &cmd ) ;

                switch( cmd )
                {
                        case '\n':
                                break ;
                        case G923MAC_HELPER_CMD_QUIT:
                                break ;
                        case G923MAC_HELPER_CMD_HELP:
                                print_help() ;
                                break ;
                        case G923MAC_HELPER_CMD_REINIT:
                                reinit = true ;
                                break ;
                        case G923MAC_HELPER_CMD_AUTO:
                                do_cmd_auto( wheel ) ;
                                break ;
                        case G923MAC_HELPER_CMD_FORCE_OFF:
                                do_cmd_stop( wheel ) ;
                                break ;
                        case G923MAC_HELPER_CMD_SPRING_CONF:
                                do_cmd_spring( wheel ) ;
                                break ;
                        case G923MAC_HELPER_CMD_DAMPER_CONF:
                                do_cmd_damper( wheel ) ;
                                break ;
                        case G923MAC_HELPER_CMD_CONST_CONF:
                                do_cmd_const( wheel ) ;
                                break ;
                        case G923MAC_HELPER_CMD_TRAP_CONF:
                                do_cmd_trap( wheel ) ;
                                break ;
                        case G923MAC_HELPER_CMD_LED_CONF:
                                do_cmd_led( wheel ) ;
                                break ;
                        default:
                                break ;
                }
                if( reinit ) goto init ;
                if( cmd != '\n' && cmd != G923MAC_HELPER_CMD_QUIT )
                {
                        print_faint_prefix() ;
                        printf( "" ) ;
                }
        }
        printf( "%s///%s\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        print_faint_prefix() ;
        printf( "quitting...\n" ) ;
        printf( "%s//%s\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s/%s\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;


        return 0 ;
}

void do_cmd_auto ( g923mac::wheel & wheel )
{
        print_faint_prefix() ;
        printf( "y to enable, n to disable, c to configure : " ) ;

        char cmd { 'n' } ;

        scanf( "%c", &cmd ) ;
        scanf( "%c", &cmd ) ;

        if( cmd == G923MAC_HELPER_CMD_AUTO_ON )
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
        else if( cmd == G923MAC_HELPER_CMD_AUTO_OFF )
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
        else if( cmd == G923MAC_HELPER_CMD_AUTO_CONF )
        {
                std::int32_t k1, k2, clip ;

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

void do_cmd_stop ( g923mac::wheel & wheel )
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

void do_cmd_spring ( g923mac::wheel & wheel )
{
        std::int32_t k1, k2, d1, d2, s1, s2, clip ;

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

void do_cmd_damper ( g923mac::wheel & wheel )
{
        std::int32_t k1, k2, s1, s2 ;

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

void do_cmd_const ( g923mac::wheel & wheel )
{
        std::int32_t force ;

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

void do_cmd_trap ( g923mac::wheel & wheel )
{
        std::int32_t force_max, force_min, t_max, t_min, diff_x, diff_y ;

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

void do_cmd_led ( g923mac::wheel & wheel )
{
        std::int32_t led_pattern ;

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
        printf( "%s///%s\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s g923mac - fuck logitech - v" G923MAC_HELPER_VERSION "\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s commands :\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    q - quit\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    h - help\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    r - reinitialize wheel\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    a - configure autocentering\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    x - stop forces\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    s - configure spring force\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    d - configure damper force\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    c - configure constant force\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    t - configure trapezoid force\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s    l - configure leds\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
        printf( "%s///%s\n", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
}

void print_faint_prefix ()
{
        printf( "%s/// g923mac %s: ", g923mac::terminal_faint_cstr(), g923mac::terminal_reset_cstr() ) ;
}

void print_warning_prefix ()
{
        printf( "%s/// g923mac::warning %s: ", g923mac::terminal_yellow_cstr(), g923mac::terminal_reset_cstr() ) ;
}

void print_error_prefix ()
{
        printf( "%s/// g923mac::error %s: ", g923mac::terminal_red_cstr(), g923mac::terminal_reset_cstr() ) ;
}
