// include/cpu.h
#ifndef CPU_H
#define CPU_H

#include <cstddef>

#define MAX_STR_LEN 128

static constexpr int CPU_MODE_COUNT = 10;
static const char *CPU_MODES[CPU_MODE_COUNT] = {
    "user", "nice",    "system", "idle",  "iowait",
    "irq",  "softirq", "steal",  "guest", "guest_nice"
};

struct cpu_info_t
{
    int processor_id;
    char vendor_id[MAX_STR_LEN];
    int cpu_family;
    int model;
    char model_name[MAX_STR_LEN];
    int stepping;
    char microcode[MAX_STR_LEN];
    double cpu_mhz;
    int cache_size_kb;
    int physical_id;
    int siblings;
    int core_id;
    int cpu_cores;
};

struct cpu_times_t
{
    unsigned long long ticks[CPU_MODE_COUNT];
};

struct cpu_stat_t
{
    cpu_times_t total;
    cpu_times_t *per_cpu;
    std::size_t cpu_count;
};

struct cpu_counters_t
{
    unsigned long long ctxt;
    unsigned long long intr;
    unsigned long long processes;
};

struct cpu_freq_t
{
    int cur_khz;
    int min_khz;
    int max_khz;
};

struct loadavg_t
{
    double load1;
    double load5;
    double load15;
    int runnable;
    int total_threads;
};

// Fetch static /proc/cpuinfo data
int fetch_cpuinfo(cpu_info_t **cpus_out, std::size_t *count_out);
void free_cpu_info(cpu_info_t *cpus);

// Fetch CPU tick stats from /proc/stat
int fetch_cpu_stat(cpu_stat_t *st);
void free_cpu_stat(cpu_stat_t *st);

// Fetch kernel counters from /proc/stat
int fetch_cpu_counters(cpu_counters_t *ctr);

// Compute percentage usage across modes
void compute_cpu_usage(const cpu_times_t *prev, const cpu_times_t *curr,
                       double pct[CPU_MODE_COUNT]);

// Fetch per‐CPU frequencies via sysfs
int fetch_cpu_freq(int cpu, cpu_freq_t *f);

// Fetch loadavg and run-queue info
int fetch_loadavg(loadavg_t *lav);

#endif // CPU_H