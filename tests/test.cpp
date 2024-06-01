#include <fileapi.h>
#include <Windows.h>
#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <fstream>

void test_fileLoad() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 虹ヶ咲学園校歌 (Rock Ver.).flac"; 
    const wchar_t* sFileName = fn.c_str(); 

    // std::string fn = "D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 虹ヶ咲学園校歌 (Rock Ver.).flac"; 
    // const char* sFileName = fn.c_str(); 

    // wprintf(L"%S", sFileName); 

    HANDLE hFile = CreateFileW(sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);	
    
    if (hFile==INVALID_HANDLE_VALUE) {
        printf("file not exists \n"); 
        return; 
    } else {
        printf("CreateFileW successfully \n"); 
    }

    uint64_t nStreamSize = GetFileSize(hFile, NULL); 

    HANDLE hFileMapping = CreateFileMappingW(hFile,NULL,PAGE_READONLY,0,0,NULL);

    if (hFileMapping==INVALID_HANDLE_VALUE) {
        printf("CreateFileMappingW fail \n"); 
        CloseHandle(hFile); 
        return; 
    } else {
        printf("CreateFileMappingW successfully \n"); 
    }

    LPVOID hViewOfFile = MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0); 

    if (hViewOfFile==NULL) {
        printf("MapViewOfFile fail \n"); 
        CloseHandle(hFileMapping); 
        CloseHandle(hFile); 
        return; 
    } else {
        printf("MapViewOfFile successfully \n"); 
    }

    uint8_t* pin = (uint8_t*)hViewOfFile; 

    // for (int i=0; i<nStreamSize; ++i) {
    //     if (i%16==0) std::cout << std::setfill('0') << std::setw(6) << i << ": \t"; 
    //     std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)*pin << "\t"; 
    //     ++pin; 
    //     if (i%16==15) std::cout << std::endl; 
    // }

    std::fstream f; 
    f.open("D:\\projects\\C++\\musicmetadata\\bin\\tmp\\hexout", std::ios::out); 

    if (f.is_open()) {
        printf("open file hexout successfully!"); 

        for (int i=0; i<nStreamSize; ++i) {
            if (i%16==0) f << std::setfill('0') << std::setw(6) << i << ": \t"; 
            f << std::hex << std::setfill('0') << std::setw(2) << (int)*pin << "\t"; 
            ++pin; 
            if (i%16==15) {
                f << std::endl; 
            }
            if (i%1600==15) printf("process: %d / %d \n", i, nStreamSize); 
        }

        f.close(); 

    }

    UnmapViewOfFile(hViewOfFile);
    CloseHandle(hFileMapping); 
    CloseHandle(hFile); 
}

void test_fileWrite() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\test_fileWriteOut.ctmd"; 

    const wchar_t* sFileName = fn.c_str(); 

    // wprintf(L"%S", sFileName); 

    HANDLE hFile = CreateFileW(sFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);	
    if (hFile==INVALID_HANDLE_VALUE) {
        printf("file not exists \n"); 
        return; 
    } else {
        printf("CreateFileW successfully \n"); 
    }

    // HANDLE hFileMapping = CreateFileMappingW(hFile,NULL,PAGE_READWRITE,0,0,NULL);

    // if (hFileMapping==INVALID_HANDLE_VALUE) {
    //     printf("CreateFileMappingW fail \n"); 
    //     CloseHandle(hFile); 
    //     return; 
    // } else {
    //     printf("CreateFileMappingW successfully \n"); 
    // }

    //成功写入的数据大小
    DWORD dwWritedDateSize;

    char a[] = "agaweag"; 

    SetFilePointer(hFile,0,0,FILE_BEGIN);
    if(!WriteFile(hFile,a,7,&dwWritedDateSize,NULL))
    {
        printf("写文件失败： %d\n",GetLastError());
    }
    else
    {
        printf("写文件成功，写入%d字节。\n",dwWritedDateSize);
    }

    SetEndOfFile(hFile); 

    // LPVOID hViewOfFile = MapViewOfFile(hFileMapping,FILE_MAP_WRITE,0,0,0); 

    // if (hViewOfFile==NULL) {
    //     printf("MapViewOfFile fail: %d \n", GetLastError()); 
    //     CloseHandle(hFileMapping); 
    //     CloseHandle(hFile); 
    //     return; 
    // } else {
    //     printf("MapViewOfFile successfully \n"); 
    // }

    // memcpy(hViewOfFile, a, 2); 

    // UnmapViewOfFile(hViewOfFile);
    // CloseHandle(hFileMapping); 
    CloseHandle(hFile); 
}

void test_charCmp() {
    char a[] = "adga\0adsga\0dag"; 

    char b[100]; 

    memcpy(b, a, 14); 

    std::cout << b << std::endl; 
}

void test_arrayEqual() {
    struct ts {
        char a[4]; 
        int b; 
    }; 

    ts a; 
    char c[4]; 
    c[0]=6; 
    c[1]=1; 
    c[2]=2; 
    c[3]=3; 
    memcpy(a.a, c, 4); 

    ts b=a; 
    b.a[0]=7; 

    std::cout << a.a[0] << std::endl; 
}

void test_pin() {
    uint8_t* pin = (uint8_t*)malloc(0xFFFF); 

    pin+=0xFFFF+3; 

    free(pin-0xFFFF-3); 
}

int main(int argc, char** argv) {
    test_pin(); 

    printf("over\n"); 
 
    return 0; 
}