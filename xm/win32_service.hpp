#ifndef INCLUDE_HPP_CLIB_WIN32_SERVICE
#define INCLUDE_HPP_CLIB_WIN32_SERVICE

#include <string>
#include <windows.h>
#include <tchar.h>

#ifndef CLIB_MAX_PATH
#define CLIB_MAX_PATH 1024
#endif
#include <functional>
using namespace std;
namespace clib
{
    namespace win32
    {
        namespace service
        {
            class Service;
            static const char* kServiceParameterA = "service";
			static const TCHAR* kServiceParameter = TEXT("service");
            typedef std::function<bool()> XSERVICE_FUNC;
            typedef std::function<void(unsigned int, const char*)> XSERVICE_LOG_FUNC;

            namespace ServiceHandle
            {
                static Service* sv;
                void WINAPI Main(DWORD argc, LPTSTR *argv);
                void WINAPI CtrlHandle(DWORD dwControl);
            }

            struct ServiceEntry
            {
                std::basic_string<TCHAR> szServiceName;
                std::basic_string<TCHAR> szServiceDisplayName;
                std::basic_string<TCHAR> szDescription;
                std::basic_string<TCHAR> szDependenicies;

                XSERVICE_FUNC pStart;
                XSERVICE_FUNC pPause;
                XSERVICE_FUNC pContinue;
                XSERVICE_FUNC pStop;
                XSERVICE_FUNC pShutdown;
                XSERVICE_FUNC pInterrogate;
                XSERVICE_FUNC pInstall;
                XSERVICE_FUNC pUnInstall;
                XSERVICE_LOG_FUNC pLogEvent;

                ServiceEntry()
                    : szServiceName(TEXT("MyService")),
                      szServiceDisplayName(TEXT("")),
                      szDescription(TEXT("")),
                      szDependenicies(TEXT("")),
                      pStart(),
                      pPause(),
                      pContinue(),
                      pStop(),
                      pShutdown(),
                      pInterrogate(),
                      pInstall(),
                      pUnInstall(),
                      pLogEvent()
                {

                }
            };

            class Service
            {
            public:
                Service(ServiceEntry *entry);
                ~Service();

                int Run(int argc, char * argv[]);
                int Run(const char* cmdLine);
                void ServiceMainThread(DWORD argc, LPTSTR * argv);
                void ServiceCtrlHandleThread(DWORD dwControl);
                void SetStatus(DWORD dwCurrentState,
                               DWORD dwWin32ExitCode,
                               DWORD dwWaitHint);
                void Installservice();
                void Uninstallservice();
                void LogEvent(const char *format, ...);
                void Errshow();
                void Errshow(const char* tag);


                ServiceEntry* Entry()
                {
                    return entry_;
                }
                void SetHandle(SERVICE_STATUS_HANDLE& handle)
                {
                    handle_ = handle;
                }
                SERVICE_STATUS_HANDLE Handle()
                {
                    return handle_;
                }

                enum LogType
                {
                    kError = 1,
                    kWarn,
                    kPrompt,
                    kCommon,
                    kSys,
                    kLog,
                    kDebug,
                    kSuccess,
                    kInfo
                };
            private:
                bool FindArg(const char* line, const char* key);
				int RunMain();
                bool log_event_;
                ServiceEntry* entry_;
                SERVICE_STATUS_HANDLE handle_;
            };

