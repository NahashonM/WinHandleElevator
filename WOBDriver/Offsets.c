
#include "Driver.h"
#include "Offsets.h"


OF_EPROCESS					of_EProcess;
OF_HANDLE_TABLE				of_HandleTable;
OF_HANDLE_TABLE_ENTRY		of_HandleTableEntry;


NTSTATUS WOBDriverInitWinVerOffsets() {

	WOB_OS_VERSION_INFO	osInfo;
	NTSTATUS			status;
	

	status = GetSystemVersionInfor(&osInfo);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = STATUS_INCOMPATIBLE_DRIVER_BLOCKED;

	// major:minor 10:0
	if (osInfo.majorVersion == 10 && osInfo.minorVersion == 0 ) {

		// build:update 22631:2861
		if (osInfo.buildNumber == 22631 && osInfo.updateRevision == 2861) {
			
			of_EProcess.ObjectTable = 0x570;

			of_HandleTable.NextHandleNeedingPool = 0x0;
			of_HandleTable.TableCode = 0x8;
			of_HandleTable.QuotaProcess = 0x10;
			of_HandleTable.HandleTableList = 0x18;
			of_HandleTable.Flags = 0x2C;

			of_HandleTableEntry.GrantedAccessBits = 0x8;

			status = STATUS_SUCCESS;
		}
	}
	return status;
}