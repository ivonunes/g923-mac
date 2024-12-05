//
//
//      flt
//      command.hpp
//

#pragma once

#include <types.hpp>

#define FLT_CMD_MAX_COUNT 4
#define FLT_CMD_MAX_LEN   8


namespace flt
{


struct report
{
        uti::u8_t cmd [ FLT_CMD_MAX_LEN ] ;
} ;


constexpr IOReturn send_report ( hid_device const & device, report const & report )
{
        uti::u8_t const * cmd = &report.cmd[ 0 ] ;

        IOReturn result = IOHIDDeviceSetReport( device.hid_device_, kIOHIDReportTypeOutput, time( nullptr ), cmd, FLT_CMD_MAX_LEN ) ;

        return result ;
}

constexpr IOReturn send_report ( hid_device const & device, vector< report > const & reports )
{
        IOReturn result ;

        for( auto const & report : reports )
        {
                result = send_report( device, report ) ;

                if( result != kIOReturnSuccess ) break ;
        }
        return result ;
}


} // namespace flt
