#include "decoder.h"
#include "log.h"

#include <fileapi.h>
#include <Windows.h>
#include <algorithm>

namespace music_data {

INITONLYLOGGER(); 

MusicDecoder::MusicDecoder()
    : m_isValid(true) {
}

MusicDecoder::~MusicDecoder() {
}

bool MusicDecoder::openFile(std::wstring& file_path) {
    const wchar_t* sFileName = file_path.c_str(); 
    return openFile(sFileName); 
}

bool MusicDecoder::openFile(const wchar_t* file_path) {
    // open file for file mapping	
    HANDLE hFile = CreateFileW(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);	
    
    if (hFile==INVALID_HANDLE_VALUE) {
        LOGE("file not exists \n"); 
        return false; 
    }

    // get file size
    size_t nStreamSize = GetFileSize(hFile, NULL); 

    if(nStreamSize == 0xFFFFFFFF || nStreamSize == 0)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE; 
        LOGE("file not exists \n"); 
        return false;
	}

    // create file mapping
    HANDLE hFileMapping = CreateFileMappingW(hFile,NULL,PAGE_READONLY,0,0,NULL);

    if (hFileMapping==INVALID_HANDLE_VALUE) {
        LOGE("CreateFileMappingW fail \n"); 
        CloseHandle(hFile); 
        return false; 
    }

    // create file view
    LPVOID hViewOfFile = MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0); 

    if (hViewOfFile==NULL) {
        LOGE("MapViewOfFile fail \n"); 
        CloseHandle(hFileMapping); 
        CloseHandle(hFile); 
        return false; 
    }

    LOGD("read file successfully, %d byte read", GetFileSize(hFile, NULL)); 

    initData((void*)hViewOfFile, nStreamSize); 

    if (isValid()) {
        m_file_path = std::wstring(file_path); 
    }

    UnmapViewOfFile(hViewOfFile); 
    CloseHandle(hFileMapping); 
    CloseHandle(hFile); 

    return true; 
}

}