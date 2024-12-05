//
//
//      flt
//      types.hpp
//

#pragma once

#include <uti/type/traits.hpp>
#include <uti/container/array.hpp>

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDManager.h>


namespace flt
{


using   device_id_t =     uti::u32_t ;
using  hid_device_t = __IOHIDDevice  ;
using hid_manager_t = __IOHIDManager ;


struct hid_device
{
        device_id_t  vendor_id_ ;
        device_id_t product_id_ ;
        device_id_t  device_id_ ;

        hid_device_t * hid_device_ ;
} ;


constexpr uti::array< device_id_t, 1 > known_wheel_ids = { 0xc266046d } ;


} // namespace flt
