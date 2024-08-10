/*++

Module Name:
	driver.c

Abstract:
	This file contains the driver entry points and callbacks.

Environment:
	Kernel-mode Driver Framework

--*/

#include "Driver.h"
#include "Offsets.h"


UNICODE_STRING	ntDeviceName;
UNICODE_STRING	ntDeviceSymbolicName;

WDFDRIVER		wdfDriver;
WDFDEVICE		wdfDevice;
WDFQUEUE        ioQueue;


#ifdef ALLOC_PRAGMA
	#pragma alloc_text (INIT, DriverEntry)
	#pragma alloc_text (PAGE, WOBDriverEvtDriverUnload)
	#pragma alloc_text (PAGE, WOBDriverEvtIoQueueIoDeviceControl)
#endif



NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	NTSTATUS				status			= STATUS_SUCCESS;
	WDF_DRIVER_CONFIG		driverConfig;
	PWDFDEVICE_INIT			pDevInit		= NULL;
	WDF_IO_QUEUE_CONFIG     ioQueueConfig;

	// check version and init version specific structs
	status = WOBDriverInitWinVerOffsets();
	if (!NT_SUCCESS(status)) {
		if( status == STATUS_INCOMPATIBLE_DRIVER_BLOCKED)
			LOG_KMSG("Unsupported Windows Kernel Version.\n");
		else
			LOG_KMSG("Error initializing driver. Error [0x%x]\n", status);
		return status;
	}


	// create the driver object
	WDF_DRIVER_CONFIG_INIT(&driverConfig, WDF_NO_EVENT_CALLBACK);

	driverConfig.DriverInitFlags |= WdfDriverInitNonPnpDriver;
	driverConfig.EvtDriverUnload = &WOBDriverEvtDriverUnload;

	status = WdfDriverCreate( DriverObject,
							  RegistryPath,
							  WDF_NO_OBJECT_ATTRIBUTES,
							  &driverConfig,
							  &wdfDriver);
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Failed to create KMDF driver object.\n");
		goto FINALLY;
	}


	// init non pnp control device object

	pDevInit = WdfControlDeviceInitAllocate(wdfDriver, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R);
	if (pDevInit == NULL) {
		LOG_KMSG("Failed to allocate WDFDEVICE_INIT.\n");
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto FINALLY;
	}

	WdfDeviceInitSetIoType(pDevInit, WdfDeviceIoBuffered);		// buffered IO

	RtlInitUnicodeString(&ntDeviceName, WOBDRIVER_DEVICE_NAME);	// set device name

	status = WdfDeviceInitAssignName(pDevInit, &ntDeviceName);	
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Failed to assign a name to the device.\n");
		goto FINALLY;
	}

	//WdfDeviceInitSetIoInCallerContextCallback( pDevInit, WOBDriverEvtIoInCallerContext);
	//WdfDeviceInitAssignWdmIrpPreprocessCallback(pDevInit, 

	// create non pnp control device object

	status = WdfDeviceCreate(&pDevInit, WDF_NO_OBJECT_ATTRIBUTES, &wdfDevice);

	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Failed to create driver control device.\n");
		goto FINALLY;
	}

	// create device symbolic link

	RtlInitUnicodeString(&ntDeviceSymbolicName, WOBDRIVER_SYMBOLIC_DEVICE_NAME);

	status = WdfDeviceCreateSymbolicLink(wdfDevice, &ntDeviceSymbolicName);

	if (!NT_SUCCESS(status)) {
		if (status != STATUS_OBJECT_NAME_COLLISION) {
			LOG_KMSG("Failed to create control device symbolic link. Error: [%x], %x\n", status, STATUS_OBJECT_NAME_COLLISION);
			goto FINALLY;
		}
		LOG_KMSG("Device symbolic link exists.\n");
		status = STATUS_SUCCESS;
	}

	// create default io queue dispatching requests in parallel
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchParallel);

	ioQueueConfig.EvtIoDeviceControl = WOBDriverEvtIoQueueIoDeviceControl;

	status = WdfIoQueueCreate(wdfDevice, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &ioQueue);
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Failed to create default device queue. error: [%x]\n", status);
		goto FINALLY;
	}

	// done initializing
	WdfControlFinishInitializing(wdfDevice);
	LOG_KMSG("Finished initializing driver.\n");

FINALLY:
	if (pDevInit != NULL) {
		WdfDeviceInitFree(pDevInit);
	}


	return status;
}



void WOBDriverEvtDriverUnload(_In_ WDFDRIVER Driver)
{
	UNREFERENCED_PARAMETER(Driver);
	LOG_KMSG("Unloading driver.\n");
}







void WOBDriverEvtIoQueueIoDeviceControl(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ size_t OutputBufferLength, _In_ size_t InputBufferLength, _In_ ULONG IoControlCode)
{
	NTSTATUS status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(Queue);

	LOG_KMSG("Received control code %x \n", IoControlCode);

	switch (IoControlCode) {
		case IOCTL_OPEN_PROCESS_METHOD_BUFFERED:
			IOCTLHandlerOpenProcess(Request, OutputBufferLength, InputBufferLength);
			break;

		case IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED:
			IOCTLElevateHandle(Request, OutputBufferLength, InputBufferLength);
			break;
	}

	WdfRequestComplete(Request, status);
}

