#include "controller_device.h"
#include "windows.h"
#include "vrmath.h"
#include <array>

// Constants for settings sections and keys
static const char* my_controller_main_settings_section = "workshop_driver";
static const char* my_controller_right_settings_section = "driver_simplecontroller_left_controller";
static const char* my_controller_left_settings_section = "driver_simplecontroller_right_controller";
static const char* my_controller_settings_key_model_number = "mycontroller_model_number";
static const char* my_controller_settings_key_serial_number = "mycontroller_serial_number";
float additionalYOffset = 0.0f;
bool flag = true; 


// Constructor
ControllerDevice::ControllerDevice(vr::ETrackedControllerRole role) :
	is_active_(false),
	role_(role),
	device_id_(vr::k_unTrackedDeviceIndexInvalid) {

		// Initialize model and serial number from settings
		char model_number[1024];
		vr::VRSettings()->GetString(my_controller_main_settings_section, my_controller_settings_key_model_number, model_number, sizeof(model_number));
		my_controller_model_number_ = model_number;

		char serial_number[1024];
		vr::VRSettings()->GetString(role_ == vr::TrackedControllerRole_LeftHand ? my_controller_left_settings_section : my_controller_right_settings_section,
			my_controller_settings_key_serial_number, serial_number, sizeof(serial_number));
		my_controller_serial_number_ = serial_number;

		udp_Receiver.Init(3501);
		udp_Receiver.StartListening();
	}

// Activation method for the controller device
vr::EVRInitError ControllerDevice::Activate(uint32_t unObjectId) {
	is_active_ = true;
	device_id_ = unObjectId;

	vr::VRDriverLog()->Log("ControllerDevice::Activate");

	// Setup properties for the device
	const vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(device_id_);
	vr::VRProperties()->SetInt32Property(container, vr::Prop_ControllerRoleHint_Int32, role_);
	vr::VRProperties()->SetStringProperty(container, vr::Prop_ModelNumber_String, "model_num_1");

	vr::VRProperties()->SetStringProperty(container, vr::Prop_InputProfilePath_String,
		"C:/Users/VR-FMRI/Desktop/driver for the project/workshop_driver/resources/input/EMG_profile.json");
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/system/click", &my_input_handles_[kInputHandle_A_click]);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/trackpad/touch", &my_input_handles_[kInputHandle_A_touch]);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trigger/value", &my_input_handles_[kInputHandle_trigger_value],
		vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/trigger/click", &my_input_handles_[kInputHandle_trigger_click]);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trackpad/x", &my_input_handles_[kInputHandle_joystick_x],
		vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trackpad/y", &my_input_handles_[kInputHandle_joystick_y],
		vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/joystick/click", &my_input_handles_[kInputHandle_joystick_click]);
	vr::VRDriverInput()->CreateHapticComponent(container, "/output/haptic", &my_input_handles_[kInputHandle_haptic]);

	return vr::VRInitError_None;
}

// Frame update logic
void ControllerDevice::RunFrame() {
	std::string udpMessage = udp_Receiver.GetLatestPacket();
	const char* c = udpMessage.c_str();
	vr::VRDriverLog()->Log("hereeee");
	vr::VRDriverLog()->Log(c);

	//Start reciving the EMG data
	if ((GetAsyncKeyState(VK_LEFT) && 0x8000) != 0) {
		flag = false;
	}
	
	//Show both controllers
	if ((GetAsyncKeyState(VK_UP) && 0x8000) != 0) {
		additionalYOffset = 0.0f; 
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_id_, GetPose(), sizeof(vr::DriverPose_t));
		vr::VRDriverLog()->Log("Key UP pressed. Pose updated and A_click input sent.");
		vr::VRDriverInput()->UpdateBooleanComponent(my_input_handles_[kInputHandle_trigger_value], 1, 0.0);
	}

	//When hand moving UP
	if (!flag && (!strcmp(c, "1")) && role_ == vr::TrackedControllerRole_RightHand) {
		additionalYOffset += 0.01f;  // Increment by 1 centimeters
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_id_, GetPose(), sizeof(vr::DriverPose_t));
		vr::VRDriverLog()->Log("Pose updated and A_click input sent.");
		vr::VRDriverInput()->UpdateBooleanComponent(my_input_handles_[kInputHandle_trigger_value], 1, 0.0);
	}

	//When hand moving down
	if (!flag && (strcmp(c, "1")) && role_ == vr::TrackedControllerRole_RightHand) {
		additionalYOffset -= 0.01f;  // Decrement by 1 centimeters
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_id_, GetPose(), sizeof(vr::DriverPose_t));
		vr::VRDriverLog()->Log("Key DOWN pressed. Pose updated and trigger value input sent.");
		vr::VRDriverInput()->UpdateBooleanComponent(my_input_handles_[kInputHandle_trigger_value], 1, 0.0);
	}
}

// Handle VR events
void ControllerDevice::HandleEvent(const vr::VREvent_t& vrevent) {
	switch (vrevent.eventType) {
		case vr::VREvent_Input_HapticVibration: {
			if (vrevent.data.hapticVibration.componentHandle == my_input_handles_[kInputHandle_haptic]) {
				vr::VRDriverLog()->Log("Received haptic feedback event");
			}
			break;
		}
	}
}

// Deactivate the device
void ControllerDevice::Deactivate() {
}

// Put the device into standby
void ControllerDevice::EnterStandby() {
}

// Retrieve component by name and version
void* ControllerDevice::GetComponent(const char* pchComponentNameAndVersion) {
	return nullptr;
}

// Debug request handling
void ControllerDevice::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

// Get device pose
vr::DriverPose_t ControllerDevice::GetPose() {
	vr::DriverPose_t pose = { 0 };
	pose.qWorldFromDriverRotation.w = 1.f;
	pose.qDriverFromHeadRotation.w = 1.f;
	pose.qRotation.w = 1.f;

	vr::TrackedDevicePose_t hmd_pose{};
	vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, &hmd_pose, 1);

	const vr::HmdVector3_t hmd_position = HmdVector3_From34Matrix(hmd_pose.mDeviceToAbsoluteTracking);
	const vr::HmdQuaternion_t hmd_orientation = HmdQuaternion_FromMatrix(hmd_pose.mDeviceToAbsoluteTracking);
	const vr::HmdQuaternion_t offset_orientation = HmdQuaternion_FromEulerAngles(0.f, DEG_TO_RAD(90.f), 0.f);

	pose.qRotation = hmd_orientation * offset_orientation;

	const vr::HmdVector3_t offset_position = {
		role_ == vr::TrackedControllerRole_LeftHand ? -0.15f : 0.15f, // translate the controller left/right 0.15m depending on its role
		0.1f + additionalYOffset,																		// shift it up a little to make it more in view
		-0.5f,																		// put each controller 0.5m forward in front of the hmd so we can see it.
	};
	const vr::HmdVector3_t position = hmd_position + (offset_position * hmd_orientation);

	// Copy our position to our pose
	pose.vecPosition[0] = position.v[0];
	pose.vecPosition[1] = position.v[1];
	pose.vecPosition[2] = position.v[2];

	pose.poseIsValid = true;
	pose.result = vr::TrackingResult_Running_OK;
	pose.deviceIsConnected = true;

	return pose;
}

// Getter for serial number
const std::string& ControllerDevice::MyGetSerialNumber() {
	return my_controller_serial_number_;
}