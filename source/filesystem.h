#ifndef FILESYSTEM_H
#define FILESYSTEM_H

extern FS_Archive saveGameArchive, sdmcArchive;

//#ifdef __cia
Result filesystemInit(u64 titleid, FS_MediaType mediatype);
//#else
//Result filesystemInit(void);
//#endif
Result filesystemExit(void);
void filesystemReadySaveRead(void);
void filesystemDoneSaveRead(void);

Result filesystemSoftReset();
Result loadFile(char* path, void* dst, FS_Archive* archive, u64 maxSize);
Result writeFile(char* path, u8* data, u32 size, FS_Archive* archive);
Result deleteFile(char* path, FS_Archive* archive);
u64 sizeFile(char* path, FS_Archive* archive);
Result readBytesFromSaveFile(const char* filename, u64 offset, u8* buffer, u32 size);
Result writeBytesToSaveFile(const char* filename, u64 offset, u8* buffer, u32 size);
Result getSaveGameFileSize(const char* filename, u64* size);
Result getSaveFreeBytes(u64* size);
Result doesFileNotExist(const char* filename, FS_Archive archive);
int file_exist (const char *filename);
#endif
