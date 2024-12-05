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


IOReturn send_report ( hid_device const & device,         report   const & report  ) ;
IOReturn send_report ( hid_device const & device, vector< report > const & reports ) ;


} // namespace flt
