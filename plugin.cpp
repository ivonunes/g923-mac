///
///
///     g923mac_scs
///     plugin.cpp
///

// STD
#include <cstdlib>
#include <cassert>
#include <cstdarg>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <tuple>


// SDK
#include <scssdk_telemetry.h>
#include <eurotrucks2/scssdk_eut2.h>
#include <eurotrucks2/scssdk_telemetry_eut2.h>
#include <amtrucks/scssdk_ats.h>
#include <amtrucks/scssdk_telemetry_ats.h>


/// G923MAC
#include <g923mac/device.hpp>
#include <g923mac/wheel.hpp>
#include <g923mac/force_feedback_config.hpp>


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
        float input_steering ;
        float throttle ;
        float    brake ;
        float   clutch ;

        float speed ;
        float   rpm ;
        int    gear ;

        float linear_velocity_x ;      // lateral velocity
        float linear_velocity_y ;      // vertical velocity  
        float linear_velocity_z ;      // longitudinal velocity
        float angular_velocity_x ;     // roll rate
        float angular_velocity_y ;     // pitch rate
        float angular_velocity_z ;     // yaw rate
        float linear_acceleration_x ;  // lateral acceleration
        float linear_acceleration_y ;  // vertical acceleration
        float linear_acceleration_z ;  // longitudinal acceleration
        float angular_acceleration_x ; // roll acceleration
        float angular_acceleration_y ; // pitch acceleration
        float angular_acceleration_z ; // yaw acceleration

        bool parking_brake ;
        bool motor_brake ;
        std::uint32_t retarder_level ;
        float brake_air_pressure ;
        float cruise_control ;
        float fuel_amount ;
        bool engine_enabled ;
        
        // Terrain impact tracking
        float last_vertical_acceleration ;
        float terrain_impact_timer ;
        float terrain_smoothed_roughness ;
} ;

telemetry_state_t g_telemetry_state ;

scs_log_t g_game_log { nullptr } ;

g923mac::vector< g923mac::wheel > g_wheels {} ;

// Global terrain state for persistence between frames
struct terrain_state_t
{
        float smoothed_roughness ;
        float impact_timer ;
        float last_vertical_accel ;
        float impact_cooldown ; // Prevent repeated impact detection
} ;

terrain_state_t g_terrain_state {} ;


bool init_wheels ()
{
        g923mac::device_manager manager ;

        g923mac::vector< g923mac::hid_device > wheels = manager.find_known_wheels() ;

        g_wheels.clear() ;

        for( auto const & device : wheels )
        {
                g923mac::wheel wheel( device ) ;

                if( !wheel.calibrate() )
                {
                        g_game_log( SCS_LOG_TYPE_warning, "g923mac::warning : failed initializing device" ) ;
                }
                else
                {
                        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : wheel initialized" ) ;
                        g_wheels.emplace_back( device ) ;
                }
        }
        return !g_wheels.empty() ;
}

bool update_leds ( g923mac::wheel & wheel, float rpm, float speed, float brake, bool parking_brake )
{
        static constexpr std::uint8_t led_0 { 0x00 } ;
        static constexpr std::uint8_t led_1 { 0x01 } ;
        static constexpr std::uint8_t led_2 { 0x03 } ;
        static constexpr std::uint8_t led_3 { 0x07 } ;
        static constexpr std::uint8_t led_4 { 0x0F } ;
        static constexpr std::uint8_t led_5 { 0x1F } ;

        using config = g923mac::ffb_config ;

        // Enhanced LED patterns based on driving conditions
        if( parking_brake )
        {
                // Flash all LEDs when parking brake is engaged
                static bool flash_state = false ;
                flash_state = !flash_state ;
                return wheel.set_led_pattern( flash_state ? led_5 : led_0 ) ;
        }
        
        if( brake > config::led_brake_threshold )
        {
                // Show braking intensity
                if(      brake > config::led_heavy_brake  ) { return wheel.set_led_pattern( led_5 ) ; }
                else if( brake > config::led_medium_brake ) { return wheel.set_led_pattern( led_4 ) ; }
                else                                        { return wheel.set_led_pattern( led_3 ) ; }
        }
        
        // Standard RPM-based LED pattern with enhanced ranges
        float const speed_kmh = speed * 3.6f ;
        float rpm_threshold_base = config::led_rpm_base ;
        
        // Adjust RPM thresholds based on speed for more realistic feel
        if( speed_kmh > config::led_speed_high_threshold )
        {
                rpm_threshold_base = config::led_rpm_highway ; // Lower threshold at highway speeds
        }
        else if( speed_kmh < config::led_speed_low_threshold )
        {
                rpm_threshold_base = config::led_rpm_city ; // Higher threshold at low speeds
        }

        if(      rpm ==   0                                         ) { return wheel.set_led_pattern( led_0 ) ; }
        else if( rpm < rpm_threshold_base                           ) { return wheel.set_led_pattern( led_1 ) ; }
        else if( rpm < rpm_threshold_base + config::led_rpm_step1   ) { return wheel.set_led_pattern( led_2 ) ; }
        else if( rpm < rpm_threshold_base + config::led_rpm_step2   ) { return wheel.set_led_pattern( led_3 ) ; }
        else if( rpm < rpm_threshold_base + config::led_rpm_step3   ) { return wheel.set_led_pattern( led_4 ) ; }
        else if( rpm < rpm_threshold_base + config::led_rpm_step4   ) { return wheel.set_led_pattern( led_5 ) ; }
        else                                                          { 
                // Flash at redline
                static bool redline_flash = false ;
                redline_flash = !redline_flash ;
                return wheel.set_led_pattern( redline_flash ? led_5 : led_4 ) ;
        }
}

