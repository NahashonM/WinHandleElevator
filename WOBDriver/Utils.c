
#include "Driver.h"
#include "Offsets.h"


NTSTATUS ReadRegistryKeyValue(UNICODE_STRING* key, UNICODE_STRING* valueName, PVOID outBuffer, SIZE_T szOutBuffer)
{
	if (!key || !valueName)
		return STATUS_INVALID_PARAMETER;

	NTSTATUS			status;
	HANDLE				hKey;
	OBJECT_ATTRIBUTES	attributes;

	// get handle 
	InitializeObjectAttributes(&attributes, key, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwOpenKey(&hKey, KEY_QUERY_VALUE, &attributes);
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Failed to open Registry Key.\n");
		return status;
	}

	// get infor size
	ULONG ulKeyInfoSizeNeeded;
	ULONG ulKeyInfoSize;
	status = ZwQueryValueKey(hKey, valueName, KeyValueFullInformation, 0, 0, &ulKeyInfoSizeNeeded);
	if (!ulKeyInfoSizeNeeded || (status != STATUS_BUFFER_TOO_SMALL && status != STATUS_BUFFER_OVERFLOW)) {
		ZwClose(hKey);
		LOG_KMSG("Failed to get Registry Key value size.\n");
		return status;
	}

	ulKeyInfoSize = ulKeyInfoSizeNeeded;

	LOG_KMSG("Reg Key Value size %d.\n", ulKeyInfoSize);

	// read key
	PKEY_VALUE_FULL_INFORMATION pKeyInfo = NULL;
	pKeyInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool2(POOL_FLAG_NON_PAGED, ulKeyInfoSize, DRIVER_TAG);
	if (pKeyInfo == NULL) {
		ZwClose(hKey);
		LOG_KMSG("Failed to allocate memory for Registry Key value info.\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = ZwQueryValueKey(hKey, valueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if (!NT_SUCCESS(status) || ulKeyInfoSize != ulKeyInfoSizeNeeded) {
		ExFreePoolWithTag(pKeyInfo, DRIVER_TAG);
		ZwClose(hKey);
		LOG_KMSG("Failed to get Registry Key value info.\n");
		return status;
	}


	if (szOutBuffer >= pKeyInfo->DataLength)
		RtlCopyMemory(outBuffer, (ULONG*)((ULONG_PTR)pKeyInfo + pKeyInfo->DataOffset), pKeyInfo->DataLength);
	else {
		LOG_KMSG("Failed to write Reg Key value. Output Buffer is too small.\n");
		status = STATUS_BUFFER_TOO_SMALL;
	}

	ExFreePoolWithTag(pKeyInfo, DRIVER_TAG);
	ZwClose(hKey);

	status = STATUS_SUCCESS;

	return status;
}





NTSTATUS GetSystemVersionInfor(PWOB_OS_VERSION_INFO pOsVersion) {
	NTSTATUS			status;
	RTL_OSVERSIONINFOW 	osInfo;
	DWORD				osUpdateRev = 0;

	if (!pOsVersion) 
		return STATUS_INVALID_PARAMETER;

	// get os infor
	osInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

	status = RtlGetVersion(&osInfo);
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Failed to query os version information\n");
		return status;
	}

	// get update revision info
	UNICODE_STRING		keyName;
	UNICODE_STRING		valueName;

	RtlInitUnicodeString(&keyName, OS_VERSION_KEY);
	RtlInitUnicodeString(&valueName, OS_VERSION_KEY_VALUE);

	status = ReadRegistryKeyValue(&keyName, &valueName, &osUpdateRev, sizeof(osUpdateRev));
	if (!NT_SUCCESS(status)) {
		LOG_KMSG("Failed to get update revision info. ErrorL %x\n", status);
		return status;
	}

	pOsVersion->majorVersion = osInfo.dwMajorVersion;
	pOsVersion->minorVersion = osInfo.dwMinorVersion;
	pOsVersion->buildNumber = osInfo.dwBuildNumber;
	pOsVersion->updateRevision = osUpdateRev;

	return status;
}





NTSTATUS FindHandleInHandleTable(HANDLE handle, PVOID handleTable, PVOID* outBuffer) 
{
	UNREFERENCED_PARAMETER(outBuffer);

	// Sanity check
	ULONG64 safeHandle = (ULONG64)handle & (ULONG64)0xFFFFFFFFFFFFFFFC;

	ULONG64 nextHandleNeedingPool = *(ULONG32*)((PUINT8)handleTable + of_HandleTable.NextHandleNeedingPool);

	// this is an invalid handle or handle being created
	if (safeHandle >= nextHandleNeedingPool) {
		LOG_KMSG("Handle not yet available\n");
		return STATUS_INVALID_PARAMETER;
	}

	// get tableCode
	ULONG64 tableCode = *(ULONG64*)((PUINT8)handleTable + of_HandleTable.TableCode);
	ULONG	tableType = tableCode & 3;

	ULONG64 nextTableAddr;

	switch (tableType) {
		case 0:
			*outBuffer = (PVOID)(tableCode + 4 * safeHandle);
			break;
		case 1:
			nextTableAddr = *(ULONG64*)(tableCode + (sizeof(PVOID) * (safeHandle >> 10)) - 1 );
			*outBuffer = (PVOID)(nextTableAddr + (4 * (safeHandle & 0x3FF)));
			break;
		default:
			nextTableAddr = *(ULONG64*)(*(ULONG64*)(tableCode + 8 * (safeHandle >> 19) - 2) + 8 * ((safeHandle >> 10) & 0x1FF));
			*outBuffer = (PVOID)(nextTableAddr + (4 * (safeHandle & 0x3FF)));
			break;
	}

	return STATUS_SUCCESS;
}