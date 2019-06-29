#ifndef INCLUDE_HPP_CLIB_WIN32_PROCESS
#define INCLUDE_HPP_CLIB_WIN32_PROCESS

#include <assert.h>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <tchar.h>
#include <vector>

#include "win32.hpp"
#include <TlHelp32.h>

namespace clib
{
    namespace win32
    {
        namespace process
        {
			struct ProcessInfo
			{
				DWORD ProcessId;
                DWORD ThreadId;
                DWORD Error;
			};

            typedef struct _THREAD_INFO
            {
                LARGE_INTEGER CreateTime;
                DWORD dwUnknown1;
                DWORD dwStartAddress;
                DWORD StartEIP;
                DWORD dwOwnerPID;
                DWORD dwThreadId;
                DWORD dwCurrentPriority;
                DWORD dwBasePriority;
                DWORD dwContextSwitches;
                DWORD Unknown;
                DWORD WaitReason;
            } THREADINFO, *PTHREADINFO;

            typedef struct _UNICODE_STRING
            {
                USHORT Length;
                USHORT MaxLength;
                PWSTR Buffer;
            } UNICODE_STRING;

            typedef struct _PROCESS_INFO
            {
                DWORD dwOffset;
                DWORD dwThreadsCount;
                DWORD dwUnused1[6];
                LARGE_INTEGER CreateTime;
                LARGE_INTEGER UserTime;
                LARGE_INTEGER KernelTime;
                UNICODE_STRING ProcessName;
                DWORD dwBasePriority;
                DWORD dwProcessID;
                DWORD dwParentProcessId;
                DWORD dwHandleCount;
                DWORD dwUnused3[2];
                DWORD dwVirtualBytesPeak;
                DWORD dwVirtualBytes;
                ULONG dwPageFaults;
                DWORD dwWorkingSetPeak;
                DWORD dwWorkingSet;
                DWORD dwQuotaPeakPagedPoolUsage;
                DWORD dwQuotaPagedPoolUsage;
                DWORD dwQuotaPeakNonPagedPoolUsage;
                DWORD dwQuotaNonPagedPoolUsage;
                DWORD dwPageFileUsage;
                DWORD dwPageFileUsagePeak;
                DWORD dCommitCharge;
                THREADINFO ThreadSysInfo[1];
            } PROCESSINFO, *PPROCESSINFO;

            bool KillProcess(DWORD ProPid);