std::uint8_t map_rpm_to_freq ( float rpm )
{
        return ( 255 - ( rpm / 3000.0f * 255.0f ) ) / 4 ;
}

std::tuple< std::uint8_t, std::uint8_t > calculate_resonance_params ( float speed, float rpm, float throttle )
{
        std::uint8_t amplitude { 0 } ;
        std::uint8_t frequency { 0 } ;

        if( rpm > 1 )
        {
                frequency = map_rpm_to_freq( rpm ) ;

                if(      speed <  5 ) { amplitude = 3 ;}
                else if( speed < 45 ) { amplitude = 2 ;}
                else if( speed < 75 ) { amplitude = 1 ;}
                else                  { amplitude = 0 ;}

                if(      throttle < 0.10f ) { amplitude -= 1 ; }
                else if( throttle < 0.25f ) {                  }
                else if( throttle < 0.50f ) { amplitude += 1 ; }
                else if( throttle < 0.75f ) { amplitude += 2 ; }
                else                        { amplitude += 3 ; }
        }
        return std::tuple{ amplitude, frequency } ;
}

// Enhanced force feedback calculations for realistic truck simulation
struct force_feedback_params_t 
{
        std::uint8_t autocenter_force ;
        std::uint8_t autocenter_slope ;
        std::uint8_t damper_force_pos ;
        std::uint8_t damper_force_neg ;
        std::uint8_t constant_force ;
        bool use_constant_force ;
        std::uint8_t spring_k1 ;
        std::uint8_t spring_k2 ;
        std::uint8_t spring_clip ;
        bool use_custom_spring ;
} ;

