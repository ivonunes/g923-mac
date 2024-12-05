///
///
///     fffb_scs
///     plugin.cpp
///

/// STD

#include <cstdlib>
#include <cassert>
#include <cstdarg>
#include <cstring>

/// SDK

#include <scssdk_telemetry.h>
#include <eurotrucks2/scssdk_eut2.h>
#include <eurotrucks2/scssdk_telemetry_eut2.h>
#include <amtrucks/scssdk_ats.h>
#include <amtrucks/scssdk_telemetry_ats.h>

/// FLT

#include <flt/device.hpp>
#include <flt/wheel.hpp>


bool g_telemetry_paused { true } ;

scs_timestamp_t g_last_timestamp { static_cast< scs_timestamp_t >( -1 ) } ;


struct telemetry_state_t
{
        scs_timestamp_t                       timestamp ;
        scs_timestamp_t         raw_rendering_timestamp ;
        scs_timestamp_t        raw_simulation_timestamp ;
        scs_timestamp_t raw_paused_simulation_timestamp ;

        bool orientation_available ;

        float heading ;
        float   pitch ;
        float    roll ;

        float steering ;
        float throttle ;
        float    brake ;
        float   clutch ;

        float speed ;
        float   rpm ;
        int    gear ;
} ;

telemetry_state_t g_telemetry_state ;

scs_log_t g_game_log { nullptr } ;

flt::wheel g_wheel {} ;


bool init_wheel ()
{
        flt::device_manager manager ;

        flt::vector< flt::hid_device > wheels = manager.find_known_wheels() ;

        for( auto const & device : wheels )
        {
                flt::wheel wheel( device ) ;

                if( !wheel.calibrate() )
                {
                        g_game_log( SCS_LOG_TYPE_warning, "fffb::warning : failed initializing device" ) ;
                }
                else
                {
                        g_game_log( SCS_LOG_TYPE_message, "fffb::info : wheel initialized" ) ;
                        g_wheel = flt::wheel( device ) ;

                        return true ;
                }
        }
        return false ;
}

bool update_leds ( float rpm )
{
        static constexpr uti::u8_t led_0 { 0x00 } ;
        static constexpr uti::u8_t led_1 { 0x01 } ;
        static constexpr uti::u8_t led_2 { 0x03 } ;
        static constexpr uti::u8_t led_3 { 0x07 } ;
        static constexpr uti::u8_t led_4 { 0x0F } ;
        static constexpr uti::u8_t led_5 { 0x1F } ;

        if(      rpm ==   0 ) { return g_wheel.set_led_pattern( led_0 ) ; }
        else if( rpm < 1000 ) { return g_wheel.set_led_pattern( led_1 ) ; }
        else if( rpm < 1300 ) { return g_wheel.set_led_pattern( led_2 ) ; }
        else if( rpm < 1600 ) { return g_wheel.set_led_pattern( led_3 ) ; }
        else if( rpm < 1800 ) { return g_wheel.set_led_pattern( led_4 ) ; }
        else if( rpm < 1900 ) { return g_wheel.set_led_pattern( led_5 ) ; }
        else                  { return g_wheel.set_led_pattern( led_5 ) ; }
}

bool update_forces ( telemetry_state_t const & telemetry )
{
        bool all_passed { true } ;

        uti::u8_t autocenter_slope { 0 } ;
        uti::u8_t autocenter_force { 0 } ;
        uti::u8_t     damper_force { 0 } ;

        if( telemetry.speed < 1 )
        {
                if( telemetry.rpm == 0 ) damper_force = 3 ;
                else                     damper_force = 2 ;
        }
        else
        {
                if(      telemetry.speed < 45 ) { autocenter_slope = 2 ; damper_force = 2 ; }
                else if( telemetry.speed < 75 ) { autocenter_slope = 3 ; damper_force = 1 ; }
                else                            { autocenter_slope = 4 ; damper_force = 0 ; }

                autocenter_force = telemetry.speed * telemetry.speed / 2000.0f * 255 ;

                if( autocenter_force < 24 ) autocenter_force = 24 ;
                if( autocenter_force > 64 ) autocenter_force = 64 ;
        }
        if( damper_force > 0 )
        {
                if( !g_wheel.set_damper( damper_force, damper_force, 0, 0 ) )
                {
                        g_game_log( SCS_LOG_TYPE_error, "fffb : failed setting damper force" ) ;
                        all_passed = false ;
                }
        }
        else
        {
                if( !g_wheel.stop_forces() )
                {
                        g_game_log( SCS_LOG_TYPE_error, "fffb : failed stopping forces" ) ;
                        all_passed = false ;
                }
        }
        if( autocenter_force > 0 )
        {
                if( !g_wheel.enable_autocenter() || !g_wheel.set_autocenter_spring( autocenter_slope, autocenter_slope, autocenter_force ) )
                {
                        g_game_log( SCS_LOG_TYPE_error, "fffb : failed setting autocenter spring force" ) ;
                        all_passed = false ;
                }
        }
        else
        {
                if( !g_wheel.disable_autocenter() )
                {
                        g_game_log( SCS_LOG_TYPE_error, "fffb : failed disabling autocenter spring" ) ;
                        all_passed = false ;
                }
        }
        return all_passed ;
}

