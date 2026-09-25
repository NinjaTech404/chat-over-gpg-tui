#ifndef RESOURCES_CONFIG_HPP
#define RESOURCES_CONFIG_HPP

#ifdef _WIN32

  #define NOMINMAX              // kill min/max macros
  #define WIN32_LEAN_AND_MEAN   // skip rarely-used WinAPI junk
  #include <windows.h>
  #include <psapi.h> // GetProcessMemoryInfo

#else

  #include <cstdio> //fopen, fscanf
  #include <unistd.h> // sysconf

#endif

#include <string>

#include <fmt/format.h>
#include <cpumem_monitor/cpumem_monitor.h>

namespace server{

  std::string currentPid() {
    #ifdef _WIN32
      return std::to_string(GetCurrentProcessId());
    #else
      return std::to_string(static_cast<unsigned long>(getpid()));
    #endif
  }


  std::string currentMemoryBytes() {
    #ifdef _WIN32

      PROCESS_MEMORY_COUNTERS pmc{};
      if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) return fmt::format("{:.1f} MB", static_cast<unsigned long long>(pmc.WorkingSetSize) / 1024.0 / 1024.0);
      return 0;

    #elif defined(__linux__)

      // /proc/self/statm: field 2 = RSS in pages
      long rssPages = 0;
      if (FILE* f = fopen("/proc/self/statm", "r")) {
          long vmPages = 0;
          if (fscanf(f, "%ld %ld", &vmPages, &rssPages) != 2) rssPages = 0;
          fclose(f);
      }
      return fmt::format("{:.1f} MB", static_cast<unsigned long long>(rssPages) * sysconf(_SC_PAGESIZE) / 1024.0 / 1024.0);

    #else
      return "0.0";   // unknown POSIX — implement per-platform
    #endif
  }

  std::string currentCpuUsage(){
    SL::NET::CPUMemMonitor mon;
    auto cpuusage = mon.getCPUUsage();
    return fmt::format("{:.1f}%", cpuusage.ProcessUse);
  }
}

#endif // !RESOURCES_CONFIG_HPP
