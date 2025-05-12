// src/cpu.cpp
#include "../include/cpu.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <limits.h>
#include <sstream>
#include <sys/stat.h>

#include "../include/tools.h"

int fetch_cpuinfo(cpu_info_t **cpus_out, size_t *count_out)
{
    std::vector<std::string> lines;
    if (readLines("/proc/cpuinfo", lines) < 0)
    {
        *cpus_out = nullptr;
        *count_out = 0;
        return -1;
    }

    std::vector<cpu_info_t> infos;
    cpu_info_t curr{};
    bool in_record = false;
    std::string key, val;

    for (auto &line : lines)
    {
        if (line.empty())
        {
            if (in_record)
            {
                infos.push_back(curr);
                in_record = false;
            }
            continue;
        }
        if (!splitKeyVal(line, key, val))
        {
            continue;
        }

        if (key == "processor")
        {
            if (in_record)
                infos.push_back(curr);
            curr = {};
            in_record = true;
            curr.processor_id = std::stoi(val);
        }
        else if (key == "vendor_id")
        {
            std::strncpy(curr.vendor_id, val.c_str(), MAX_STR_LEN - 1);
        }
        else if (key == "cpu family")
        {
            curr.cpu_family = std::stoi(val);
        }
        else if (key == "model")
        {
            curr.model = std::stoi(val);
        }
        else if (key == "model name")
        {
            std::strncpy(curr.model_name, val.c_str(), MAX_STR_LEN - 1);
        }
        else if (key == "stepping")
        {
            curr.stepping = std::stoi(val);
        }
        else if (key == "microcode")
        {
            std::strncpy(curr.microcode, val.c_str(), MAX_STR_LEN - 1);
        }
        else if (key == "cpu MHz")
        {
            curr.cpu_mhz = std::stod(val);
        }
        else if (key == "cache size")
        {
            curr.cache_size_kb = std::stoi(val);
        }
        else if (key == "physical id")
        {
            curr.physical_id = std::stoi(val);
        }
        else if (key == "siblings")
        {
            curr.siblings = std::stoi(val);
        }
        else if (key == "core id")
        {
            curr.core_id = std::stoi(val);
        }
        else if (key == "cpu cores")
        {
            curr.cpu_cores = std::stoi(val);
        }
    }

    if (in_record)
    {
        infos.push_back(curr);
    }
    if (infos.empty())
    {
        *cpus_out = nullptr;
        *count_out = 0;
        return -1;
    }

    *count_out = infos.size();
    *cpus_out = (cpu_info_t *)std::malloc(infos.size() * sizeof(cpu_info_t));
    if (!*cpus_out)
    {
        return -1;
    }
    std::memcpy(*cpus_out, infos.data(), infos.size() * sizeof(cpu_info_t));
    return 0;
}

void free_cpu_info(cpu_info_t *cpus)
{
    std::free(cpus);
}

int fetch_cpu_stat(cpu_stat_t *st)
{
    std::vector<std::string> lines;
    if (readLines("/proc/stat", lines) < 0)
    {
        return -1;
    }

    size_t count = 0;
    for (auto &l : lines)
    {
        if (l.rfind("cpu", 0) == 0 && l.size() > 3
            && std::isdigit((unsigned char)l[3]))
        {
            ++count;
        }
    }
    st->cpu_count = count;
    st->per_cpu = (cpu_times_t *)std::calloc(count, sizeof(cpu_times_t));
    if (!st->per_cpu)
    {
        return -1;
    }

    auto parse = [&](const std::string &s, cpu_times_t &t) {
        std::istringstream iss(s);
        for (int i = 0; i < CPU_MODE_COUNT; ++i)
        {
            iss >> t.ticks[i];
        }
    };

    size_t idx = 0;
    for (auto &l : lines)
    {
        if (l.rfind("cpu ", 0) == 0)
        {
            parse(l.substr(4), st->total);
        }
        else if (l.rfind("cpu", 0) == 0 && l.size() > 3
                 && std::isdigit((unsigned char)l[3]))
        {
            if (idx < count)
            {
                parse(l.substr(3), st->per_cpu[idx]);
                ++idx;
            }
        }
    }
    return 0;
}

void free_cpu_stat(cpu_stat_t *st)
{
    std::free(st->per_cpu);
    st->per_cpu = nullptr;
    st->cpu_count = 0;
}

int fetch_cpu_counters(cpu_counters_t *ctr)
{
    std::vector<std::string> lines;
    if (readLines("/proc/stat", lines) < 0)
    {
        return -1;
    }

    bool got_ctxt = false, got_intr = false, got_proc = false;
    for (auto &l : lines)
    {
        if (l.rfind("ctxt ", 0) == 0)
        {
            ctr->ctxt = std::stoull(l.substr(5));
            got_ctxt = true;
        }
        else if (l.rfind("intr ", 0) == 0)
        {
            ctr->intr = std::stoull(l.substr(5));
            got_intr = true;
        }
        else if (l.rfind("processes ", 0) == 0)
        {
            ctr->processes = std::stoull(l.substr(10));
            got_proc = true;
        }
    }
    return (got_ctxt && got_intr && got_proc) ? 0 : -1;
}

void compute_cpu_usage(const cpu_times_t *prev, const cpu_times_t *curr,
                       double pct[CPU_MODE_COUNT])
{
    unsigned long long total = 0, delta[CPU_MODE_COUNT];
    for (int i = 0; i < CPU_MODE_COUNT; ++i)
    {
        delta[i] = curr->ticks[i] - prev->ticks[i];
        total += delta[i];
    }
    for (int i = 0; i < CPU_MODE_COUNT; ++i)
    {
        pct[i] = total ? static_cast<double>(delta[i]) / total : 0.0;
    }
}

int fetch_cpu_freq(int cpu, cpu_freq_t *f)
{
    struct stat st;
    char path[PATH_MAX];

    // Try scaling_cur_freq first
    std::snprintf(path, sizeof(path),
                  "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq",
                  cpu);
    if (stat(path, &st) == 0)
    {
        if (readSingleValue(path, f->cur_khz) < 0)
            return -1;
    }
    else
    {
        // Fallback to cpuinfo_cur_freq
        std::snprintf(path, sizeof(path),
                      "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_cur_freq",
                      cpu);
        if (stat(path, &st) == 0)
        {
            if (readSingleValue(path, f->cur_khz) < 0)
                return -1;
        }
        else
        {
            return -1;
        }
    }

    // scaling_min_freq (optional)
    std::snprintf(path, sizeof(path),
                  "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq",
                  cpu);
    if (stat(path, &st) == 0)
    {
        readSingleValue(path, f->min_khz);
    }
    else
    {
        f->min_khz = 0;
    }

    // scaling_max_freq (optional)
    std::snprintf(path, sizeof(path),
                  "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",
                  cpu);
    if (stat(path, &st) == 0)
    {
        readSingleValue(path, f->max_khz);
    }
    else
    {
        f->max_khz = 0;
    }

    return 0;
}

int fetch_loadavg(loadavg_t *lav)
{
    std::vector<std::string> lines;
    if (readLines("/proc/loadavg", lines) < 0 || lines.empty())
    {
        return -1;
    }

    std::istringstream iss(lines[0]);
    char slash;
    if (!(iss >> lav->load1 >> lav->load5 >> lav->load15 >> lav->runnable
          >> slash >> lav->total_threads))
    {
        return -1;
    }
    return 0;
}