bool update_wheel ( telemetry_state_t const & telemetry )
{
        static uti::i32_t ffb_rate       { 16 } ;
        static uti::i32_t ffb_rate_count { 16 } ;

        --ffb_rate_count ;

        if( ffb_rate_count == 0 )
        {
                if( !update_forces( telemetry     ) ) { g_game_log( SCS_LOG_TYPE_error, "update_forces" ) ; return false ; }
                if( !update_leds  ( telemetry.rpm ) ) { g_game_log( SCS_LOG_TYPE_error, "update_leds"   ) ; return false ; }

                ffb_rate_count = ffb_rate ;
        }
        return true ;
}

void deinit_wheel ()
{
        return ;
}


SCSAPI_VOID telemetry_frame_start ( [[ maybe_unused ]] scs_event_t const event, void const * const event_info, [[ maybe_unused ]] scs_context_t const context )
{
        scs_telemetry_frame_start_t const * const info = static_cast< scs_telemetry_frame_start_t const * >( event_info ) ;

        if( g_last_timestamp == static_cast< scs_timestamp_t >( -1 ) )
        {
                g_last_timestamp = info->paused_simulation_time ;
        }
        if( info->flags & SCS_TELEMETRY_FRAME_START_FLAG_timer_restart )
        {
                g_last_timestamp = 0 ;
        }
        g_telemetry_state.timestamp += ( info->paused_simulation_time - g_last_timestamp ) ;
        g_last_timestamp = info->paused_simulation_time ;

        g_telemetry_state.        raw_rendering_timestamp = info->           render_time ;
        g_telemetry_state.       raw_simulation_timestamp = info->       simulation_time ;
        g_telemetry_state.raw_paused_simulation_timestamp = info->paused_simulation_time ;
}

SCSAPI_VOID telemetry_frame_end ( [[ maybe_unused ]] scs_event_t const event, [[ maybe_unused ]] void const * const event_info, [[ maybe_unused ]] scs_context_t const context )
{
        if( g_telemetry_paused )
        {
                g_wheel.stop_forces() ;
                g_wheel.disable_autocenter() ;
                update_leds( 0 ) ;
                return ;
        }
        if( !update_wheel( g_telemetry_state ) )
        {
                g_game_log( SCS_LOG_TYPE_error, "fffb::error : failed updating forces!" ) ;
        }
}

SCSAPI_VOID telemetry_pause ( scs_event_t const event, [[ maybe_unused ]] void const * const event_info, [[ maybe_unused ]] scs_context_t const context )
{
        g_telemetry_paused = ( event == SCS_TELEMETRY_EVENT_paused ) ;

        if( g_telemetry_paused )
        {
                g_wheel.stop_forces() ;
                g_wheel.disable_autocenter() ;
                update_leds( 0 ) ;
                g_game_log( SCS_LOG_TYPE_message, "fffb::info : telemetry paused, stopped forces" ) ;
        }
        else
        {

        }
}

SCSAPI_VOID telemetry_store_orientation ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( context ) ;

        telemetry_state_t * const state = static_cast< telemetry_state_t * >( context ) ;

        if( !value )
        {
                state->orientation_available = false ;
                return ;
        }
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_euler ) ;
        state->orientation_available = true ;
        state->heading = value->value_euler.heading * 360.0f ;
        state->  pitch = value->value_euler.  pitch * 360.0f ;
        state->   roll = value->value_euler.   roll * 360.0f ;
}

SCSAPI_VOID telemetry_store_float ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_float ) ;
        assert( context ) ;
        *static_cast< float * >( context ) = value->value_float.value ;
}

SCSAPI_VOID telemetry_store_s32 ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_s32 ) ;
        assert( context ) ;
        *static_cast< int * >( context ) = value->value_s32.value ;
}