force_feedback_params_t calculate_enhanced_forces ( telemetry_state_t const & telemetry )
{
        force_feedback_params_t params {} ;
        
        using config = g923mac::ffb_config ;
        
        float const speed_kmh = telemetry.speed * 3.6f ; // Convert m/s to km/h
        float const abs_speed = std::abs( telemetry.speed ) ;
        // float const steering_input = telemetry.input_steering ; // Currently unused
        float const effective_steering = telemetry.steering ;
        
        // Calculate tire load transfer effects
        float const lateral_g = telemetry.linear_acceleration_x / 9.81f ;
        // float const longitudinal_g = telemetry.linear_acceleration_z / 9.81f ; // Currently unused
        
        // Enhanced terrain detection using multiple acceleration sources
        float const vertical_acceleration = telemetry.linear_acceleration_y ;
        float const abs_vertical_accel = std::abs( vertical_acceleration ) ;
        
        // Also check lateral acceleration for additional surface variation detection - DISABLED to prevent road shaking
        // float const lateral_variation = std::abs( telemetry.linear_acceleration_x ) ;
        // float const combined_variation = std::max( abs_vertical_accel, lateral_variation * 0.5f ) ;
        // These are now disabled to prevent turning-induced shaking
        
        // Detect sudden impacts (curbs, potholes, road edges) - ONLY use vertical acceleration for terrain detection
        float const accel_change = std::abs( vertical_acceleration - g_terrain_state.last_vertical_accel ) ;
        
        // Add intelligent filtering to avoid normal driving vibrations
        bool const is_high_speed = abs_speed > 40.0f ; // High speed driving
        bool const is_turning = std::abs( telemetry.angular_velocity_y ) > 0.1f ; // Currently turning (using correct field name)
        bool const is_accelerating = std::abs( telemetry.linear_acceleration_z ) > 1.0f ; // Accelerating/braking
        
        // Use stricter thresholds during normal driving to avoid false triggers
        float impact_threshold = config::terrain_minor_threshold * 5.0f ; // Much higher base threshold 
        // float roughness_threshold = config::terrain_minor_threshold * 3.0f ; // Higher base threshold for bumps - UNUSED
        
        // Increase thresholds during conditions that cause normal vibrations
        if( is_high_speed ) {
                impact_threshold *= 3.0f ; // Much stricter at high speed
                // roughness_threshold *= 2.5f ; // UNUSED
        }
        if( is_turning ) {
                impact_threshold *= 2.5f ; // Much stricter when turning  
                // roughness_threshold *= 2.0f ; // UNUSED
        }
        if( is_accelerating ) {
                impact_threshold *= 2.0f ; // Much stricter when accelerating
                // roughness_threshold *= 1.5f ; // UNUSED
        }
        
        bool const sudden_impact = ( accel_change > impact_threshold ) &&  // Must exceed dynamic threshold AND
                                   ( ( abs_vertical_accel > ( impact_threshold * 2.0f ) ) ||  // High upward/downward acceleration OR
                                     ( std::abs( vertical_acceleration - g_terrain_state.last_vertical_accel ) > ( impact_threshold * 1.5f ) ) ) && // Significant change from last frame AND
                                   ( accel_change > 0.12f ) ; // Must be a significant change (0.12G minimum)
        
        // Smooth terrain roughness for consistent feel - ONLY use vertical acceleration, NOT lateral
        float current_roughness = abs_vertical_accel / 9.81f ;
        g_terrain_state.smoothed_roughness = g_terrain_state.smoothed_roughness * config::terrain_smoothing_factor + 
                                           current_roughness * ( 1.0f - config::terrain_smoothing_factor ) ;
        
        // Classify terrain types - re-enabled with conservative thresholds
        bool const on_minor_bump = current_roughness > ( config::terrain_minor_threshold * 3.0f ) ; // Conservative threshold 
        bool const on_rough_terrain = g_terrain_state.smoothed_roughness > ( config::terrain_detection_threshold * 3.0f ) ; // Conservative threshold
        bool const on_major_terrain = g_terrain_state.smoothed_roughness > ( config::terrain_major_threshold * 2.0f ) ; // Conservative threshold
        
        // Impact timer for sustained effects after hitting something - allow lower speeds for curbs
        // Only detect new impacts if not in cooldown period
        if( sudden_impact && abs_speed > 3.0f && g_terrain_state.impact_cooldown <= 0.0f ) // Added cooldown check
        {
                g_terrain_state.impact_timer = config::terrain_impact_duration * 0.25f ; // Shorter duration for cleaner separation
                g_terrain_state.impact_cooldown = 0.6f ; // Shorter cooldown to allow curb on/off detection
        }
        
        // Update timers
        if( g_terrain_state.impact_timer > 0.0f )
        {
                g_terrain_state.impact_timer -= 1.0f / 60.0f ; // Assume 60 FPS for timer decay
                g_terrain_state.impact_timer = std::max( 0.0f, g_terrain_state.impact_timer ) ;
        }
        if( g_terrain_state.impact_cooldown > 0.0f )
        {
                g_terrain_state.impact_cooldown -= 1.0f / 60.0f ; // Cooldown timer
                g_terrain_state.impact_cooldown = std::max( 0.0f, g_terrain_state.impact_cooldown ) ;
        }
        
        // Self-aligning torque simulation based on speed and tire physics
        float self_align_torque = 0.0f ;
        if( abs_speed > config::speed_stationary_threshold )
        {
                // Base self-aligning torque increases with speed and steering angle
                self_align_torque = abs_speed * config::sat_base_torque_factor * std::abs( effective_steering ) ;
                
                // Reduce at very high speeds to simulate tire saturation
                if( speed_kmh > config::sat_speed_reduction_start )
                {
                        float speed_factor = 1.0f - ( ( speed_kmh - config::sat_speed_reduction_start ) / config::sat_speed_reduction_range ) ;
                        self_align_torque *= std::max( config::sat_min_factor, speed_factor ) ;
                }
                
                // Lateral acceleration reduces self-aligning torque (tire saturation)
                float lateral_factor = 1.0f - std::min( config::sat_max_lateral_reduction, std::abs( lateral_g ) * config::sat_lateral_g_factor ) ;
                self_align_torque *= lateral_factor ;
        }
        
        // Power steering simulation - steering assistance when engine is running
        float power_steering_multiplier = 1.0f ;
        if( telemetry.engine_enabled && telemetry.rpm > 500.0f ) // Higher RPM threshold for power assist
        {
                // Power steering makes steering lighter, but more gradually
                if( speed_kmh < 10.0f ) {
                        power_steering_multiplier = 0.7f ; // Moderately lighter at very low speeds
                } else if( speed_kmh < 30.0f ) {
                        power_steering_multiplier = 0.8f ; // Slightly lighter at medium speeds
                } else {
                        power_steering_multiplier = 0.9f ; // Minimal assistance at highway speeds
                }
        }
        else
        {
                // No power steering - much heavier manual steering, especially at low speeds
                if( speed_kmh < 10.0f ) {
                        power_steering_multiplier = 2.0f ; // Much heavier when parking without power assist
                } else if( speed_kmh < 30.0f ) {
                        power_steering_multiplier = 1.6f ; // Heavy at city speeds
                } else {
                        power_steering_multiplier = 1.3f ; // Still heavier at highway speeds
                }
        }
        
        // Calculate centering multiplier - inverse of power steering (lighter steering = less aggressive centering)
        float centering_multiplier = 1.0f ;
        if( telemetry.engine_enabled && telemetry.rpm > 500.0f ) {
                // With power steering, centering should be gentler
                centering_multiplier = 0.7f ; 
        } else {
                // Without power steering, centering can be stronger to help return to center
                centering_multiplier = 1.0f ;
        }
        
        // Calculate road feel and centering force
        if( abs_speed < config::speed_stationary_threshold )
        {
                // Stationary - no centering force (realistic truck behavior), heavier steering effort 
                params.autocenter_force = static_cast< std::uint8_t >( config::center_stationary_force * centering_multiplier ) ;
                params.autocenter_slope = static_cast< std::uint8_t >( config::center_stationary_slope ) ;
                params.damper_force_pos = static_cast< std::uint8_t >( config::damper_stationary_pos * power_steering_multiplier ) ;
                params.damper_force_neg = static_cast< std::uint8_t >( config::damper_stationary_neg * power_steering_multiplier ) ;
        }
        else if( speed_kmh < config::speed_low_threshold )
        {
                // Low speed - gentle centering for maneuvering, separate centering and steering effort
                params.autocenter_force = static_cast< std::uint8_t >( ( config::center_low_speed_base + speed_kmh * config::center_low_speed_factor ) * centering_multiplier ) ;
                params.autocenter_slope = 2 ;
                params.damper_force_pos = static_cast< std::uint8_t >( config::damper_low_speed * power_steering_multiplier ) ;
                params.damper_force_neg = static_cast< std::uint8_t >( config::damper_low_speed * power_steering_multiplier ) ;
        }
        else
        {
                // Highway speeds - dynamic self-aligning torque, separate centering and steering effort
                params.autocenter_force = static_cast< std::uint8_t >( 
                        std::min( config::center_max_force, ( config::center_highway_base + self_align_torque * config::center_highway_factor ) * centering_multiplier ) ) ;
                
                // Slope varies with speed for different feel
                if( speed_kmh < config::speed_medium_threshold )      params.autocenter_slope = 2 ;
                else if( speed_kmh < config::speed_high_threshold )   params.autocenter_slope = 3 ;
                else if( speed_kmh < config::speed_very_high_threshold ) params.autocenter_slope = 4 ;
                else                                                  params.autocenter_slope = 5 ;
                
                // Dynamic damping based on speed, affected by power steering
                std::uint8_t base_damper = static_cast< std::uint8_t >( 
                        std::min( config::damper_max, ( 1.0f + speed_kmh / config::damper_speed_factor ) * power_steering_multiplier ) ) ;
                params.damper_force_pos = base_damper ;
                params.damper_force_neg = base_damper ;
        }
        
        // Engine braking effects
        if( telemetry.motor_brake || telemetry.retarder_level > 0 )
        {
                float brake_factor = config::damper_brake_factor + ( telemetry.retarder_level * config::damper_retarder_factor ) ;
                if( telemetry.motor_brake ) brake_factor += config::damper_engine_brake_factor ;
                
                params.damper_force_pos = static_cast< std::uint8_t >( 
                        std::min( config::damper_max_total, params.damper_force_pos * brake_factor ) ) ;
                params.damper_force_neg = static_cast< std::uint8_t >( 
                        std::min( config::damper_max_total, params.damper_force_neg * brake_factor ) ) ;
        }
        
        // Simulate heavy truck inertia effects
        float const yaw_rate = telemetry.angular_velocity_z ;
        if( std::abs( yaw_rate ) > config::yaw_rate_threshold && abs_speed > 5.0f )
        {
                // Add understeer/oversteer effects through asymmetric damping
                float yaw_factor = std::min( config::yaw_max_factor, std::abs( yaw_rate ) * config::yaw_rate_factor ) ;
                
                if( ( yaw_rate > 0 && effective_steering > 0 ) || ( yaw_rate < 0 && effective_steering < 0 ) )
                {
                        // Oversteer - reduce centering, add damping
                        params.autocenter_force = static_cast< std::uint8_t >( 
                                params.autocenter_force * ( 1.0f - yaw_factor * config::oversteer_reduction ) ) ;
                        params.damper_force_pos += static_cast< std::uint8_t >( yaw_factor * config::oversteer_damping_add ) ;
                        params.damper_force_neg += static_cast< std::uint8_t >( yaw_factor * config::oversteer_damping_add ) ;
                }
                else
                {
                        // Understeer - increase centering force  
                        params.autocenter_force = static_cast< std::uint8_t >( 
                                std::min( 80.0f, params.autocenter_force * ( 1.0f + yaw_factor * config::understeer_factor ) ) ) ;
                }
        }
        
        // Enhanced terrain effects - responsive to all surface variations
        float terrain_force_multiplier = 1.0f ;
        float terrain_damping_add = 0.0f ;
        bool use_terrain_spring = false ;
        std::uint8_t terrain_spring_intensity = 0 ;
        
        // Sudden impact effects (curbs, potholes, road edges) - stronger for better feel
        if( g_terrain_state.impact_timer > 0.0f )
        {
                float impact_intensity = g_terrain_state.impact_timer / ( config::terrain_impact_duration * 0.3f ) ;
                terrain_force_multiplier += impact_intensity * 1.0f ; // Increased multiplier for curbs
                terrain_damping_add += impact_intensity * 2.0f ; // Increased damping for impact feel
                
                // Use spring effects for noticeable curb feedback
                use_terrain_spring = true ;
                terrain_spring_intensity = static_cast< std::uint8_t >( 
                        std::min( 12.0f, impact_intensity * 20.0f ) ) ; // Much stronger for noticeable curb hits
        }
        // Minor bumps and surface variations - re-enabled with conservative thresholds
        else if( on_minor_bump && abs_speed > 12.0f ) // Higher speed threshold
        {
                terrain_force_multiplier += current_roughness * 1.0f ; // Reduced multiplier
                terrain_damping_add += current_roughness * 0.8f ; // Reduced damping
                
                use_terrain_spring = true ;
                terrain_spring_intensity = static_cast< std::uint8_t >( 
                        std::min( 4.0f, 1.0f + current_roughness * 3.0f ) ) ; // Much lower intensity
        }
        // Continuous rough terrain (dirt roads, gravel) - re-enabled with conservative settings
        else if( on_rough_terrain && abs_speed > 8.0f ) // Lower speed but conservative detection
        {
                if( on_major_terrain )
                {
                        terrain_force_multiplier = 1.0f + config::terrain_offroad_multiplier * 0.1f ; // Much reduced
                        terrain_damping_add = g_terrain_state.smoothed_roughness * 0.8f ; // Reduced
                }
                else
                {
                        terrain_force_multiplier = 1.0f + g_terrain_state.smoothed_roughness * 0.5f ; // Much reduced
                        terrain_damping_add = g_terrain_state.smoothed_roughness * 0.4f ; // Reduced
                }
                
                use_terrain_spring = true ;
                terrain_spring_intensity = static_cast< std::uint8_t >( 
                        std::min( 3.0f, 0.5f + g_terrain_state.smoothed_roughness * 2.0f ) ) ; // Much lower intensity
        }
        
        // Apply terrain effects to force feedback parameters
        if( terrain_force_multiplier > 1.0f || terrain_damping_add > 0.0f )
        {
                // Increase centering force for terrain feel
                params.autocenter_force = static_cast< std::uint8_t >( 
                        std::min( 80.0f, params.autocenter_force * terrain_force_multiplier ) ) ;
                
                // Add terrain-based damping
                params.damper_force_pos = static_cast< std::uint8_t >( 
                        std::min( 8.0f, params.damper_force_pos + terrain_damping_add ) ) ;
                params.damper_force_neg = static_cast< std::uint8_t >( 
                        std::min( 8.0f, params.damper_force_neg + terrain_damping_add ) ) ;
        }
        
        // Terrain-specific spring effects
        if( use_terrain_spring )
        {
                params.use_custom_spring = true ;
                params.spring_k1 = terrain_spring_intensity ;
                params.spring_k2 = terrain_spring_intensity ;
                params.spring_clip = static_cast< std::uint8_t >( 20 + terrain_spring_intensity * 8 ) ;
        }
        // Smooth road surface simulation - DISABLED to eliminate unwanted shaking
        // This was causing vibrations during normal highway driving
        /* TEMPORARILY DISABLED
        else if( abs_speed > config::road_feel_speed_threshold && telemetry.rpm > config::road_feel_rpm_threshold && !on_rough_terrain && !on_minor_bump )
        {
                // Road feel simulation temporarily disabled to eliminate unwanted vibrations
        }
        */
        
        // Steering kickback simulation for sudden load changes (only if no terrain impact)
        float const steering_rate = std::abs( telemetry.angular_acceleration_z ) ;
        if( steering_rate > config::kickback_threshold && abs_speed > config::kickback_speed_threshold && !params.use_constant_force )
        {
                // Sudden steering inputs create momentary force feedback
                params.use_constant_force = true ;
                params.constant_force = static_cast< std::uint8_t >( 
                        std::min( config::kickback_max_force, steering_rate * config::kickback_factor ) ) ;
        }
        
        // Weight transfer effects during acceleration/braking - DISABLED (variable unused)
        /*
        if( std::abs( longitudinal_g ) > config::weight_transfer_threshold )
        {
                float weight_factor = 1.0f + std::abs( longitudinal_g ) * config::weight_transfer_factor ;
                params.autocenter_force = static_cast< std::uint8_t >( 
                        std::min( config::weight_transfer_max_force, params.autocenter_force * weight_factor ) ) ;
        }
        */
        
        // Parking brake - lock steering feel
        if( telemetry.parking_brake )
        {
                params.autocenter_force = static_cast< std::uint8_t >( config::parking_brake_force ) ;
                params.autocenter_slope = static_cast< std::uint8_t >( config::parking_brake_slope ) ;
                params.damper_force_pos = static_cast< std::uint8_t >( config::parking_brake_damper ) ;
                params.damper_force_neg = static_cast< std::uint8_t >( config::parking_brake_damper ) ;
        }
        
        // Note: telemetry.last_vertical_acceleration will be updated by caller
        
        return params ;
}


