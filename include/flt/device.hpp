//
//
//      flt
//      device.hpp
//

#pragma once

#include <types.hpp>

#include <iterator>
#include <algorithm>

#define make_device_id(productID, vendorID) ( ( ( ( productID ) % 0xFFFF ) << 16 ) | ( ( vendorID ) & 0xFFFF ) )


namespace flt
{


constexpr void set_applier_function_copy_to_cfarray ( void const * value, void * context ) ;

constexpr CFStringRef get_property_string ( IOHIDDeviceRef hid_device, CFStringRef property ) ;
constexpr uti::u32_t  get_property_number ( IOHIDDeviceRef hid_device, CFStringRef property ) ;

constexpr IOReturn  open_device ( hid_device const & device ) ;
constexpr IOReturn close_device ( hid_device const & device ) ;


class device_manager
{
public:
        constexpr device_manager () noexcept : hid_manager_( _create_hid_manager() ) {}

        constexpr auto list_devices () -> vector< hid_device >
        {
                vector< hid_device > dev_set ;

                CFSetRef device_setref = IOHIDManagerCopyDevices( hid_manager_ ) ;
                CFIndex count = CFSetGetCount( device_setref ) ;

                CFMutableArrayRef device_arrayref = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks ) ;

                CFSetApplyFunction( device_setref, set_applier_function_copy_to_cfarray, static_cast< void * >( device_arrayref ) ) ;

                for( CFIndex i = 0; i < count; ++i )
                {
                        hid_device_t * device = ( hid_device_t * ) CFArrayGetValueAtIndex( device_arrayref, i ) ;

                        uti::u32_t  vendor_id = get_property_number( device, CFSTR( kIOHIDVendorIDKey  ) ) ;
                        uti::u32_t product_id = get_property_number( device, CFSTR( kIOHIDProductIDKey ) ) ;

                        device_id_t device_id = make_device_id( product_id, vendor_id ) ;

                        hid_device device_data{ vendor_id, product_id, device_id, device } ;

                        dev_set.push_back( device_data ) ;
                }
                return dev_set ;
        }

        constexpr auto find_known_wheels () -> vector< hid_device >
        {
                vector< hid_device > dev_set = list_devices() ;
                vector< hid_device > wheels ;

                std::copy_if( dev_set.begin(), dev_set.end(), std::back_inserter( wheels ),
                             [ & ]( hid_device const & dev )
                             {
                                return std::find( known_wheel_ids.begin(), known_wheel_ids.end(), dev.device_id_ ) != known_wheel_ids.end() ;
                             }
                ) ;
                return wheels ;
        }

        constexpr ~device_manager () { _destroy_hid_manager( hid_manager_ ) ; }
private:
        hid_manager_t * hid_manager_ ;

        constexpr hid_manager_t * _create_hid_manager ()
        {
                hid_manager_t * manager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDManagerOptionNone ) ;
                IOHIDManagerSetDeviceMatching( manager, NULL ) ;
                IOHIDManagerOpen( manager, kIOHIDOptionsTypeSeizeDevice ) ;
                return manager ;
        }

        constexpr void _destroy_hid_manager ( hid_manager_t * manager )
        {
                IOHIDManagerClose( manager, kIOHIDManagerOptionNone ) ;
        }
} ;


constexpr void set_applier_function_copy_to_cfarray ( void const * value, void * context )
{
        CFArrayAppendValue( static_cast< CFMutableArrayRef >( context ), value ) ;
}

constexpr CFStringRef get_property_string ( hid_device_t * hid_device, CFStringRef property )
{
        CFTypeRef data_ref = IOHIDDeviceGetProperty( hid_device, property ) ;
        return CFStringCreateCopy( kCFAllocatorDefault, CFStringRef( data_ref ) ) ;
}

constexpr uti::u32_t get_property_number ( hid_device_t * hid_device, CFStringRef property )
{
        CFTypeRef data_ref = IOHIDDeviceGetProperty( hid_device, property ) ;
        if( data_ref && ( CFNumberGetTypeID() == CFGetTypeID( data_ref ) ) )
        {
                uti::u32_t number ;
                CFNumberGetValue( ( CFNumberRef ) data_ref, kCFNumberSInt32Type, &number ) ;
                return number ;
        }
        return 0 ;
}

constexpr IOReturn open_device ( hid_device const & device )
{
        return IOHIDDeviceOpen( device.hid_device_, kIOHIDOptionsTypeSeizeDevice ) ;
}

constexpr IOReturn close_device ( hid_device const & device )
{
        return IOHIDDeviceClose( device.hid_device_, 0 ) ;
}


} // namespace flt
