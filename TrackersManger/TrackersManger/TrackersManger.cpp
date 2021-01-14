// TrackersManger.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <iostream>
#include "md5.h"

#ifdef _WIN32
#include <io.h>
#include <tchar.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#include <wininet.h>
#include <string>
#pragma comment(lib, "wininet.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#endif


///////////////////////////////////////GLOBAL/////////////////////////////////
const int G_FILE_LEVEL_NUM = 1;    // num of file level.
const int G_SLEEP_TIME = 60;       // sleep seconds when check trackers over.
//////////////////////////////////////////////////////////////////////////////

bool CopyFile(std::string old_file_path, std::string new_file_path) {
	std::ifstream in(old_file_path.c_str(), std::ios::binary);
	std::ofstream out(new_file_path.c_str(), std::ios::binary);
	if (!in) {
		printf("open file error");
		return false;
	}
	if (!out) {
		printf("open file error");
		return false;
	}
	out << in.rdbuf();
	in.close();
	out.close();
	return true;
}


void MakeTrackersList(std::string curr_path, bool is_first) {
#ifdef _WIN32
	std::string top_tracker_360_path = curr_path + "360_tracker.exe";
	std::string top_tracker_list_path = curr_path + "tracker_path\\tracker_list";
	std::string top_tracker_list_path_tmp = curr_path + "tracker_path\\tracker_list_tmp";
#else
	std::string top_tracker_360_path = curr_path + "360_tracker";
	std::string top_tracker_list_path = curr_path + "tracker_path/tracker_list";
	std::string top_tracker_list_path_tmp = curr_path + "tracker_path/tracker_list_tmp";
#endif
	remove(top_tracker_list_path_tmp.c_str());

#ifdef _WIN32
	intptr_t handle;
	_finddata_t fileinfo;
#else
	DIR* pdir;
	struct dirent* ptr;
#endif
#ifdef _WIN32
	std::string find_path = curr_path + "360_trackers\\" + "*";
	handle = _findfirst(find_path.c_str(), &fileinfo);    // 查找目录中的第一个文件
  if (handle == -1) {
#else
	std::string find_path = curr_path + "360_trackers/";
	if (!(pdir = opendir(find_path.c_str()))) {
#endif
		// do nothing.
	}
#ifdef _WIN32
	do
	{
		if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
			std::string filename = fileinfo.name;
#else
	while ((ptr = readdir(pdir)) != 0) {
		if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
			std::string filename = ptr->d_name;
#endif
			printf("%s\n", filename.c_str());

			// 读取各个 tracker_list
			std::string tracker_list_path = curr_path + "360_trackers\\" + filename + "\\tracker_path\\tracker_list";
			std::ifstream file_tracker_list(tracker_list_path.c_str());
			std::istreambuf_iterator<char> beg(file_tracker_list), end;
			std::string tracker_list_data(beg, end);
			file_tracker_list.close();
			
			// 写入 tmp_list
			std::fstream sfile(top_tracker_list_path_tmp, std::ios::app | std::ios::out | std::ios_base::binary);
			sfile.write(tracker_list_data.c_str(), tracker_list_data.size());
			sfile.close();

			if (is_first) {
#ifdef _WIN32
				std::string tracker_360_path = curr_path + "360_trackers\\" + filename + "\\360_tracker.exe";
				CopyFile(top_tracker_360_path.c_str(), tracker_360_path.c_str());
				STARTUPINFO startupInfo = { 0 };
				PROCESS_INFORMATION  processInformation = { 0 };
				/*打开Word应用程序 C:\\Program Files (x86)\\Microsoft Office\\Office14\\WINWORD.EXE 为程序路径*/
				BOOL bSuccess = CreateProcess(tracker_360_path.c_str(), NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &startupInfo, &processInformation);

				if (bSuccess)
				{
					std::cout << processInformation.dwProcessId << std::endl;
				}
#else
				// 启动进程
				std::string tracker_360_path = curr_path + "360_trackers/" + filename + "/360_tracker";
				CopyFile(top_tracker_360_path.c_str(), tracker_360_path.c_str());
				system(tracker_360_path.c_str());
#endif
			}
		}
#ifdef _WIN32
	} while (!_findnext(handle, &fileinfo));
#else
	}
#endif

#ifdef _WIN32
	_findclose(handle);
#else
	closedir(pdir);
#endif

#ifdef _WIN32
	if (_access(top_tracker_list_path_tmp.c_str(), 0) == -1) {
#else
	if (access(top_tracker_list_path_tmp.c_str(), F_OK) == -1) {
#endif
    // have no tmp. do nothing.
	} else {
		remove(top_tracker_list_path.c_str());
		rename(top_tracker_list_path_tmp.c_str(), top_tracker_list_path.c_str());
	}
}


int main() {
	// 360_tracker 管理平台
	// 1. 遍历现有 torrent 文件，根据 md5 分发到不同的目录
	// 2. 检查该文件夹是否已有 360_tracker:
	//    a .如果有，只复制 torrent 文件;
	//    b. 如果没有，创建目录，拷贝 torrent 文件，拷贝 360_tracker, 执行 360_tracker;
	// 3. 更新 tracker_list
	// 4. sleep 一段时间后，继续遍历。
	// (PS:后续添加进程 PID 的管理，既可以守护进程，又可以关闭指定进程等)

#ifdef _WIN32
	system("taskkill /f /im 360_tracker.exe");
#else
	system("pkill -f 360_tracker");
#endif

	char curr_path[1024];
#ifdef _WIN32
	char all_torrent_path[] = "torrent_path\\";
	char top_tracker_path[] = "tracker_path\\";
	char process_360_trackers_path[] = "360_trackers\\";
#else
	char all_torrent_path[] = "torrent_path/";
	char top_tracker_path[] = "tracker_path/";
	char process_360_trackers_path[] = "360_trackers/";
#endif
#ifdef _WIN32
	::GetModuleFileName(NULL, curr_path, MAX_PATH);
	(_tcsrchr(curr_path, '\\'))[1] = 0;
#else	
	getcwd(curr_path, 1024);
	sprintf(curr_path, "%s/", curr_path);
#endif

	std::string str_all_torrent_path = curr_path;
	str_all_torrent_path = str_all_torrent_path + all_torrent_path;

	std::string str_top_tracker_path = curr_path;
	str_top_tracker_path = str_top_tracker_path + top_tracker_path;

	std::string str_360_trackers_path = curr_path;
	str_360_trackers_path = str_360_trackers_path + process_360_trackers_path;

#ifdef _WIN32
	if (_access(str_all_torrent_path.c_str(), 0) == -1)
	{
		_mkdir(str_all_torrent_path.c_str());
	}
	if (_access(str_top_tracker_path.c_str(), 0) == -1)
	{
		_mkdir(str_top_tracker_path.c_str());
	}
	if (_access(str_360_trackers_path.c_str(), 0) == -1)
	{
		_mkdir(str_360_trackers_path.c_str());
	}
#else
	if (access(str_all_torrent_path.c_str(), F_OK) == -1)
	{
		mkdir(str_all_torrent_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	}
	if (access(str_top_tracker_path.c_str(), F_OK) == -1)
	{
		mkdir(str_top_tracker_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	}
	if (access(str_360_trackers_path.c_str(), F_OK) == -1)
	{
		mkdir(str_360_trackers_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	}
#endif

	std::string last_360_tracker = curr_path;
#ifdef _WIN32
	last_360_tracker  = last_360_tracker + "360_tracker.exe";
#else
	last_360_tracker = last_360_tracker + "360_tracker";
#endif

	MakeTrackersList(curr_path, true);

	bool bquit = false;
	while (!bquit) {
#ifdef _WIN32
		intptr_t handle;
		_finddata_t fileinfo;
#else
		DIR* pdir;
		struct dirent* ptr;
#endif
#ifdef _WIN32
		std::string find_path = str_all_torrent_path + "*.torrent";
		handle = _findfirst(find_path.c_str(), &fileinfo);    // 查找目录中的第一个文件
		if (handle == -1) {
#else
		std::string find_path = str_all_torrent_path;
		if (!(pdir = opendir(find_path.c_str()))) {
#endif
			printf("torrent_path is empty: %s\n", find_path.c_str());
			Sleep(G_SLEEP_TIME * 1000);
			continue;
		}
#ifdef _WIN32
		do
		{
			if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
				std::string filename = fileinfo.name;
#else
		while ((ptr = readdir(pdir)) != 0) {
			if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
				std::string filename = ptr->d_name;
#endif
				printf("%s\n", filename.c_str());

				// 获取 torrent 路径
				std::string old_torrent = str_all_torrent_path;
				old_torrent = old_torrent + filename;

				// 获取文件 md5
				ifstream file(old_torrent.c_str());
				std::string md5 = MD5(file).toString();

				std::string str_file_level = md5.substr(0, G_FILE_LEVEL_NUM);
				std::string str_360_tracker_path = str_360_trackers_path + str_file_level;

				if (_access(str_360_tracker_path.c_str(), 0) == -1)
				{
					// 不存在
					_mkdir(str_360_tracker_path.c_str());

					// 拷贝 360_tracker
#ifdef _WIN32
					std::string copy_360_tracker = str_360_tracker_path + "\\360_tracker.exe";
#else
					std::string copy_360_tracker = str_360_tracker_path + "/360_tracker";
#endif
					CopyFile(last_360_tracker.c_str(), copy_360_tracker.c_str());

#ifdef _WIN32
					{
						std::string dll1 = curr_path;
						dll1 = dll1 + "torrent-rasterbar.dll";
						std::string dll1_copy = str_360_tracker_path;
						dll1_copy = dll1_copy + "\\torrent-rasterbar.dll";
						CopyFile(dll1.c_str(), dll1_copy.c_str());

						std::string dll2 = curr_path;
						dll2 = dll2 + "libssl-1_1.dll";
						std::string dll2_copy = str_360_tracker_path;
						dll2_copy = dll2_copy + "\\libssl-1_1.dll";
						CopyFile(dll2.c_str(), dll2_copy.c_str());

						std::string dll3 = curr_path;
						dll3 = dll3 + "libcrypto-1_1.dll";
						std::string dll3_copy = str_360_tracker_path;
						dll3_copy = dll3_copy + "\\libcrypto-1_1.dll";
						CopyFile(dll3.c_str(), dll3_copy.c_str());
					}
#endif

#ifdef _WIN32
					std::string str_360_tracker_torrent = str_360_tracker_path + "\\torrent_path\\";
#else
					std::string str_360_tracker_torrent = str_360_tracker_path + "/torrent_path/";
#endif
					_mkdir(str_360_tracker_torrent.c_str());

					// 拷贝 torrent
					str_360_tracker_torrent = str_360_tracker_torrent + md5 + ".torrent";
					CopyFile(old_torrent.c_str(), str_360_tracker_torrent.c_str());

#ifdef _WIN32
					//system(copy_360_tracker.c_str());
					STARTUPINFO startupInfo = { 0 };
					PROCESS_INFORMATION  processInformation = { 0 };
					/*打开Word应用程序 C:\\Program Files (x86)\\Microsoft Office\\Office14\\WINWORD.EXE 为程序路径*/
					BOOL bSuccess = CreateProcess(copy_360_tracker.c_str(), NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &startupInfo, &processInformation);

					if (bSuccess)
					{
						std::cout << processInformation.dwProcessId << std::endl;
					}
#else
					// 启动进程
					system(copy_360_tracker.c_str());
#endif
				}
				else {
				  // 存在
#ifdef _WIN32
					std::string str_360_tracker_torrent = str_360_tracker_path + "\\torrent_path\\" + md5 + ".torrent";
#else
					std::string str_360_tracker_torrent = str_360_tracker_path + "/torrent_path/" + md5 + ".torrent";
#endif
					CopyFile(old_torrent.c_str(), str_360_tracker_torrent);
				}
				
			}
#ifdef _WIN32
		} while (!_findnext(handle, &fileinfo));
#else
		}
#endif

#ifdef _WIN32
		_findclose(handle);
#else
		closedir(pdir);
#endif

		MakeTrackersList(curr_path, false);
		Sleep(G_SLEEP_TIME * 1000);
	}


	return 0;
}