std::uint8_t calculate_damper_force ( float speed, float rpm )
{
        if( rpm != 0 ) return 0 ;

        if(      speed <  1 ) { return 3 ; }
        else if( speed <  5 ) { return 2 ; }
        else if( speed < 45 ) { return 1 ; }
        else if( speed < 75 ) { return 1 ; }
        else                  { return 0 ; }
}

std::tuple< std::uint8_t, std::uint8_t > calculate_autocentering_force ( float speed )
{
        std::uint8_t force ;
        std::uint8_t slope ;

        if( speed < 1 )
        {
                force = 0 ;
                slope = 0 ;
        }
        else
        {
                if(      speed <  5 ) { slope = 2 ; }
                else if( speed < 45 ) { slope = 2 ; }
                else if( speed < 75 ) { slope = 3 ; }
                else                  { slope = 4 ; }

                force = speed * speed / 2000.0f * 255 ;

                if( force < 24 ) force = 24 ;
                if( force > 64 ) force = 64 ;
        }
        return std::tuple{ slope, force } ;
}

bool update_forces ( g923mac::vector< g923mac::wheel > & wheels, telemetry_state_t const & telemetry ) {
        bool all_passed { true } ;

        // Calculate enhanced force feedback parameters
        force_feedback_params_t const params = calculate_enhanced_forces( telemetry ) ;

        for( auto & wheel : wheels ) {
                // Apply constant force first if needed (for steering kickback)
                if( params.use_constant_force )
                {
                        if( !wheel.set_constant_force( params.constant_force ) )
                        {
                                g_game_log( SCS_LOG_TYPE_error, "g923mac : failed setting constant force" ) ;
                                all_passed = false ;
                        }
                        continue ; // Don't apply other forces when using constant force
                }
                else
                {
                        // Stop any previous constant forces
                        wheel.stop_forces() ;
                }

                // Apply custom spring for road surface simulation
                if( params.use_custom_spring )
                {
                        if( !wheel.set_custom_spring( 0, 0, params.spring_k1, params.spring_k2, 
                                                     0, 0, params.spring_clip ) )
                        {
                                g_game_log( SCS_LOG_TYPE_error, "g923mac : failed setting custom spring" ) ;
                                all_passed = false ;
                        }
                }

                // Apply damping forces
                if( params.damper_force_pos > 0 || params.damper_force_neg > 0 )
                {
                        if( !wheel.set_damper( params.damper_force_pos, params.damper_force_neg, 0, 0 ) )
                        {
                                g_game_log( SCS_LOG_TYPE_error, "g923mac : failed setting damper force" ) ;
                                all_passed = false ;
                        }
                }

                // Apply self-aligning torque (autocenter)
                if( params.autocenter_force > 0 )
                {
                        if( !wheel.enable_autocenter() || 
                            !wheel.set_autocenter_spring( params.autocenter_slope, params.autocenter_slope, 
                                                         params.autocenter_force ) )
                        {
                                g_game_log( SCS_LOG_TYPE_error, "g923mac : failed setting autocenter spring force" ) ;
                                all_passed = false ;
                        }
                }
                else
                {
                        if( !wheel.disable_autocenter() )
                        {
                                g_game_log( SCS_LOG_TYPE_error, "g923mac : failed disabling autocenter spring" ) ;
                                all_passed = false ;
                        }
                }
        }

        return all_passed ;
}

