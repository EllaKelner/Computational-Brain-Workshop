#pragma once
#include "openvr_driver.h" 
#include "UDPReceiver.h"
#include <array>
#include <string>
#include <atomic>
#include <thread>

// Enumerated type for defining the different types of input handles that the controller supports.
enum InputHandles {
    kInputHandle_A_click,
    kInputHandle_A_touch,
    kInputHandle_trigger_value,
    kInputHandle_trigger_click,
    kInputHandle_joystick_x,
    kInputHandle_joystick_y,
    kInputHandle_joystick_click,
    kInputHandle_haptic,
    kInputHandle_COUNT
};

// ControllerDevice class interfaces with the SteamVR driver to represent the custom controller device.
class ControllerDevice : public vr::ITrackedDeviceServerDriver {
public:
    explicit ControllerDevice(vr::ETrackedControllerRole role);

    // Activation of the device. Typically called when the device is plugged in or detected.
    virtual vr::EVRInitError Activate(uint32_t unObjectId) override;

    // Deactivation of the device. Typically called before the device is powered off.
    virtual void Deactivate() override;

    // Method called when the system enters a standby state.
    virtual void EnterStandby() override;

    // Retrieves a specific component associated with this device.
    virtual void* GetComponent(const char* pchComponentNameAndVersion) override;

    // Used for debugging purposes, this method allows a request to be made to the device.
    virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;

    // Returns the current pose of the device in the virtual space.
    virtual vr::DriverPose_t GetPose() override;

    // Get the serial number associated with this device.
    const std::string& MyGetSerialNumber();

    // Update loop for the device, where input states are checked and pose updates are made.
    void RunFrame();

    // Handle specific events directed to this device.
    void HandleEvent(const vr::VREvent_t& vrevent);

private:
    vr::ETrackedControllerRole role_; // Role of the controller (e.g. left or right hand).
    std::atomic<vr::TrackedDeviceIndex_t> device_id_; // Device ID associated with SteamVR.
    std::array<vr::VRInputComponentHandle_t, kInputHandle_COUNT> my_input_handles_; // Array of input handles.

    // Model and serial numbers of the controller.
    std::string my_controller_model_number_;
    std::string my_controller_serial_number_;

    std::atomic<bool> is_active_; // Atomic boolean to check if device is active.
    std::thread my_pose_update_thread_; // Thread for updating pose data.
    UDPReceiver udp_Receiver;
};