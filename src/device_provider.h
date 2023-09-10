#pragma once

#include <memory>
#include "controller_device.h"
#include "openvr_driver.h"

/**
 * Represents a device provider that handles left and right controller devices.
 * This class interacts with the OpenVR system, performing tasks such as initialization and frame updates.
 */
class DeviceProvider : public vr::IServerTrackedDeviceProvider {
public:
    // Overrides from IServerTrackedDeviceProvider
    vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;
    void Cleanup() override;
    const char* const* GetInterfaceVersions() override;
    void RunFrame() override;
    bool ShouldBlockStandbyMode() override;
    void EnterStandby() override;
    void LeaveStandby() override;

private:
    std::unique_ptr<ControllerDevice> my_left_device_;
    std::unique_ptr<ControllerDevice> my_right_device_;
};