bool update_wheels ( telemetry_state_t const & telemetry )
{
        using config = g923mac::ffb_config ;
        
        static std::int32_t ffb_rate       { config::force_update_rate } ;
        static std::int32_t ffb_rate_count { config::force_update_rate } ;
        static std::int32_t led_rate       { config::led_update_rate } ;
        static std::int32_t led_rate_count { config::led_update_rate } ;

        --ffb_rate_count ;
        --led_rate_count ;

        if( ffb_rate_count == 0 )
        {
                if( !update_forces( g_wheels, telemetry ) ) 
                { 
                        g_game_log( SCS_LOG_TYPE_error, "g923mac::error : update_forces failed" ) ; 
                        return false ; 
                }
                ffb_rate_count = ffb_rate ;
        }

        if( led_rate_count == 0 )
        {
                for( auto & wheel : g_wheels )
                {
                        if( !update_leds( wheel, telemetry.rpm, telemetry.speed, 
                                         telemetry.brake, telemetry.parking_brake ) )
                        {
                                g_game_log( SCS_LOG_TYPE_warning, "g923mac::warning : LED update failed" ) ;
                        }
                }
                led_rate_count = led_rate ;
        }

        return true ;
}

bool reset_wheels ()
{
        bool succ { true } ;

        for( auto & wheel : g_wheels )
        {
                if( !wheel.       stop_forces() ) succ = false ;
                if( !wheel.disable_autocenter() ) succ = false ;
                if( !update_leds( wheel, 0, 0, 0, false ) ) succ = false ;
        }
        return succ ;
}