            namespace ServiceHandle
            {
                //服务全程控制句柄
                inline void WINAPI Main(DWORD argc, LPTSTR *argv)
                {
                    if(!sv)
                    {
                        return;
                    }
                    //装入服务控制句柄
                    SERVICE_STATUS_HANDLE ssHandle = RegisterServiceCtrlHandler(sv->Entry()->szServiceName.c_str(), CtrlHandle);
                    if(ssHandle == NULL)
                    {
                        return;
                    }
                    sv->SetHandle(ssHandle);
                    sv->SetStatus(SERVICE_CONTINUE_PENDING, NO_ERROR, 0);
                    if(!sv->Entry() || !sv->Entry()->pStart || sv->Entry()->pStart())
                    {
                        sv->SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
                        sv->LogEvent("service start success");
                    }
                    else
                    {
                        sv->SetStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 1);
                        sv->LogEvent("service start fail");
                    }
                }
                //服务入口函数
                inline void WINAPI CtrlHandle(DWORD dwControl)
                {
                    if(!sv)
                    {
                        return;
                    }
                    switch(dwControl)
                    {
                    case SERVICE_CONTROL_PAUSE:
                        sv->SetStatus(SERVICE_PAUSE_PENDING, NO_ERROR, 0);
                        if(!sv->Entry() || !sv->Entry()->pPause || sv->Entry()->pPause())
                        {
                            sv->SetStatus(SERVICE_PAUSED, NO_ERROR, 0);
                            sv->LogEvent("service pause success");
                        }
                        else
                        {
                            sv->LogEvent("service pause fail");
                        }
                        break;
                    case SERVICE_CONTROL_CONTINUE:
                        sv->SetStatus(SERVICE_CONTINUE_PENDING, NO_ERROR, 0);
                        if(!sv->Entry() || !sv->Entry()->pContinue || sv->Entry()->pContinue())
                        {
                            sv->SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
                            sv->LogEvent("service continue success");
                        }
                        else
                        {
                            sv->LogEvent("service continue fail");
                        }
                        break;
                    case SERVICE_CONTROL_STOP:
                        sv->SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
                        if(!sv->Entry() || !sv->Entry()->pStop || sv->Entry()->pStop())
                        {
                            sv->SetStatus(SERVICE_STOPPED, NO_ERROR, 0);
                            sv->LogEvent("service stop success");
                        }
                        else
                        {
                            sv->LogEvent("service stop fail");
                        }
                        break;
                    case SERVICE_CONTROL_SHUTDOWN:
                        if(!sv->Entry() || !sv->Entry()->pShutdown || sv->Entry()->pShutdown())
                        {
                            sv->SetStatus(SERVICE_STOPPED, NO_ERROR, 0);
                            sv->LogEvent("service shutdown success");
                        }
                        else
                        {
                            sv->LogEvent("service shutdown fail");
                        }
                        break;
                    case SERVICE_CONTROL_INTERROGATE:
                        if(!sv->Entry() || !sv->Entry()->pInterrogate || sv->Entry()->pInterrogate())
                        {
                            sv->SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
                            sv->LogEvent("service interrogate success");
                        }
                        else
                        {
                            sv->LogEvent("service interrogate fail");
                        }
                        break;
                    }
                }

            }

            inline Service::Service(ServiceEntry *entry)
                : log_event_(false),
                  entry_(entry),
                  handle_(NULL)
            {

            }

            inline Service::~Service()
            {
                ServiceHandle::sv = NULL;
            }

            inline int Service::Run(int argc, char * argv[])
            {
                if(argc >= 3 && _stricmp(argv[1], kServiceParameterA) == 0 && _stricmp(argv[2], "install") == 0)
                {
                    Installservice();
                    if(entry_->pInstall)
                    {
                        entry_->pInstall();
                    }
                    return 0;
                }
                else if(argc >= 3 && _stricmp(argv[1], kServiceParameterA) == 0 && _stricmp(argv[2], "uninstall") == 0)
                {
                    Uninstallservice();
                    if(entry_->pUnInstall)
                    {
                        entry_->pUnInstall();
                    }
                    return 0;
                }

                return RunMain();
            }

            inline int Service::Run(const char* cmdLine)
            {
				if (FindArg(cmdLine, kServiceParameterA) && FindArg(cmdLine, "install"))
				{
					Installservice();
					if (entry_->pInstall)
					{
						entry_->pInstall();
					}
					return 0;
				}
				else if (FindArg(cmdLine, kServiceParameterA) && FindArg(cmdLine, "uninstall"))
				{
					Uninstallservice();
					if (entry_->pUnInstall)
					{
						entry_->pUnInstall();
					}
					return 0;
				}
				return RunMain();
            }

			inline int Service::RunMain()
			{
				ServiceHandle::sv = this;
				SERVICE_TABLE_ENTRY service_table_entry[] =
				{
					{ const_cast<TCHAR*>(entry_->szServiceName.c_str()), &ServiceHandle::Main },
					{ NULL, NULL }
				};
				//获取路径
                
				//执行服务调度器
				//将服务入口函数装入系统进程(等待进程调用)
				if (!StartServiceCtrlDispatcher(service_table_entry))
				{
					TCHAR szfilename[CLIB_MAX_PATH];
					GetModuleFileName(NULL, szfilename, sizeof(szfilename));

					LogEvent("start service lost");
					_tprintf(TEXT("Usage: \n\n\t%s\n\n"), szfilename);
					_tprintf(TEXT("service install          to install the service \n"));
					_tprintf(TEXT("service uninstall        to remove the service \n"));
					_tprintf(TEXT("\n"));

					return 0;
				}
				else
				{
					LogEvent("service run over");
				}
				return 1;
			}

