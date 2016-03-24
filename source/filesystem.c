#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>

#include "filesystem.h"

static Handle fsHandle;
FS_Archive saveGameArchive, sdmcArchive;

typedef struct lsLine
{
    FS_DirectoryEntry thisLine;
    struct lsLine* nextLine;
} lsLine;

typedef struct lsDir
{
    struct lsLine* firstLine;
    struct lsDir* parentDir;
} lsDir;

static u32 lowPath[3];
//#ifdef __cia
Result filesystemInit(u64 titleid, FS_MediaType mediatype)
//#else
//Result filesystemInit(void)
//#endif
{
    Result ret;
    
    ret = srvGetServiceHandleDirect(&fsHandle, "fs:USER");
    if(ret)return ret;
    
    if (R_FAILED(ret = FSUSER_Initialize(fsHandle))) return ret;

    fsUseSession(fsHandle, false);

//#ifdef __cia
		lowPath[0] = mediatype;
		lowPath[1] = titleid; /// titleid & 0xFFFFFFFF
		lowPath[2] = titleid >> 32; // (titleid >> 32) & 0xFFFFFFFF

		saveGameArchive = (FS_Archive) { ARCHIVE_USER_SAVEDATA, (FS_Path) { PATH_BINARY, 12, lowPath } };
//#else
//    saveGameArchive = (FS_Archive){ARCHIVE_SAVEDATA, (FS_Path){PATH_EMPTY, 1, (u8*)""}, 0};
//#endif

    ret = FSUSER_OpenArchive(&saveGameArchive);
	printf(" > FSUSER_OpenArchive: %lx\n", ret);

    sdmcArchive = (FS_Archive){ARCHIVE_SDMC, (FS_Path){PATH_EMPTY, 1, (u8*)""}, 0};
    ret = FSUSER_OpenArchive(&sdmcArchive);

    return ret;
}

