#pragma once

#include <array>
#include <vector>
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDManager.h>

namespace g923mac {
    using device_id_t = std::uint32_t;
    using hid_device_t = __IOHIDDevice;
    using hid_manager_t = __IOHIDManager;

    template<typename T>
    using vector = std::vector<T>;

    struct hid_device {
        device_id_t vendor_id_;
        device_id_t product_id_;
        device_id_t device_id_;

        hid_device_t *hid_device_;
    };

    constexpr std::array<device_id_t, 1> known_wheel_ids = {0xc266046d};
}
