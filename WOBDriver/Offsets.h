#pragma once



typedef struct _OF_EPROCESS {
	ULONG ObjectTable;
} OF_EPROCESS, * POF_EPROCESS;



typedef struct _OF_HANDLE_TABLE {
	ULONG  NextHandleNeedingPool;
	ULONG  TableCode;
	ULONG  QuotaProcess;
	ULONG  HandleTableList;
	ULONG  Flags;
} OF_HANDLE_TABLE, *POF_HANDLE_TABLE;


typedef struct _OF_HANDLE_TABLE_ENTRY {
	ULONG  GrantedAccessBits;
} OF_HANDLE_TABLE_ENTRY, * POF_HANDLE_TABLE_ENTRY;




extern OF_EPROCESS					of_EProcess;
extern OF_HANDLE_TABLE				of_HandleTable;
extern OF_HANDLE_TABLE_ENTRY		of_HandleTableEntry;



EXTERN_C_START

NTSTATUS WOBDriverInitWinVerOffsets();

EXTERN_C_END