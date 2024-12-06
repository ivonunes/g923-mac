//
//
//      flt
//      types.hpp
//

#pragma once

#include <uti/type/traits.hpp>
#include <uti/container/array.hpp>
#include <uti/container/vector.hpp>
#include <uti/allocator/freelist_resource.hpp>

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDManager.h>


namespace flt
{


using   device_id_t =     uti::u32_t ;
using  hid_device_t = __IOHIDDevice  ;
using hid_manager_t = __IOHIDManager ;

template< typename T >
using vector = uti::vector< T, uti::allocator< T, uti::static_freelist_resource< 1024 > > > ;


struct hid_device
{
        device_id_t  vendor_id_ ;
        device_id_t product_id_ ;
        device_id_t  device_id_ ;

        hid_device_t * hid_device_ ;
} ;


constexpr uti::array< device_id_t, 1 > known_wheel_ids = { 0xc266046d } ;


} // namespace flt
