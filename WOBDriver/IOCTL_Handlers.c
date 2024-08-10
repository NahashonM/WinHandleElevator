

#include "Driver.h"
#include "Offsets.h"



NTSTATUS IOCTLHandlerOpenProcess(_In_ WDFREQUEST Request, _In_ size_t OutputBufferLength, _In_ size_t InputBufferLength) 
{
	if (OutputBufferLength != sizeof(HANDLE) || InputBufferLength != sizeof(DWORD)) 
		return STATUS_INVALID_PARAMETER;

	PHANDLE		outHandle;
	PDWORD		inProcId;
	NTSTATUS	status;

	status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, (void*)&inProcId, NULL);
	if (!NT_SUCCESS(status))
		return STATUS_INSUFFICIENT_RESOURCES;

	// Open Process
	HANDLE		hProcess	= NULL;
	CLIENT_ID	targetProc	= { (HANDLE)*inProcId, NULL};
	OBJECT_ATTRIBUTES attributes;
	InitializeObjectAttributes(&attributes, NULL, 0, 0, NULL);

	status = ZwOpenProcess(&hProcess, SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL , &attributes, &targetProc);
	if (!NT_SUCCESS(status))
		return status;

	LOG_KMSG("Open Process %d\n", *inProcId);
	
	// write back
	status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, (void*)&outHandle, NULL);
	if (!NT_SUCCESS(status))
		return STATUS_INSUFFICIENT_RESOURCES;

	RtlCopyMemory(outHandle, &hProcess, OutputBufferLength);
	WdfRequestSetInformation(Request, OutputBufferLength);

	return status;
}






NTSTATUS IOCTLElevateHandle(_In_ WDFREQUEST Request, _In_ size_t OutputBufferLength, _In_ size_t InputBufferLength)
{
	if (InputBufferLength != sizeof(ELEVATE_HANDLE_PROCESS_INFO) || OutputBufferLength != sizeof(NTSTATUS))
		return STATUS_INVALID_PARAMETER;
	
	PBOOL							pOutStatus;
	PELEVATE_HANDLE_PROCESS_INFO	pProcInfo;
	NTSTATUS						status;
	PEPROCESS						eProcess;

	status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, (void*)&pProcInfo, NULL);
	if (!NT_SUCCESS(status))
		return STATUS_INSUFFICIENT_RESOURCES;

	// get process EPROCESS
	status = PsLookupProcessByProcessId((HANDLE)pProcInfo->tPID, &eProcess);
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Could not find process. Error: %x \n", status);
		return status;
	}

	// find handle
	PVOID handleAddress = NULL;
	PVOID handleTable = *(PVOID*)((PUINT8)eProcess + of_EProcess.ObjectTable);

	status = FindHandleInHandleTable(pProcInfo->tHandle, handleTable, &handleAddress);
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Could not find handle. Error: %x \n", status);
		return status;
	}

	// elevate handle
	PULONG32 grantedAccessAddr = (ULONG32*)((ULONG64)handleAddress + of_HandleTableEntry.GrantedAccessBits);
	ULONG32 handleAccessValue = *grantedAccessAddr;

	handleAccessValue = handleAccessValue | (pProcInfo->newAccess & 0x1FFFFFF);
	*grantedAccessAddr = handleAccessValue;


	NTSTATUS okay = STATUS_SUCCESS;

	// write back res
	status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, (void*)&pOutStatus, NULL);
	if (!NT_SUCCESS(status))
		return STATUS_INSUFFICIENT_RESOURCES;

	RtlCopyMemory(pOutStatus, &okay, OutputBufferLength);
	WdfRequestSetInformation(Request, OutputBufferLength);

	return status;
}