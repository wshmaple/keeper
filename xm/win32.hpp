#ifndef INCLUDE_HPP_CLIB_WIN32
#define INCLUDE_HPP_CLIB_WIN32

#include <windows.h>
#include <assert.h>
#include <algorithm>
#include <string>
#include <tchar.h>

#define NEWBUFFERSIZE 0x100
namespace clib
{
    namespace win32
    {
        static const bool kLittleEndian = true;

        //ANSI to Unicode
        inline std::wstring ANSIToUnicode( const std::string& str )
        {
            size_t  len = 0;
            len = str.length();
            int  unicodeLen = ::MultiByteToWideChar( CP_ACP,
                              0,
                              str.c_str(),
                              -1,
                              NULL,
                              0 );
            wchar_t *  pUnicode;
            pUnicode = new  wchar_t[unicodeLen + 1];
            memset(pUnicode, 0, (unicodeLen + 1)*sizeof(wchar_t));
            ::MultiByteToWideChar( CP_ACP,
                                   0,
                                   str.c_str(),
                                   -1,
                                   (LPWSTR)pUnicode,
                                   unicodeLen );
            std::wstring  rt;
            rt = ( wchar_t* )pUnicode;
            delete  pUnicode;

            return  rt;
        }
        //Unicode to ANSI
        inline std::string UnicodeToANSI( const std::wstring& str )
        {
            char* pElementText;
            int iTextLen;
            // wide char to multi char
            iTextLen = WideCharToMultiByte( CP_ACP,
                                            0,
                                            str.c_str(),
                                            -1,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL );
            pElementText = new char[iTextLen + 1];
            memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
            ::WideCharToMultiByte( CP_ACP,
                                   0,
                                   str.c_str(),
                                   -1,
                                   pElementText,
                                   iTextLen,
                                   NULL,
                                   NULL );
            std::string strText;
            strText = pElementText;
            delete[] pElementText;
            return strText;
        }

        //UTF-8 to Unicode
        inline std::wstring UTF8ToUnicode( const std::string& str )
        {
            size_t  len = 0;
            len = str.length();
            int  unicodeLen = ::MultiByteToWideChar( CP_UTF8,
                              0,
                              str.c_str(),
                              -1,
                              NULL,
                              0 );
            wchar_t *  pUnicode;
            pUnicode = new  wchar_t[unicodeLen + 1];
            memset(pUnicode, 0, (unicodeLen + 1)*sizeof(wchar_t));
            ::MultiByteToWideChar( CP_UTF8,
                                   0,
                                   str.c_str(),
                                   -1,
                                   (LPWSTR)pUnicode,
                                   unicodeLen );
            std::wstring  rt;
            rt = ( wchar_t* )pUnicode;
            delete  pUnicode;

            return  rt;
        }

        //Unicode to UTF-8
        inline std::string UnicodeToUTF8( const std::wstring& str )
        {
            char* pElementText;
            int iTextLen;
            // wide char to multi char
            iTextLen = WideCharToMultiByte( CP_UTF8,
                                            0,
                                            str.c_str(),
                                            -1,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL );
            pElementText = new char[iTextLen + 1];
            memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
            ::WideCharToMultiByte( CP_UTF8,
                                   0,
                                   str.c_str(),
                                   -1,
                                   pElementText,
                                   iTextLen,
                                   NULL,
                                   NULL );
            std::string strText;
            strText = pElementText;
            delete[] pElementText;
            return strText;
        }
#ifdef UNICODE
#else
#endif

        
#ifdef  UNICODE
#define AnsiToTStr ANSIToUnicode
#define TStrToAnsi UnicodeToANSI
#define Utf8ToTStr UTF8ToUnicode
#define TStrToUtf8 UnicodeToUTF8
#else
#define AnsiToTStr
#define TStrToAnsi
#define Utf8ToTStr
#define TStrToUtf8
#endif

        inline bool GetHeapProfile(void (*func)(void*, const char*, int), void* arg)
        {
            return false;
        }

        inline bool FolderExists(std::string& dir)
        {
            DWORD attr;
            attr = GetFileAttributesA(dir.c_str());
            return (attr != (DWORD)(-1) ) &&
                   ( attr & FILE_ATTRIBUTE_DIRECTORY);
        }

