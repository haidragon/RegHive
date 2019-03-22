#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
//////////////////////////////////////////////////////////////////////////


ULONGLONG *
GetFileClusters(
	PCHAR lpFileName,
	ULONG ClusterSize, 
	ULONG *ClCount,
	ULONG *FileSize 
	) ;

void 
FileCopy (
	IN PCHAR lpSrcName,
	IN PCHAR lpDstName
	) ;

void 
CopyOneHive (
	IN char* szOrigalPath,
	IN char* szNewPath
	) ;


//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif