/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/02/15 [15:2:2009 - 14:58]
* MODULE : D:\Program\R0\Coding\RegHive\code\ParaseHive_new\mfc_test\code\CopyFile.cpp
* 
* Description:
*   老毛子的复制占用文件的方法,在某些机器上不成功. 不能全面支持FAT32/NTFS                     
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#define _WIN32_WINNT 0x0400

#include "CopyFile.h"
#include <windows.h>
#include <winioctl.h>

//////////////////////////////////////////////////////////////////////////

ULONGLONG *
GetFileClusters(
	PCHAR lpFileName,
	ULONG ClusterSize, 
	ULONG *ClCount,
	ULONG *FileSize 
	)
{
    HANDLE  hFile;
    ULONG   OutSize;
    ULONG   Bytes, Cls, CnCount, r;
    ULONGLONG *Clusters = NULL;
    BOOLEAN Result = FALSE;
    LARGE_INTEGER PrevVCN, Lcn;
    STARTING_VCN_INPUT_BUFFER  InBuf;
    PRETRIEVAL_POINTERS_BUFFER OutBuf;
	
    hFile = CreateFile(lpFileName, FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, 0, 0);
	
    if (hFile != INVALID_HANDLE_VALUE)
    {
        *FileSize = GetFileSize(hFile, NULL);
		
        OutSize = sizeof(RETRIEVAL_POINTERS_BUFFER) + (*FileSize / ClusterSize) * sizeof(OutBuf->Extents);
		
        OutBuf = (PRETRIEVAL_POINTERS_BUFFER) malloc(OutSize);
		
        InBuf.StartingVcn.QuadPart = 0;
		
        if (DeviceIoControl(hFile, FSCTL_GET_RETRIEVAL_POINTERS, &InBuf, 
			sizeof(InBuf), OutBuf, OutSize, &Bytes, NULL))
        {
            *ClCount = (*FileSize + ClusterSize - 1) / ClusterSize;
			
            Clusters = (ULONGLONG *) malloc(*ClCount * sizeof(ULONGLONG));
			
            PrevVCN = OutBuf->StartingVcn;
			
            for (r = 0, Cls = 0; r < OutBuf->ExtentCount; r++)
            {
                Lcn = OutBuf->Extents[r].Lcn;
				
                for (CnCount = (ULONG)(OutBuf->Extents[r].NextVcn.QuadPart - PrevVCN.QuadPart);
				CnCount; CnCount--, Cls++, Lcn.QuadPart++) Clusters[Cls] = Lcn.QuadPart;
				
                PrevVCN = OutBuf->Extents[r].NextVcn;
            }
        }
		
        free(OutBuf);	
		
        CloseHandle(hFile);
    }

    return Clusters;
}


void 
FileCopy (
	IN PCHAR lpSrcName,
	IN PCHAR lpDstName
	)
{
    ULONG         ClusterSize, BlockSize;
    ULONGLONG    *Clusters;
    ULONG         ClCount, FileSize, Bytes;
    HANDLE        hDrive, hFile;
    ULONG         SecPerCl, BtPerSec, r;
    PVOID         Buff;
    LARGE_INTEGER Offset;
    CHAR          Name[7];
	
    Name[0] = lpSrcName[0];
    Name[1] = ':';
    Name[2] = 0;
	
    GetDiskFreeSpace(Name, &SecPerCl, &BtPerSec, NULL, NULL);
	
    ClusterSize = SecPerCl * BtPerSec;
	
    Clusters = GetFileClusters(lpSrcName, ClusterSize, &ClCount, &FileSize);
	
    if (Clusters)
    {
        Name[0] = '\\';
        Name[1] = '\\';
        Name[2] = '.';
        Name[3] = '\\';
        Name[4] = lpSrcName[0];
        Name[5] = ':';
        Name[6] = 0;
		
        hDrive = CreateFile(Name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
		
        if (hDrive != INVALID_HANDLE_VALUE)
        {
            hFile = CreateFile(lpDstName, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, 0);
			
            if (hFile != INVALID_HANDLE_VALUE)
            {
                Buff = malloc(ClusterSize);
				
                for (r = 0; r < ClCount; r++, FileSize -= BlockSize)
                {
                    Offset.QuadPart = ClusterSize * Clusters[r];
					
                    SetFilePointer(hDrive, Offset.LowPart, &Offset.HighPart, FILE_BEGIN);
					
                    ReadFile(hDrive, Buff, ClusterSize, &Bytes, NULL);
					
                    BlockSize = FileSize < ClusterSize ? FileSize : ClusterSize;
					
                    WriteFile(hFile, Buff, BlockSize, &Bytes, NULL);
                }
				
                free(Buff);
				
                CloseHandle(hFile);
            }
            CloseHandle(hDrive);
        }
        free(Clusters);
    }

	return ;
}


void 
CopyOneHive (
	IN char* szOrigalPath,
	IN char* szNewPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/28 

Routine Description:
  复制被占用的系统hive句柄到指定路径 eg.
  CopyOneHive( "\\config\\SYSTEM", "c:\\system.hiv" );
  
Arguments:
  szOrigalPath - 要被复制的系统hive文件的不完全路径 eg.
                 "\\config\\SAM" "\\config\\SECURITY"
				 "\\config\\SOFTWARE" "\\config\\SYSTEM"
				 "\\config\\SAM" "\\config\\SECURITY"
  szNewPath - 新路径 eg. "c:\\system.hiv"

--*/
{
	CHAR Name[MAX_PATH] ;

    GetSystemDirectory( Name, MAX_PATH );
    lstrcat( Name, szOrigalPath );
	
	FileCopy( Name, szNewPath );
	return ;
}