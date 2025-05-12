// src/main.cpp
#include <chrono>
#include <iostream>
#include <thread>

#include "cpu.h"

int main()
{
    cpu_info_t *cpus = nullptr;
    size_t count = 0;
    if (fetch_cpuinfo(&cpus, &count) != 0)
    {
        std::cerr << "Failed to read /proc/cpuinfo\n";
        return 1;
    }
    std::cout << "Detected " << count << " CPU(s):\n";
    for (size_t i = 0; i < count; ++i)
    {
        auto &c = cpus[i];
        std::cout << "CPU " << c.processor_id << ": " << c.vendor_id << " - "
                  << c.model_name << "\n";
    }
    free_cpu_info(cpus);

    cpu_stat_t s1{}, s2{};
    fetch_cpu_stat(&s1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    fetch_cpu_stat(&s2);
    double pct[CPU_MODE_COUNT];
    compute_cpu_usage(&s1.total, &s2.total, pct);
    std::cout << "\nTotal CPU usage (%):\n";
    for (int m = 0; m < CPU_MODE_COUNT; ++m)
        std::cout << "  " << CPU_MODES[m] << ": " << pct[m] * 100 << "%\n";
    free_cpu_stat(&s1);
    free_cpu_stat(&s2);

    cpu_counters_t ctr{};
    fetch_cpu_counters(&ctr);
    std::cout << "\nContext switches: " << ctr.ctxt
              << "\nInterrupts      : " << ctr.intr
              << "\nProcesses forks : " << ctr.processes << "\n";

    loadavg_t lav{};
    fetch_loadavg(&lav);
    std::cout << "\nLoadavg: " << lav.load1 << " " << lav.load5 << " "
              << lav.load15 << " (" << lav.runnable << "/" << lav.total_threads
              << ")\n";

    for (size_t i = 0; i < count; ++i)
    {
        cpu_freq_t f{};
        fetch_cpu_freq(i, &f);
        std::cout << "CPU " << i << " freq: cur=" << f.cur_khz
                  << " min=" << f.min_khz << " max=" << f.max_khz << "\n";
    }
    return 0;
}