Result FS_CommitArchive(const FS_Archive* archive)
{
	if (!archive) return -1;

	Result ret;

	//debug_print("FS_CommitArchive:\n");

	ret = FSUSER_ControlArchive(*archive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
	//r(" > FSUSER_ControlArchive: %lx\n", ret);

	return ret;
}

Result filesystemExit(void)
{
    Result ret;
//#ifdef __cia
	ret = FS_CommitArchive(&saveGameArchive);
//#endif
    ret = FSUSER_CloseArchive(&saveGameArchive);
    ret |= FSUSER_CloseArchive(&sdmcArchive);
    fsEndUseSession();
    ret |= svcCloseHandle(fsHandle);
    return ret;
}

void filesystemDoneSaveRead(void)
{
    fsUseSession(fsHandle, false);
}

void filesystemReadySaveRead(void)
{
    fsEndUseSession();
}

Result filesystemSoftReset()
{
    // exit and reinit without giving up those handles
    Result ret;
    
    ret = FSUSER_CloseArchive(&saveGameArchive);
    ret = FSUSER_CloseArchive(&sdmcArchive);

    saveGameArchive = (FS_Archive){ARCHIVE_SAVEDATA, (FS_Path){PATH_EMPTY, 1, (u8*)""}, 0};
    ret = FSUSER_OpenArchive(&saveGameArchive);

    sdmcArchive = (FS_Archive){ARCHIVE_SDMC, (FS_Path){PATH_EMPTY, 1, (u8*)""}, 0};
    ret = FSUSER_OpenArchive(&sdmcArchive);

    return ret;
}

// let's add stuff from 3ds_hb_menu, just because
Result loadFile(char* path, void* dst, FS_Archive* archive, u64 maxSize)
{
    // must malloc first! (and memset, if you'd like)
    if(!path || !dst || !archive)return -1;

    u64 size;
    u32 bytesRead;
    Result ret;
    Handle fileHandle;

    ret=FSUSER_OpenFile(&fileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
    if(ret!=0)return ret;

    ret=FSFILE_GetSize(fileHandle, &size);
    if(ret!=0)goto loadFileExit;
    if(size>maxSize){ret=-2; goto loadFileExit;}

    ret=FSFILE_Read(fileHandle, &bytesRead, 0x0, dst, size);
    if(ret!=0)goto loadFileExit;
    if(bytesRead<size){ret=-3; goto loadFileExit;}

    loadFileExit:
    FSFILE_Close(fileHandle);
    return ret;
}

// oh and let's add in a writeFile, because we kind of need that
Result writeFile(char* path, u8* data, u32 size, FS_Archive* archive)
{
    if(!path || !data)return -1;

    Handle outFileHandle;
    u32 bytesWritten;
    Result ret = 0;

    ret = FSUSER_OpenFile( &outFileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_CREATE | FS_OPEN_WRITE, 0);
    if(ret!=0)return ret;

    ret = FSFILE_Write(outFileHandle, &bytesWritten, 0x0, data, size, 0x10001);
    if(ret!=0)return ret;

    ret = FSFILE_Close(outFileHandle);
    if(ret!=0)return ret;

    if(archive==&saveGameArchive)
    {
        //printf("calling ControlArchive\n");
        ret = FSUSER_ControlArchive(saveGameArchive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
    }

    return ret;
}

// I'll try deleting! that's a good trick!
Result deleteFile(char* path, FS_Archive* archive)
{
    if(!path || !archive)return -1;

    Result ret = FSUSER_DeleteFile(*archive, fsMakePath(PATH_ASCII, path));
    if(ret!=0)return ret;

    if(archive==&saveGameArchive)
    {
        //printf("\ncalling ControlArchive\n");
        ret = FSUSER_ControlArchive(saveGameArchive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
    }
    return ret;
}

u64 sizeFile(char* path, FS_Archive* archive)
{
    if(!path || !archive)return -1;

    u64 size = -1;
    Handle fileHandle;

    Result ret=FSUSER_OpenFile( &fileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
    if(ret!=0)return -1;

    ret=FSFILE_GetSize(fileHandle, &size);
    if(ret!=0)return -1;
    
    FSFILE_Close(fileHandle);
    return size;
}

Result readBytesFromSaveFile(const char* filename, u64 offset, u8* buffer, u32 size)
{
    Result ret;
    Handle fileHandle;

    ret=FSUSER_OpenFile(&fileHandle, saveGameArchive, fsMakePath(PATH_ASCII, filename), FS_OPEN_READ, 0);
    if(ret!=0)return ret;

    ret=FSFILE_Read(fileHandle, NULL, offset, buffer, size);
    if(ret!=0)return ret;
    
    FSFILE_Close(fileHandle);
    return ret;
}

Result writeBytesToSaveFile(const char* filename, u64 offset, u8* buffer, u32 size)
{
    Result ret;
    Handle fileHandle;

    ret=FSUSER_OpenFile(&fileHandle, saveGameArchive, fsMakePath(PATH_ASCII, filename), FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
    if(ret!=0)
        return ret;

    ret=FSFILE_Write(fileHandle, NULL, offset, buffer, size, 0x10001);
    if(ret!=0)
        return ret;
    
    FSFILE_Close(fileHandle);
    ret = FSUSER_ControlArchive(saveGameArchive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
    return ret;
}

Result getSaveGameFileSize(const char* filename, u64* size)
{
    Result ret;
    Handle fileHandle;

    ret=FSUSER_OpenFile(&fileHandle, saveGameArchive, fsMakePath(PATH_ASCII, filename), FS_OPEN_READ, 0);
    if(ret!=0)return ret;

    ret=FSFILE_GetSize(fileHandle, size);
    if(ret!=0)return ret;
    
    FSFILE_Close(fileHandle);
    
    if(*size)
        return ret;
    else
        return -1;
}

Result getSaveFreeBytes(u64* size)
{
    Result ret;

    ret=FSUSER_GetFreeBytes(size, saveGameArchive);
    if(ret!=0)return ret;

    if(*size)
        return ret;
    else
        return -1;
}

Result doesFileNotExist(const char* filename, FS_Archive archive)
{
    Result ret;
    Handle fileHandle;
    u64 size;

    ret=FSUSER_OpenFile( &fileHandle, archive, fsMakePath(PATH_ASCII, filename), FS_OPEN_READ, 0);
    if(ret!=0)return ret;

    ret=FSFILE_GetSize(fileHandle, &size);
    if(ret!=0)return ret;
    
    FSFILE_Close(fileHandle);
    
    if(size)
        return ret;
    else
        return -1;
}

int file_exist(const char * filename)
{
    FILE * pFile;
    long lSize;
    char * buffer;
    size_t result;

    pFile = fopen ( filename , "rb" );
    if (pFile==NULL) return 0;
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);
    buffer = (char*) malloc (sizeof(char)*lSize);
    if (buffer == NULL) return 0;
    result = fread (buffer,1,lSize,pFile);
    if ((long)result != lSize) return 0;
    fclose (pFile);
    free (buffer);
    if (lSize > 0) return 1;
    return 0;
}