        inline bool CreateMuliteDirectory(std::string& dir)
        {
            std::string path = dir;
            size_t len = dir.size();
            if(len < 4) return FolderExists(path);
            if('\\' != path[len - 1])
            {
                return false;
            }
            if(FolderExists(path))
            {
                return true;
            }
            size_t pos = path.rfind('\\', len - 2);

            std::string parent(path.c_str(), pos + 1);
            if(CreateMuliteDirectory(parent))
            {
                if('\\' != path[len - 2])
                {
                    SECURITY_ATTRIBUTES sa;
                    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                    sa.lpSecurityDescriptor = NULL;
                    sa.bInheritHandle = 0;
                    return (CreateDirectoryA(dir.c_str(), &sa) == TRUE);
                }
                else
                {
                    return true;
                }
            }
            return false;
        }
        inline std::string GetCurrentFileName()
        {
            char path[MAX_PATH];
            ::GetModuleFileNameA(::GetModuleHandleA(NULL), path, MAX_PATH - 1);
            return std::string(strrchr(path, '\\') + 1);
        }
        inline std::wstring GetCurrentFileNameW()
        {
            wchar_t path[MAX_PATH];
            ::GetModuleFileNameW(::GetModuleHandleW(NULL), path, MAX_PATH - 1);
            return std::wstring(wcsrchr(path, '\\') + 1);
        }
        inline std::basic_string<TCHAR> GetCurrentFile()
        {
            TCHAR path[MAX_PATH];
            ::GetModuleFileName(::GetModuleHandle(NULL), path, MAX_PATH);
            return std::basic_string<TCHAR>(path);
        }

        inline std::basic_string<TCHAR> GetCurrentDir()
        {
            TCHAR path[MAX_PATH];
            ::GetModuleFileNameW(::GetModuleHandleW(NULL), path, MAX_PATH - 1);
            *(_tcsrchr(path, TEXT('\\')) + 1) = 0;
            return std::basic_string<TCHAR>(path);
        }

        static const std::basic_string<TCHAR> CurrentDir = GetCurrentDir();

        inline std::basic_string<TCHAR> ModifyPath(const std::basic_string<TCHAR>& path)
        {
            std::basic_string<TCHAR> res;

            if(path.empty())
            {
                return res;
            }

            if(path[0] == TEXT('/') || path[0] == TEXT('\\'))
            {
                res = CurrentDir + path;
            }
            else if(path[0] == TEXT('.'))
            {
                res  = CurrentDir + TEXT("\\") + path.substr(path[1] == TEXT('/') ? 2 : 1);
            }
            else if (path.find(TEXT("/")) == std::string::npos && path.find(TEXT("\\")) == std::string::npos)
            {
                res = CurrentDir + TEXT("\\") + path;
            }

            size_t i = 0, j = 0;
            for(; i < res.size(); ++i)
            {
                if(res[i] == TEXT('/'))
                {
                    res[i] = TEXT('\\');
                }
                if(res[i] == TEXT('\\') && (res[i + 1] == TEXT('/') || res[i + 1] == TEXT('\\')))
                {
                    continue;
                }
                if (j < i)
                {
                    res[j] = res[i];
                }
                j++;
            }
            if (i > j)
            {
                res.resize(j);
            }
            return res;
        }

        inline std::basic_string<TCHAR> ModifyDir(std::basic_string<TCHAR>& dir)
        {
            if(dir.empty())
            {
                return dir;
            }
            ModifyPath(dir);
            if(dir[dir.size() - 1] != TEXT('\\'))
            {
                dir.push_back(TEXT('\\'));
            }
            return dir;
        }

        inline bool FileExists(std::string& file)
        {
            DWORD attr;
            attr = GetFileAttributesA(file.c_str());
            return (attr != (DWORD)(-1) ) &&
                   !( attr & FILE_ATTRIBUTE_DIRECTORY);
        }
        inline std::string GetLastErrSz()
        {
            LPVOID lpMsgBuf;
            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                0, // Default language
                (LPSTR) &lpMsgBuf,
                0,
                NULL);

            std::string Err = (LPSTR)lpMsgBuf;
            LocalFree(lpMsgBuf);
            return Err;
        }
        inline std::wstring GetLastErrSzW()
        {
            LPVOID lpMsgBuf;
            FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                0, // Default language
                (LPWSTR) &lpMsgBuf,
                0,
                NULL
            );
            std::wstring Err = (LPCWSTR)lpMsgBuf;
            LocalFree(lpMsgBuf);
            return Err;
        }

        inline size_t GetPageSize()
        {
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            return std::max<DWORD>(si.dwPageSize, si.dwAllocationGranularity);
        }

        const size_t g_PageSize = GetPageSize();

        typedef void (*ScheduleProc)(void*);
        struct WorkItemWrapper
        {
            WorkItemWrapper(ScheduleProc proc, void* content): proc_(proc), content_(content) {}
            ScheduleProc proc_;
            void* content_;
        };
        inline DWORD WINAPI WorkItemWrapperProc(LPVOID content)
        {
            WorkItemWrapper* item = static_cast<WorkItemWrapper*>(content);
            ScheduleProc TempProc = item->proc_;
            void* arg = item->content_;
            delete item;
            TempProc(arg);
            return 0;
        }

        inline void set_current_dir()
        {
            TCHAR path[MAX_PATH];
            ::GetModuleFileName(::GetModuleHandle(NULL), path, MAX_PATH);
            *(_tcsrchr(path, TEXT('\\')) + 1) = 0;
            ::SetCurrentDirectory(path);
        }
    }
}

#endif