SCSAPI_RESULT scs_telemetry_init ( scs_u32_t const version, scs_telemetry_init_params_t const * const params )
{
        if( version != SCS_TELEMETRY_VERSION_1_01 )
        {
                return SCS_RESULT_unsupported ;
        }
        scs_telemetry_init_params_v101_t const * const version_params = static_cast< scs_telemetry_init_params_v101_t const * >( params ) ;

        g_game_log = version_params->common.log ;

        g_game_log( SCS_LOG_TYPE_message, "fffb::info : starting initialization..." ) ;

        if( strcmp( version_params->common.game_id, SCS_GAME_ID_EUT2 ) == 0 )
        {
                scs_u32_t const MIN_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_1_00 ;
                if( version_params->common.game_version < MIN_VERSION )
                {
                        g_game_log( SCS_LOG_TYPE_warning, "ftl_ffb::warning : game version too old, some stuff might be broken" ) ;
                }

                scs_u32_t const IMPLD_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_CURRENT ;
                if( SCS_GET_MAJOR_VERSION( version_params->common.game_version ) > SCS_GET_MAJOR_VERSION( IMPLD_VERSION ) )
                {
                        g_game_log( SCS_LOG_TYPE_warning, "ftl_ffb::warning : game major version too new, some stuff might be broken" ) ;
                }
        }
        else if( strcmp( version_params->common.game_id, SCS_GAME_ID_ATS ) == 0 )
        {
                scs_u32_t const MIN_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_1_00 ;
                if( version_params->common.game_version < MIN_VERSION )
                {
                        g_game_log( SCS_LOG_TYPE_warning, "ftl_ffb::warning : game version too old, some stuff might be broken" ) ;
                }

                scs_u32_t const IMPLD_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_CURRENT ;
                if( SCS_GET_MAJOR_VERSION( version_params->common.game_version ) > SCS_GET_MAJOR_VERSION( IMPLD_VERSION ) )
                {
                        g_game_log( SCS_LOG_TYPE_warning, "ftl_ffb::warning : game major version too new, some stuff might be broken" ) ;
                }
        }
        else
        {
                g_game_log( SCS_LOG_TYPE_warning, "ftl_ffb::warning : unknown game, some stuff might be broken" ) ;
        }
        g_game_log( SCS_LOG_TYPE_message, "fffb::info : version checks passed" ) ;
        g_game_log( SCS_LOG_TYPE_message, "fffb::info : registering to events..." ) ;

        bool const events_registered =
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_frame_start, telemetry_frame_start, nullptr ) == SCS_RESULT_ok ) &&
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_frame_end  , telemetry_frame_end  , nullptr ) == SCS_RESULT_ok ) &&
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_paused     , telemetry_pause      , nullptr ) == SCS_RESULT_ok ) &&
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_started    , telemetry_pause      , nullptr ) == SCS_RESULT_ok )  ;

        if( !events_registered )
        {
                g_game_log( SCS_LOG_TYPE_error, "fffb::error : failed to register event callbacks" ) ;
                return SCS_RESULT_generic_error ;
        }
        g_game_log( SCS_LOG_TYPE_message, "fffb::info : event registration successful" ) ;
        g_game_log( SCS_LOG_TYPE_message, "fffb::info : registering to channels..." ) ;

        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_world_placement, SCS_U32_NIL, SCS_VALUE_TYPE_euler, SCS_TELEMETRY_CHANNEL_FLAG_no_value, telemetry_store_orientation, &g_telemetry_state       );
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_speed          , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none    , telemetry_store_float      , &g_telemetry_state.speed );
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm     , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none    , telemetry_store_float      , &g_telemetry_state.rpm   );
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear    , SCS_U32_NIL, SCS_VALUE_TYPE_s32  , SCS_TELEMETRY_CHANNEL_FLAG_none    , telemetry_store_s32        , &g_telemetry_state.gear  );

        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_steering, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.steering  );
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_throttle, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.throttle  );
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_brake   , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.brake     );
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_clutch  , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.clutch    );

        g_game_log( SCS_LOG_TYPE_message, "fffb::info : channel registration successful" ) ;

        g_game_log( SCS_LOG_TYPE_message, "fffb::info : initializing wheel..." ) ;
        if( !init_wheel() )
        {
                g_game_log( SCS_LOG_TYPE_error, "ftl_ffb::error : failed to initialize wheel" ) ;
                return SCS_RESULT_generic_error ;
        }
        g_game_log( SCS_LOG_TYPE_message, "fffb::info : wheel initialization successful" ) ;

        memset( &g_telemetry_state, 0, sizeof( g_telemetry_state ) ) ;
        g_last_timestamp = static_cast< scs_timestamp_t >( -1 ) ;

        g_telemetry_paused = true ;

        g_game_log( SCS_LOG_TYPE_message, "fffb::info : successfully initialized" ) ;
        return SCS_RESULT_ok ;
}

SCSAPI_VOID scs_telemetry_shutdown ()
{
        g_game_log = nullptr ;
        deinit_wheel() ;
}

void __attribute__(( destructor )) unload ()
{
        deinit_wheel() ;
}