void deinit_wheels ()
{
        g_wheels.clear() ;
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
                reset_wheels() ;
                return ;
        }
        if( !update_wheels( g_telemetry_state ) )
        {
                g_game_log( SCS_LOG_TYPE_error, "g923mac::error : failed updating forces!" ) ;
        }
        
        // Store current vertical acceleration for next frame impact detection
        g_terrain_state.last_vertical_accel = g_telemetry_state.linear_acceleration_y ;
}

SCSAPI_VOID telemetry_pause ( scs_event_t const event, [[ maybe_unused ]] void const * const event_info, [[ maybe_unused ]] scs_context_t const context )
{
        g_telemetry_paused = ( event == SCS_TELEMETRY_EVENT_paused ) ;

        if( g_telemetry_paused )
        {
                reset_wheels() ;
                g_game_log( SCS_LOG_TYPE_message, "g923mac::info : telemetry paused, stopped forces" ) ;
        }
        else {}
}

SCSAPI_VOID telemetry_store_linear_velocity ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( context ) ;
        telemetry_state_t * const state = static_cast< telemetry_state_t * >( context ) ;

        if( !value )
        {
                return ;
        }
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_fvector ) ;
        state->linear_velocity_x = value->value_fvector.x ; // X (lateral)
        state->linear_velocity_y = value->value_fvector.y ; // Y (vertical)
        state->linear_velocity_z = value->value_fvector.z ; // Z (longitudinal)
}

