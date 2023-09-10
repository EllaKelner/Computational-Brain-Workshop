#include "device_provider.h"

// Initialize the device provider and its associated devices
vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

    // Initialize left controller device
    my_left_device_ = std::make_unique<ControllerDevice>(vr::TrackedControllerRole_LeftHand);
    bool left_success = vr::VRServerDriverHost()->TrackedDeviceAdded(
        "4294967297",
        vr::TrackedDeviceClass_Controller,
        my_left_device_.get()
    );

    // Log left controller initialization status
    if (left_success) {
        vr::VRDriverLog()->Log("Left controller initialized successfully.");
    }
    else {
        vr::VRDriverLog()->Log("Failed to initialize left controller.");
        vr::VRDriverLog()->Log(my_left_device_->MyGetSerialNumber().c_str());
    }

    // Initialize right controller device+
    my_right_device_ = std::make_unique<ControllerDevice>(vr::TrackedControllerRole_RightHand);
    bool right_success = vr::VRServerDriverHost()->TrackedDeviceAdded(
        "4294967298",
        vr::TrackedDeviceClass_Controller,
        my_right_device_.get()
    );

    // Log right controller initialization status
    if (right_success) {
        vr::VRDriverLog()->Log("Right controller initialized successfully.");
    }
    else {
        vr::VRDriverLog()->Log("Failed to initialize right controller.");
        vr::VRDriverLog()->Log(my_left_device_->MyGetSerialNumber().c_str());
    }

    return vr::VRInitError_None;
}

void DeviceProvider::Cleanup() {
    my_left_device_ = nullptr;
    my_right_device_ = nullptr;
}

const char* const* DeviceProvider::GetInterfaceVersions() {
    return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame() {
    if (my_left_device_) {
        my_left_device_->RunFrame();
    }

    if (my_right_device_) {
        my_right_device_->RunFrame();
    }

    vr::VREvent_t vrevent;
    while (vr::VRServerDriverHost()->PollNextEvent(&vrevent, sizeof(vrevent))) {
        if (my_left_device_) {
            my_left_device_->HandleEvent(vrevent);
        }

        if (my_right_device_) {
            my_right_device_->HandleEvent(vrevent);
        }
    }
}

bool DeviceProvider::ShouldBlockStandbyMode() {
    return false;
}

void DeviceProvider::EnterStandby() {
    // Implement necessary actions when the device enters standby
}

void DeviceProvider::LeaveStandby() {
    // Implement necessary actions when the device leaves standby
}