#ifndef RESOURCES_CONFIG_HPP
#define RESOURCES_CONFIG_HPP

#ifdef _WIN32

  #define NOMINMAX              // kill min/max macros
  #define WIN32_LEAN_AND_MEAN   // skip rarely-used WinAPI junk
  #include <windows.h> // for GetCurrentProcessId();
  #include <tlhelp32.h> // for process thread count function

#else

  #include <unistd.h> // for getpid();
  #include <fstream> // for process thread count function
#endif

#include <string>
#include <cmath>
#include <chrono>
#include <memory>

#include <fmt/format.h>

#include <ProcessInfo.h> // for Memory/Cpu usage

namespace server{

  ProcessInfo process; // for Memory/Cpu usage

  std::string currentPid() {
    #ifdef _WIN32
      return std::to_string(GetCurrentProcessId());
    #else
      return std::to_string(static_cast<unsigned long>(getpid()));
    #endif
  }

  std::string currentCpuUsage(){
    return fmt::format("{:.1f}%", process.GetCpuUsage());
  }

  std::string currentMemoryBytes(){
    double usage = static_cast<double>( process.GetMemoryUsage() / static_cast<unsigned int>(std::pow(1024u, 2)) );
    return fmt::format("{:.1f}MB", usage);
  }

  std::string getUptime (const std::shared_ptr<std::chrono::steady_clock::time_point>& start_time){
    auto now = std::chrono::steady_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - (*start_time)).count();
    auto hour = seconds / 3600;
    auto minute = (seconds % 3600) / 60;
    auto second = (seconds % 60);
    return fmt::format("{:02}:{:02}:{:02}", hour, minute, second);
  }


  int getThreadCount(){
    #ifdef _WIN32

      DWORD pid = GetCurrentProcessId();
      HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
      if (snap == INVALID_HANDLE_VALUE) return -1;

        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        int count = 0;

        if (Thread32First(snap, &te)) {
          do {
              if (te.th32OwnerProcessID == pid) count++;
          } while (Thread32Next(snap, &te));
      }

      CloseHandle(snap);
      return count;

    #else

      std::ifstream status("/proc/self/status");
      std::string line;
      while (std::getline(status, line)) {
        if (line.compare(0, 8, "Threads:") == 0) {
            return std::stoi(line.substr(8));
        }
      }

    return -1;

    #endif
  }


}

#endif // !RESOURCES_CONFIG_HPP
