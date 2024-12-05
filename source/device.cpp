//
//
//      flt
//      device.cpp
//

#include <device.hpp>


namespace flt
{


void set_applier_function_copy_to_cfarray ( void const * value, void * context )
{
        CFArrayAppendValue( static_cast< CFMutableArrayRef >( context ), value ) ;
}

CFStringRef get_property_string ( hid_device_t * hid_device, CFStringRef property )
{
        CFTypeRef data_ref = IOHIDDeviceGetProperty( hid_device, property ) ;
        return CFStringCreateCopy( kCFAllocatorDefault, CFStringRef( data_ref ) ) ;
}

uti::u32_t get_property_number ( hid_device_t * hid_device, CFStringRef property )
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

IOReturn open_device ( hid_device const & device )
{
        return IOHIDDeviceOpen( device.hid_device_, kIOHIDOptionsTypeSeizeDevice ) ;
}

IOReturn close_device ( hid_device const & device )
{
        return IOHIDDeviceClose( device.hid_device_, 0 ) ;
}


} // namespace flt