SCSAPI_VOID telemetry_store_angular_velocity ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( context ) ;
        telemetry_state_t * const state = static_cast< telemetry_state_t * >( context ) ;

        if( !value )
        {
                return ;
        }
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_fvector ) ;
        state->angular_velocity_x = value->value_fvector.x ; // X (roll rate)
        state->angular_velocity_y = value->value_fvector.y ; // Y (pitch rate)
        state->angular_velocity_z = value->value_fvector.z ; // Z (yaw rate)
}

SCSAPI_VOID telemetry_store_linear_acceleration ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( context ) ;
        telemetry_state_t * const state = static_cast< telemetry_state_t * >( context ) ;

        if( !value )
        {
                return ;
        }
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_fvector ) ;
        state->linear_acceleration_x = value->value_fvector.x ; // X (lateral)
        state->linear_acceleration_y = value->value_fvector.y ; // Y (vertical) - THIS IS KEY FOR TERRAIN!
        state->linear_acceleration_z = value->value_fvector.z ; // Z (longitudinal)
}

SCSAPI_VOID telemetry_store_angular_acceleration ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( context ) ;
        telemetry_state_t * const state = static_cast< telemetry_state_t * >( context ) ;

        if( !value )
        {
                return ;
        }
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_fvector ) ;
        state->angular_acceleration_x = value->value_fvector.x ; // X (roll acceleration)
        state->angular_acceleration_y = value->value_fvector.y ; // Y (pitch acceleration)
        state->angular_acceleration_z = value->value_fvector.z ; // Z (yaw acceleration)
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

SCSAPI_VOID telemetry_store_bool ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_bool ) ;
        assert( context ) ;
        *static_cast< bool * >( context ) = ( value->value_bool.value != 0 ) ;
}

SCSAPI_VOID telemetry_store_u32 ( [[ maybe_unused ]] scs_string_t const name, [[ maybe_unused ]] scs_u32_t const index, scs_value_t const * const value, scs_context_t const context )
{
        assert( value ) ;
        assert( value->type == SCS_VALUE_TYPE_u32 ) ;
        assert( context ) ;
        *static_cast< std::uint32_t * >( context ) = value->value_u32.value ;
}

