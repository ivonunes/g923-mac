//
//
//      g923mac
//      force_feedback_config.hpp
//

#pragma once

namespace g923mac
{

// Force feedback configuration constants
struct ffb_config
{
        // Update rates (lower = more frequent updates)
        static constexpr int force_update_rate = 8 ;   // Force feedback update every 8 frames
        static constexpr int led_update_rate = 32 ;    // LED update every 32 frames
        
        // Self-aligning torque parameters
        static constexpr float sat_base_torque_factor = 0.8f ;      // Base self-aligning torque multiplier
        static constexpr float sat_speed_reduction_start = 80.0f ;  // Speed (km/h) where SAT starts reducing
        static constexpr float sat_speed_reduction_range = 120.0f ; // Speed range for SAT reduction
        static constexpr float sat_min_factor = 0.3f ;             // Minimum SAT factor at high speeds
        static constexpr float sat_lateral_g_factor = 0.8f ;       // How much lateral G reduces SAT
        static constexpr float sat_max_lateral_reduction = 0.7f ;  // Maximum SAT reduction from lateral G
        
        // Centering force parameters
        static constexpr float center_stationary_force = 0.0f ;    // No centering when stationary (realistic truck behavior)
        static constexpr float center_stationary_slope = 0.0f ;    // No centering slope when stationary
        static constexpr float center_low_speed_base = 20.0f ;     // Reduced base centering force at low speeds
        static constexpr float center_low_speed_factor = 0.8f ;    // Reduced multiplier for low speed centering
        static constexpr float center_highway_base = 18.0f ;       // Reduced base centering force at highway speeds
        static constexpr float center_highway_factor = 0.6f ;      // Reduced multiplier for highway centering
        static constexpr float center_max_force = 45.0f ;          // Reduced maximum centering force
        
        // Speed thresholds for different behaviors (km/h)
        static constexpr float speed_stationary_threshold = 2.0f ; // Increased threshold for stationary behavior
        static constexpr float speed_low_threshold = 15.0f ;       // Increased threshold for low speed
        static constexpr float speed_medium_threshold = 35.0f ;
        static constexpr float speed_high_threshold = 65.0f ;
        static constexpr float speed_very_high_threshold = 100.0f ;
        
        // Damping parameters
        static constexpr float damper_stationary_pos = 2.0f ;      // Reduced stationary damping
        static constexpr float damper_stationary_neg = 2.0f ;      // Reduced stationary damping
        static constexpr float damper_low_speed = 2.5f ;           // Reduced low speed damping
        static constexpr float damper_speed_factor = 40.0f ;       // Increased speed divisor (less damping)
        static constexpr float damper_max = 3.0f ;                 // Reduced maximum damping
        static constexpr float damper_brake_factor = 0.8f ;        // Reduced brake damping multiplier
        static constexpr float damper_retarder_factor = 0.08f ;    // Reduced retarder damping
        static constexpr float damper_engine_brake_factor = 0.25f ; // Reduced engine brake damping
        static constexpr float damper_max_total = 6.0f ;           // Reduced maximum total damping
        
        // Vehicle dynamics parameters
        static constexpr float yaw_rate_threshold = 0.1f ;         // Minimum yaw rate to trigger effects
        static constexpr float yaw_rate_factor = 10.0f ;           // Yaw rate to force multiplier
        static constexpr float yaw_max_factor = 2.0f ;             // Maximum yaw rate effect
        static constexpr float understeer_factor = 0.3f ;          // Understeer centering increase
        static constexpr float oversteer_reduction = 0.2f ;        // Oversteer centering reduction
        static constexpr float oversteer_damping_add = 1.0f ;      // Additional damping during oversteer
        
        // Road surface simulation
        static constexpr float road_feel_speed_threshold = 20.0f ; // Higher speed to start road surface effects
        static constexpr float road_feel_rpm_threshold = 600.0f ;  // Higher RPM threshold for road effects
        static constexpr float road_feel_speed_factor = 80.0f ;    // Higher speed divisor (less intense vibration)
        static constexpr float road_feel_intensity_threshold = 0.5f ; // Higher minimum intensity for custom spring
        static constexpr float road_feel_spring_base = 1.5f ;      // Reduced base spring constant
        static constexpr float road_feel_spring_factor = 1.5f ;    // Reduced spring factor multiplier
        static constexpr float road_feel_clip_base = 15.0f ;       // Reduced base clipping
        static constexpr float road_feel_clip_factor = 10.0f ;     // Reduced clipping factor multiplier
        
        // Terrain surface effects (enhanced sensitivity)
        static constexpr float terrain_offroad_multiplier = 4.0f ; // Increased force multiplier for off-road surfaces
        static constexpr float terrain_rough_frequency = 8.0f ;    // Vibration frequency for rough terrain
        static constexpr float terrain_smooth_frequency = 15.0f ;  // Vibration frequency for smooth surfaces
        static constexpr float terrain_detection_threshold = 0.08f ; // Further lowered threshold for detecting surface changes
        static constexpr float terrain_minor_threshold = 0.02f ;   // Very sensitive threshold for minor bumps (curbs, potholes)
        static constexpr float terrain_major_threshold = 0.25f ;   // Threshold for major terrain changes
        static constexpr float terrain_impact_multiplier = 10.0f ; // Increased force multiplier for sudden impacts (curbs)
        static constexpr float terrain_impact_duration = 0.5f ;    // Increased duration of impact effects in seconds
        static constexpr float terrain_smoothing_factor = 0.8f ;   // Smoothing factor for terrain transitions
        
        // Steering kickback simulation
        static constexpr float kickback_threshold = 2.0f ;         // Angular acceleration threshold
        static constexpr float kickback_speed_threshold = 5.0f ;   // Speed threshold for kickback
        static constexpr float kickback_factor = 10.0f ;           // Angular acceleration to force multiplier
        static constexpr float kickback_max_force = 40.0f ;        // Maximum kickback force
        
        // Weight transfer effects
        static constexpr float weight_transfer_threshold = 0.2f ;  // Longitudinal G threshold
        static constexpr float weight_transfer_factor = 0.5f ;     // Weight transfer effect multiplier
        static constexpr float weight_transfer_max_force = 90.0f ; // Maximum force from weight transfer
        
        // Parking brake effects
        static constexpr float parking_brake_force = 80.0f ;
        static constexpr float parking_brake_slope = 6.0f ;
        static constexpr float parking_brake_damper = 8.0f ;
        
        // LED configuration
        static constexpr float led_brake_threshold = 0.5f ;        // Brake input to show brake LEDs
        static constexpr float led_heavy_brake = 0.9f ;            // Heavy braking threshold
        static constexpr float led_medium_brake = 0.7f ;           // Medium braking threshold
        static constexpr float led_speed_high_threshold = 50.0f ;  // Speed for RPM threshold adjustment
        static constexpr float led_speed_low_threshold = 10.0f ;   // Speed for RPM threshold adjustment
        static constexpr float led_rpm_base = 1000.0f ;            // Base RPM threshold
        static constexpr float led_rpm_highway = 800.0f ;          // RPM threshold at highway speeds
        static constexpr float led_rpm_city = 1200.0f ;            // RPM threshold at city speeds
        static constexpr float led_rpm_step1 = 300.0f ;            // RPM steps for LED progression
        static constexpr float led_rpm_step2 = 600.0f ;
        static constexpr float led_rpm_step3 = 800.0f ;
        static constexpr float led_rpm_step4 = 1000.0f ;
} ;

} // namespace g923mac
