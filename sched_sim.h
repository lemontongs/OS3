//
// Created by mlamonta on 4/14/18.
//

#ifndef OS_PROGRAM3_SCHED_SIM_H
#define OS_PROGRAM3_SCHED_SIM_H

class Process
{
public:
    Process(int pid, const std::__cxx11::string &line);

    int get_pid() const;
    int get_priority() const;
    int get_start() const;
    int get_burst_remaining() const;
    int get_turnaround_time() const;
    int get_wait_time() const;

    bool done() const;

    bool ready(int index) const;

    void run(int sim_index);

private:
    int m_pid;
    int m_burst;
    int m_burst_remaining;
    int m_priority;
    int m_start;
    int m_actual_start;
    int m_turnaround_time;
    int m_wait_time;
};


class Statistics
{
public:
    void run(int pid);
    void print_statistics(int sim_index, const std::vector<Process> &processes);
    void print_summary(const std::vector<Process> &processes);

    std::vector<int> m_run_sequence;
    int m_context_switches;
    float m_wt_avg;
    float m_tt_avg;

};

// Scheduling algorithms
void run_sjf(int sim_index, std::vector<Process> &processes, Statistics &stats);
void run_fcfs(int sim_index, std::vector<Process> &processes, Statistics &stats);
void run_rr(int sim_index, std::vector<Process> &processes, Statistics &stats);
bool run_stcf(int sim_index, std::vector<Process> &processes, Statistics &stats);
void run_npp(int sim_index, std::vector<Process> &processes, Statistics &stats);


#endif //OS_PROGRAM3_SCHED_SIM_H