            inline BOOL ExecProcess(const std::basic_string<TCHAR>& pFileStr, std::string* result, bool over_to_kill = false, DWORD* pid = NULL)
            {
                HANDLE hRead = NULL;
                HANDLE hWrite = NULL;

                SECURITY_ATTRIBUTES sa;
                sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                sa.lpSecurityDescriptor = NULL;
                // �´����Ľ��̼̳йܵ���д���
                sa.bInheritHandle = TRUE;
                if(FALSE == CreatePipe(&hRead, &hWrite, &sa, 0))
                {
                    return FALSE;
                }

                if(NULL == hRead || NULL == hWrite)
                {
                    return FALSE;
                }

                // ���������������ԣ���
                STARTUPINFO si;
                si.cb = sizeof(STARTUPINFO);
                GetStartupInfo(&si);
                si.hStdError = hWrite;            // �Ѵ������̵ı�׼��������ض��򵽹ܵ�����
                si.hStdOutput = hWrite;           // �Ѵ������̵ı�׼����ض��򵽹ܵ�����
                si.wShowWindow = SW_HIDE;
                // STARTF_USESHOWWINDOW:The wShowWindow member contains additional information.
                // STARTF_USESTDHANDLES:The hStdInput, hStdOutput, and hStdError members contain additional information.
                si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

                PROCESS_INFORMATION pi;

                // �������� - CREATE_NO_WINDOW��������
                BOOL bSuc = CreateProcess(NULL, const_cast<LPTSTR>(pFileStr.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
                if(NULL != hWrite)
                {
                    CloseHandle(hWrite);
                    hWrite = NULL;
                }
                if(pid)
                {
                    *pid = pi.dwProcessId;
                }

                // �ȷ����ȡ�����ݿռ�
                DWORD dwTotalSize = NEWBUFFERSIZE;                     // �ܿռ�
                char* pchReadBuffer = new char[dwTotalSize];
                memset(pchReadBuffer, 0, NEWBUFFERSIZE);

                DWORD dwFreeSize = dwTotalSize;							// ���ÿռ�
                DWORD dwTotalReadSize = 0;
                do
                {
                    if(FALSE == bSuc)
                    {
                        break;
                    }

                    // ���óɹ���־��֮��Ҫ�Ӷ�ȡ�Ƿ�ɹ�������
                    bSuc = FALSE;

                    char chTmpReadBuffer[NEWBUFFERSIZE] = {0};
                    DWORD dwbytesRead = 0;

                    // ���ڿ��ƶ�ȡƫ��
                    OVERLAPPED Overlapped;
                    memset(&Overlapped, 0, sizeof(OVERLAPPED));

                    while(true)
                    {

                        // ��ջ���
                        memset(chTmpReadBuffer, 0, NEWBUFFERSIZE);

                        // ��ȡ�ܵ�
                        BOOL bRead = ReadFile(hRead, chTmpReadBuffer, NEWBUFFERSIZE, &dwbytesRead, &Overlapped);
                        DWORD dwLastError = GetLastError();

                        if(bRead)
                        {
                            if(dwFreeSize >= dwbytesRead)
                            {
                                memcpy_s(pchReadBuffer + Overlapped.Offset,
                                         dwFreeSize, chTmpReadBuffer, dwbytesRead);  // ���пռ��㹻������£�����ȡ����Ϣ������ʣ�µĿռ���
                                dwFreeSize -= dwbytesRead; // ���¼����¿ռ�Ŀ��пռ�
                            }
                            else
                            {
                                DWORD dwAddSize = (1 + dwbytesRead / NEWBUFFERSIZE) * NEWBUFFERSIZE;   // ����Ҫ����Ŀռ��С
                                DWORD dwNewTotalSize = dwTotalSize + dwAddSize; // �����¿ռ��С
                                dwFreeSize += dwAddSize; // �����¿ռ�Ŀ��д�С
                                char* pTempBuffer = new char[dwNewTotalSize]; // �·�����ʴ�С�Ŀռ�

                                memset(pTempBuffer, 0, dwNewTotalSize);   // ����·���Ŀռ�
                                memcpy_s(pTempBuffer,
                                         dwNewTotalSize, pchReadBuffer, dwTotalSize);  // ��ԭ�ռ����ݿ�������
                                dwTotalSize = dwNewTotalSize; // �����µĿռ��С
                                memcpy_s(pTempBuffer + Overlapped.Offset,
                                         dwFreeSize, chTmpReadBuffer, dwbytesRead);  // ����ȡ����Ϣ���浽�µĿռ���
                                dwFreeSize -= dwbytesRead; // ���¼����¿ռ�Ŀ��пռ�

                                delete [] pchReadBuffer;  // ��ԭ�ռ��ͷŵ�
                                pchReadBuffer = pTempBuffer; // ��ԭ�ռ�ָ��ָ���¿ռ��ַ
                            }

                            // ��ȡ�ɹ����������ȡ������ƫ��
                            Overlapped.Offset += dwbytesRead;
                            dwTotalReadSize += dwbytesRead;
                        }
                        else
                        {
                            if(ERROR_BROKEN_PIPE == dwLastError)
                            {
                                bSuc = TRUE;
                            }
                            break;
                        }
                    }
                }
                while(0);

                if(NULL != hRead)
                {
                    CloseHandle(hRead);
                    hRead = NULL;
                }

                if(bSuc)
                {
                    result->assign(pchReadBuffer, dwTotalReadSize);
                }
                delete [] pchReadBuffer;
                pchReadBuffer = NULL;

                if(over_to_kill)
                {
                    KillProcess(pi.dwProcessId);
                }
                return bSuc;
            }
            inline bool StartProcess(const std::basic_string<TCHAR>& path, bool showWindow = true, ProcessInfo* pinfo = NULL, DWORD* pid = NULL)
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                si.cb = sizeof(STARTUPINFO);
                GetStartupInfo(&si);

                //�Ƿ���ʾ�����Ľ��̵Ĵ���
                if(showWindow)
                {
                    si.wShowWindow = SW_SHOW;
                    si.dwFlags = 0;
                }
                else
                {
                    si.wShowWindow = SW_HIDE;
                    si.dwFlags = STARTF_USESHOWWINDOW;
                }

                BOOL bRet = ::CreateProcess(
                                NULL,
                                const_cast<LPTSTR>(path.c_str()),
                                NULL,
                                NULL,
                                FALSE,
                                CREATE_NEW_CONSOLE,
                                NULL,
                                NULL,
                                &si,
                                &pi);

                if(pinfo != NULL)
                {
                    pinfo->Error = GetLastError();
                }
                if(bRet)
                {
                    ::CloseHandle(pi.hThread);
                    ::CloseHandle(pi.hProcess);

                    if(pid)
                    {
                        *pid = pi.dwProcessId;
                    }

                    if(pinfo != NULL)
                    {
                        pinfo->ProcessId = pi.dwProcessId;
                        pinfo->ThreadId = pi.dwThreadId;
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
            inline bool StartProcessList(const std::vector<std::basic_string<TCHAR> >& names)
            {
                int result = 0;
                for(std::vector<std::basic_string<TCHAR> >::const_iterator iter = names.begin();
                        iter != names.end(); ++iter)
                {
                    if(StartProcess(iter->c_str()))
                    {
                        result++;
                    }
                }
                return (names.size() == result);
            }
            inline long GetPidByWindowTiltle(const TCHAR * title)
            {
                long re = -1;
                HWND hwnd = FindWindow(NULL, title);
                if(hwnd != NULL)
                {
                    unsigned long pid;
                    GetWindowThreadProcessId(hwnd, &pid);
                    re = pid;
                }
                return re;
            }
            inline long GetPidByName(const TCHAR * ProcessName) //CHAR * ProName)
            {

                HANDLE hPSnap = NULL;
                PROCESSENTRY32 pe32 = {0};
                pe32.dwSize = sizeof(PROCESSENTRY32);

                hPSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

                if(hPSnap == INVALID_HANDLE_VALUE)
                {
                    return -1;
                }
                if(Process32First(hPSnap, &pe32))
                {
                    do
                    {
                        if(0 == _tcsicmp(pe32.szExeFile, ProcessName))
                        {
                            CloseHandle(hPSnap);
                            return pe32.th32ProcessID;
                        }

                    }
                    while(Process32Next(hPSnap, &pe32));
                }
                CloseHandle(hPSnap);
                return -1;
            }

            bool ProcessExist(DWORD process_id)
            {
                if(!process_id)
                {
                    return false;
                }

                PROCESSENTRY32 pe;
                DWORD id = 0;

                HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                pe.dwSize = sizeof(PROCESSENTRY32);
                if (!Process32First(hSnapshot, &pe))
                {
                    CloseHandle(hSnapshot);
                    return false;
                }

                while (1)
                {
                    pe.dwSize = sizeof(PROCESSENTRY32);
                    if (Process32Next(hSnapshot, &pe) == FALSE)
                    {
                        CloseHandle(hSnapshot);
                        return false;
                    }

                    if (process_id == pe.th32ProcessID)
                    {
                        CloseHandle(hSnapshot);
                        return true;
                    }
                }
            }

            inline bool KillProcess(DWORD ProPid)
            {
                if (!ProPid)
                {
                    return false;
                }

                HANDLE hd = NULL;
                hd = OpenProcess(PROCESS_TERMINATE, FALSE, ProPid);
                if(hd == INVALID_HANDLE_VALUE)
                {
                    return false;
                }
                bool b = TerminateProcess(hd, 0) ? true : false;
                CloseHandle(hd);
                return b;
            }

            inline int KillProcess(LPCTSTR ProcessName)
            {
                int result = 0;
                HANDLE hPSnap = NULL;
                PROCESSENTRY32 pe32 = { 0 };
                pe32.dwSize = sizeof(PROCESSENTRY32);

                hPSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

                if(hPSnap == INVALID_HANDLE_VALUE)
                {
                    result = -1;
                    return result;
                }
                if(Process32First(hPSnap, &pe32))
                {
                    do
                    {
                        if(0 == _tcsicmp(pe32.szExeFile, ProcessName))
                        {
                            DWORD ProPid = pe32.th32ProcessID;
                            HANDLE hd = OpenProcess(PROCESS_TERMINATE, FALSE, ProPid);
                            if(hd == INVALID_HANDLE_VALUE)
                            {
                                result = -1;
                                break;
                            }
                            if(TerminateProcess(hd, 0))
                            {
                                result++;
                            }
                            else
                            {
                                result = -1;
                                break;
                            }
                            CloseHandle(hd);
                        }

                    }
                    while(Process32Next(hPSnap, &pe32));
                }
                CloseHandle(hPSnap);

                return result;
            }

            inline bool killProcessList(std::vector<std::basic_string<TCHAR> >& names)
            {
                int result = 0;
                for(std::vector<std::basic_string<TCHAR> >::const_iterator iter = names.begin();
                        iter != names.end(); ++iter)
                {
                    if(KillProcess(iter->c_str()) >= 0)
                    {
                        result++;
                    }
                }
                return (names.size() == result);
            }

			inline int GetProcessCpu(DWORD pid)
            {
                int cpuusage;
                PVOID pProcInfo = NULL;
                DWORD dwInfoSize = 0x20000;
                PPROCESSINFO pProcessInfo;
                DWORD dwWorkingSet;

                long(__stdcall * NtQuerySystemInformation)(DWORD, PVOID, DWORD, DWORD);
                static int64_t LastTotalProcessCPUUsage = 0;
                static int64_t LastCurrentProcessCPUUsage = 0;

                int64_t CurrentDelta;
                int64_t TotalDelta;
                int64_t TotalProcessCPUUsage = 0;
                int64_t CurrentProcessCPUUsage = 0;

                pProcInfo = (PVOID)(new _TUCHAR[dwInfoSize]);
                NtQuerySystemInformation = (long(__stdcall*)(DWORD, PVOID, DWORD, DWORD))
                                           GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtQuerySystemInformation");

                NtQuerySystemInformation(5, pProcInfo, dwInfoSize, 0);
                pProcessInfo = (PPROCESSINFO)pProcInfo;
                do
                {
                    TotalProcessCPUUsage += (__int64)pProcessInfo->KernelTime.QuadPart + (__int64)pProcessInfo->UserTime.QuadPart;
                    if(pProcessInfo->dwProcessID == pid)
                    {
                        dwWorkingSet = pProcessInfo->dwWorkingSet;
                        CurrentProcessCPUUsage += (__int64)pProcessInfo->KernelTime.QuadPart + (__int64)pProcessInfo->UserTime.QuadPart;
                    }
                    if(pProcessInfo->dwOffset == 0)
                    {
                        break;
                    }
                    pProcessInfo = (PPROCESSINFO)((_TUCHAR*)pProcessInfo + pProcessInfo->dwOffset);
                }
                while(true);

                TotalDelta = TotalProcessCPUUsage - LastTotalProcessCPUUsage;
                CurrentDelta = CurrentProcessCPUUsage - LastCurrentProcessCPUUsage;
                if(TotalDelta != 0)
                {
                    cpuusage = static_cast<int>(100 * CurrentDelta / TotalDelta);
                }
                LastTotalProcessCPUUsage = TotalProcessCPUUsage;
                LastCurrentProcessCPUUsage = CurrentProcessCPUUsage;
                delete[] pProcInfo;
                return cpuusage;
            }
        }
    }
}
#endif