            inline void Service::SetStatus(DWORD dwCurrentState,
                                           DWORD dwWin32ExitCode,
                                           DWORD dwWaitHint)
            {
                static DWORD dwCheckPoint = 1;

                SERVICE_STATUS status;
                status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
                status.dwCurrentState = dwCurrentState;
                status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
                status.dwWin32ExitCode = dwWin32ExitCode;
                status.dwServiceSpecificExitCode = 0;
                status.dwCheckPoint = 0;
                status.dwWaitHint = dwWaitHint;

                if(dwCurrentState == SERVICE_START_PENDING)
                {
                    status.dwControlsAccepted = 0;
                }
                else
                {
                    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
                }

                if((dwCurrentState == SERVICE_RUNNING) ||
                        (dwCurrentState == SERVICE_STOPPED))
                {
                    status.dwCheckPoint = 0;
                }
                else
                {
                    status.dwCheckPoint = dwCheckPoint++;
                }
                // Report the status of the service to the SCM.
                SetServiceStatus(handle_, &status);
            }
            //安装服务
            inline void Service::Installservice()
            {

                SC_HANDLE handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
                if(handle == 0)
                {
                    Errshow();
                    return;
                }
                SC_HANDLE hservicechk = OpenService(handle, entry_->szServiceName.c_str(), DELETE);
                if(hservicechk != 0)
                {
                    DeleteService(hservicechk);
                    CloseServiceHandle(hservicechk);
                }

                TCHAR szfilename[CLIB_MAX_PATH];
                const int n = sizeof(szfilename);
                GetModuleFileName(NULL, szfilename, n);
                //_tcscat_s(szfilename, n, TEXT(" -"));
                //_tcscat_s(szfilename, n, kServiceParameter);
                SC_HANDLE hservice = CreateService(handle,
                                                   entry_->szServiceName.c_str(),
                                                   entry_->szServiceDisplayName.c_str(),
                                                   SERVICE_ALL_ACCESS,
                                                   SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, //允许与桌面交互
                                                   SERVICE_AUTO_START,
                                                   SERVICE_ERROR_IGNORE,
                                                   szfilename,
                                                   NULL, NULL, NULL, NULL, NULL);
                if(hservice == 0)
                {
                    Errshow();
                    return;
                }

                //注册表写入描述
                HKEY hkey;
                LSTATUS ret;
                TCHAR keyp[CLIB_MAX_PATH];
                _stprintf_s<CLIB_MAX_PATH>(keyp, TEXT("%s\\%s"), TEXT("System\\CurrentControlSet\\Services"), entry_->szServiceName.c_str());
                ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyp, 0, KEY_WRITE, &hkey);
                if(ERROR_SUCCESS == ret)
                {
                    RegSetValueEx(hkey,
                                  TEXT("Description"),
                                  0,
                                  REG_SZ,
                                  (CONST BYTE*)entry_->szDescription.c_str(),
                                  (DWORD)entry_->szDescription.size());
                }

                CloseServiceHandle(hservice);
                CloseServiceHandle(handle);

