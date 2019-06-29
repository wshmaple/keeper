// xm.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include "json_db.h"
#include "fun.hpp"
#include "win32.hpp"
#include "win32_service.hpp"
#include "win32_process.hpp"

#define CONFIG_JSON_FILENAME "WindowsService.conf"

using namespace clib;
using namespace clib::win32;

int main(int argc, char* argv[])
{
	set_current_dir();

	std::shared_ptr<JsonDB> json_db(new JsonDB(CONFIG_JSON_FILENAME));
	json_db->init();

	std::string filepath = json_db->get("FilePath", std::string()).asString();
	std::string service_name = json_db->get("ServiceName", std::string()).asString();
	std::string service_show = json_db->get("ServiceDisplayName", std::string()).asString();
	std::string service_desc = json_db->get("ServiceDescription", std::string()).asString();

	if (filepath.empty() || service_name.empty()
		|| service_show.empty() || service_desc.empty())
	{
		printf("the config json file parse fail (" CONFIG_JSON_FILENAME ")");
		return 1;
	}

	std::basic_string<TCHAR> filepath_tstr = ModifyPath(Utf8ToTStr(filepath));
	service::ServiceEntry entry;
	entry.szServiceName = Utf8ToTStr(service_name);
	entry.szServiceDisplayName = Utf8ToTStr(service_show);
	entry.szDescription = Utf8ToTStr(service_desc);

	bool chk_run = true;
	std::shared_ptr<std::thread> chk_thread;
	process::ProcessInfo pinfo;
	std::function<void() > chk_func = [&filepath_tstr, &pinfo, &chk_run]()
	{
		while (chk_run)
		{
			if (pinfo.ProcessId)
			{
				if (!process::ProcessExist(pinfo.ProcessId))
				{
					process::StartProcess(filepath_tstr, false, &pinfo);
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	};

	entry.pStart = [&filepath_tstr, &pinfo, &chk_run, &chk_thread, chk_func]()->bool
	{
		bool res = true;

		if (filepath_tstr.size() > 2)
		{
			res = process::StartProcess(filepath_tstr, false, &pinfo);

			chk_run = true;

			if (res)
			{
				chk_thread.reset(new std::thread(chk_func));
			}
		}

		return res;
	};

	entry.pStop = [&pinfo, &chk_run, &chk_thread]()->bool
	{
		if (chk_thread)
		{
			chk_run = false;
			chk_thread->join();

			if (pinfo.ProcessId && process::ProcessExist(pinfo.ProcessId))
			{
				int i = 0;

				while (i < 5 && !process::KillProcess(pinfo.ProcessId))
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					i++;
				}
			}
		}

		return true;
	};

	entry.pLogEvent = [](unsigned int type, const char* msg)
	{
		switch (type)
		{
		case service::Service::kError:
		case service::Service::kWarn:
		case service::Service::kPrompt:
		case service::Service::kCommon:
		case service::Service::kSys:
		case service::Service::kLog:
		case service::Service::kDebug:
		case service::Service::kSuccess:
		case service::Service::kInfo:
#ifdef _DEBUG
			// CLOG(kNorm, msg);
#endif
			break;
		};
	};

	service::Service serv(&entry);
	return serv.Run(argc, argv);
}