#ifndef HMD_DRIVER_FACTORY_H
#define HMD_DRIVER_FACTORY_H

#include "device_provider.h"
#include "openvr_driver.h"
#include <cstring>

// Platform-specific DLL export/import macro
#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C" 
#else
#error "Unsupported Platform."
#endif

// Global instance of our device provider interface.
DeviceProvider device_provider;

/**
 * The HmdDriverFactory function is an exported method that is used to interface with OpenVR.
 * When OpenVR wants to collect all the drivers, it will call this function from each driver's dll
 * to get an instance of the vr::IServerTrackedDeviceProvider interface.
 *
 * @param pInterfaceName Pointer to the interface name string.
 * @param pReturnCode Pointer to an error return code. Can be one of vr::EVRInitError.
 *
 * @return Returns a pointer to an instance of the requested interface on success. Returns nullptr on failure.
 */
HMD_DLL_EXPORT void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode) {
	// Check if the requested interface is the IServerTrackedDeviceProvider version.
	if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName)) {
		return &device_provider;
	}

	// If we don't have the requested interface, return a VRInitError_Init_InterfaceNotFound error.
	if (pReturnCode) {
		*pReturnCode = vr::VRInitError_Init_InterfaceNotFound;
	}

	return nullptr;
}

#endif // HMD_DRIVER_FACTORY_H