                printf("install service success!\n");
            }
            //卸载服务
            inline void Service::Uninstallservice()
            {
                BOOL ret;

                SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
                SC_HANDLE hService = OpenService(hSCM, entry_->szServiceName.c_str(), DELETE);

                ret = DeleteService(hService);
                if(ret)
                {
                    printf("remove service success!\n");
                }
                else
                {
                    printf("remove service fail!\n");
                }

                CloseServiceHandle(hService);
                CloseServiceHandle(hSCM);

            }
            inline void Service::LogEvent(const char *format, ...)
            {
                if(log_event_ || (entry_ && entry_->pLogEvent))
                {
                    HANDLE hEventSource;
                    va_list args;
                    int len;
                    char *buffer;

                    va_start(args, format);
                    len = _vscprintf(format, args) + 1;
                    buffer = (char *)malloc(len * sizeof(char));
                    vsprintf_s(buffer, len, format, args);

                    if(entry_ && entry_->pLogEvent)
                    {
                        entry_->pLogEvent(kCommon, buffer);
                    }
                    if(log_event_)
                    {
                        hEventSource = RegisterEventSource(NULL, entry_->szServiceName.c_str());
                        if(hEventSource != NULL)
                        {
                            ReportEvent(hEventSource,
                                        EVENTLOG_INFORMATION_TYPE,
                                        0,
                                        0,
                                        NULL,
                                        1,
                                        0,
                                        (LPCTSTR *)&buffer,
                                        NULL);
                            DeregisterEventSource(hEventSource);
                        }
                    }
                    free(buffer);
                }
            }
            inline void Service::Errshow()
            {
                Errshow("Normal");
            }
            inline void Service::Errshow(const char *tag)
            {
                LPVOID lpMsgBuf;
                DWORD ret;
                DWORD dw = GetLastError();
                if(dw > 0)
                {
                    ret = FormatMessageA(
                              FORMAT_MESSAGE_ALLOCATE_BUFFER |
                              FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_IGNORE_INSERTS,
                              NULL,
                              dw,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPSTR)&lpMsgBuf,
                              0,
                              NULL);

                    if(ret)
                    {
                        if(entry_ && entry_->pLogEvent)
                        {
                            char logdata[CLIB_MAX_PATH];
                            sprintf_s<CLIB_MAX_PATH>(logdata, "%s\tERROR[%d]:%s", tag, dw, (LPSTR)lpMsgBuf);
                            entry_->pLogEvent(kError, logdata);
                        }
                        printf("error[%d]: %s\n", dw, (LPSTR)lpMsgBuf);

                    }
                    else
                    {
                        printf("error[%d]\n", dw);
                    }
                    LocalFree(lpMsgBuf);
                }
            }
            inline bool Service::FindArg(const char* line, const char* key)
            {
                std::string cmd(line);
                size_t pos;
                char tmp[260];
                size_t len;

                tmp[0] = '-';
                tmp[1] = '\0';
                strcat_s(tmp, sizeof(tmp) - 1, key);
                len = strlen(tmp);
                pos = cmd.find(tmp);
                if(pos != std::string::npos &&
                        (pos == 0 || cmd[pos - 1] == ' ' || cmd[pos - 1] == '-') &&
                        ((pos + len) == cmd.size() || cmd[pos + len] == ' '))
                {
                    return true;
                }

                tmp[0] = '/';
                tmp[1] = '\0';
                strcat_s(tmp, sizeof(tmp) - 1, key);
                len = strlen(tmp);
                pos = cmd.find(tmp);
                if(pos != std::string::npos &&
                        (pos == 0 || cmd[pos - 1] == ' ' || cmd[pos - 1] == '/') &&
                        ((pos + len) == cmd.size() || cmd[pos + len] == ' '))
                {
                    return true;
                }

                return false;
            }

            inline bool Stop(const TCHAR* ServiceName)
            {
                SC_HANDLE scm, service;
                SERVICE_STATUS status;
                scm = OpenSCManager(
                          NULL,                    // local computer
                          NULL,                    // servicesActive database
                          SC_MANAGER_ALL_ACCESS);  // full access rights

                if(NULL == scm)
                {
                    return false;
                }
                service = OpenService(scm, ServiceName, SERVICE_ALL_ACCESS);
                if(service == NULL)
                {
                    return false;
                }
                if(!QueryServiceStatus(service, &status))
                {
                    return false;
                }
                if(status.dwCurrentState != SERVICE_STOPPED)
                {
                    if(!ControlService(service, SERVICE_CONTROL_STOP, &status))
                    {
                        return false;
                    }
                }
                CloseServiceHandle(service);
                CloseServiceHandle(scm);

                return true;
            }
            inline bool Start(const TCHAR* ServiceName)
            {
                SC_HANDLE scm, service;
                SERVICE_STATUS status;
                scm = OpenSCManager(
                          NULL,                    // local computer
                          NULL,                    // servicesActive database
                          SC_MANAGER_ALL_ACCESS);  // full access rights

                if(NULL == scm)
                {
                    return false;
                }
                service = OpenService(scm, ServiceName, SERVICE_ALL_ACCESS);
                if(service == NULL)
                {
                    return false;
                }
                if(!QueryServiceStatus(service, &status))
                {
                    return false;
                }
                if(status.dwCurrentState == SERVICE_STOPPED)
                {
                    if(!StartService(
                                service,     // handle to service
                                0,           // number of arguments
                                NULL))       // no arguments
                    {
                        CloseServiceHandle(service);
                        CloseServiceHandle(scm);
                        return false;
                    }
                }
                CloseServiceHandle(service);
                CloseServiceHandle(scm);

                return true;
            }
        }
    }
}
#endif