SCSAPI_RESULT scs_telemetry_init ( scs_u32_t const version, scs_telemetry_init_params_t const * const params )
{
        if( version != SCS_TELEMETRY_VERSION_1_01 )
        {
                return SCS_RESULT_unsupported ;
        }
        scs_telemetry_init_params_v101_t const * const version_params = static_cast< scs_telemetry_init_params_v101_t const * >( params ) ;

        g_game_log = version_params->common.log ;

        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : version " G923MAC_HELPER_VERSION " starting initialization..." ) ;

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
        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : version checks passed" ) ;
        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : registering to events..." ) ;

        bool const events_registered =
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_frame_start, telemetry_frame_start, nullptr ) == SCS_RESULT_ok ) &&
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_frame_end  , telemetry_frame_end  , nullptr ) == SCS_RESULT_ok ) &&
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_paused     , telemetry_pause      , nullptr ) == SCS_RESULT_ok ) &&
                ( version_params->register_for_event( SCS_TELEMETRY_EVENT_started    , telemetry_pause      , nullptr ) == SCS_RESULT_ok )  ;

        if( !events_registered )
        {
                g_game_log( SCS_LOG_TYPE_error, "g923mac::error : failed to register event callbacks" ) ;
                return SCS_RESULT_generic_error ;
        }
        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : event registration successful" ) ;
        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : registering to channels..." ) ;

        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : channel registration completed" ) ;
        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : registering to channels..." ) ;

        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_world_placement, SCS_U32_NIL, SCS_VALUE_TYPE_euler, SCS_TELEMETRY_CHANNEL_FLAG_no_value, telemetry_store_orientation, &g_telemetry_state       ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_speed          , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none    , telemetry_store_float      , &g_telemetry_state.speed ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm     , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none    , telemetry_store_float      , &g_telemetry_state.rpm   ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear    , SCS_U32_NIL, SCS_VALUE_TYPE_s32  , SCS_TELEMETRY_CHANNEL_FLAG_none    , telemetry_store_s32        , &g_telemetry_state.gear  ) ;

        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_input_steering    , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.input_steering ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_steering, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.steering       ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_throttle, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.throttle       ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_brake   , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.brake          ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_effective_clutch  , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &g_telemetry_state.clutch         ) ;

        // Register for enhanced physics data using proper fvector types
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_local_linear_velocity, SCS_U32_NIL, SCS_VALUE_TYPE_fvector, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_linear_velocity, &g_telemetry_state ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_local_angular_velocity, SCS_U32_NIL, SCS_VALUE_TYPE_fvector, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_angular_velocity, &g_telemetry_state ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_local_linear_acceleration, SCS_U32_NIL, SCS_VALUE_TYPE_fvector, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_linear_acceleration, &g_telemetry_state ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_local_angular_acceleration, SCS_U32_NIL, SCS_VALUE_TYPE_fvector, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_angular_acceleration, &g_telemetry_state ) ;

        // Additional truck state for enhanced force feedback
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_parking_brake       , SCS_U32_NIL, SCS_VALUE_TYPE_bool , SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool       , &g_telemetry_state.parking_brake      ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_motor_brake         , SCS_U32_NIL, SCS_VALUE_TYPE_bool , SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool       , &g_telemetry_state.motor_brake        ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_retarder_level      , SCS_U32_NIL, SCS_VALUE_TYPE_u32  , SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_u32        , &g_telemetry_state.retarder_level     ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_brake_air_pressure  , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float      , &g_telemetry_state.brake_air_pressure ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_cruise_control      , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float      , &g_telemetry_state.cruise_control     ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_fuel                , SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float      , &g_telemetry_state.fuel_amount        ) ;
        version_params->register_for_channel( SCS_TELEMETRY_TRUCK_CHANNEL_engine_enabled      , SCS_U32_NIL, SCS_VALUE_TYPE_bool , SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool       , &g_telemetry_state.engine_enabled     ) ;

        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : enhanced channel registration completed" ) ;

        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : initializing wheel..." ) ;
        if( !init_wheels() )
        {
                g_game_log( SCS_LOG_TYPE_error, "ftl_ffb::error : failed to initialize wheel" ) ;
                return SCS_RESULT_generic_error ;
        }
        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : wheel initialization successful" ) ;

        memset( &g_telemetry_state, 0, sizeof( g_telemetry_state ) ) ;
        memset( &g_terrain_state, 0, sizeof( g_terrain_state ) ) ;
        g_last_timestamp = static_cast< scs_timestamp_t >( -1 ) ;

        g_telemetry_paused = true ;

        g_game_log( SCS_LOG_TYPE_message, "g923mac::info : successfully initialized" ) ;
        return SCS_RESULT_ok ;
}

SCSAPI_VOID scs_telemetry_shutdown ()
{
        g_game_log = nullptr ;
        deinit_wheels() ;
}

void __attribute__(( destructor )) unload ()
{
        deinit_wheels() ;
}
