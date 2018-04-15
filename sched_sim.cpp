//
// Created by mlamonta on 4/14/18.
//

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include "sched_sim.h"

Process::Process(const int pid, const std::__cxx11::string &line) :
        m_pid(0),
        m_burst(0),
        m_burst_remaining(0),
        m_priority(0),
        m_start(0),
        m_actual_start(0),
        m_turnaround_time(0),
        m_wait_time(0)
{
    m_pid = pid;

    std::stringstream ss(line);
    ss >> m_burst >> m_priority >> m_start;

    m_burst_remaining = m_burst;
}

int Process::get_pid() const
{
    return m_pid;
}

int Process::get_priority() const
{
    return m_priority;
}

int Process::get_start() const
{
    return m_start;
}

int Process::get_burst_remaining() const
{
    return m_burst_remaining;
}

int Process::get_turnaround_time() const
{
    return m_turnaround_time;
}

int Process::get_wait_time() const
{
    return m_wait_time;
}


bool Process::done() const
{
    return m_burst_remaining == 0;
}

bool Process::ready(const int index) const
{
    return index > m_start && !done();
}

void Process::run(const int sim_index)
{
    if ( ! done() )
    {
        if (m_burst == m_burst_remaining)
        {
            m_actual_start = sim_index;
        }

        m_burst_remaining--;

        if (done())
        {
            m_turnaround_time = sim_index - m_start;
            m_wait_time = sim_index - m_burst - m_start;
        }
    }
}



void Statistics::run(int pid)
{
    m_run_sequence.push_back(pid);
}

void Statistics::print_statistics(int sim_index, const std::vector<Process> &processes)
{
    std::cout << "t = " << sim_index << std::endl;
    std::cout << "CPU: running process " << m_run_sequence.at(m_run_sequence.size() - 1) << " ";
    std::cout << "(remaining burst = " <<
              processes.at(m_run_sequence.at(m_run_sequence.size()-1)).get_burst_remaining() << ")" << std::endl;

    std::stringstream ready_str;
    for ( auto &p : processes )
    {
        if ( p.ready(sim_index) )
        {
            ready_str << p.get_pid() << "-";
        }
    }

    if ( ready_str.str().empty() )
    {
        std::cout << "Ready queue: empty" << std::endl;
    }
    else
    {
        std::cout << "Ready queue: " << ready_str.str().substr(0, ready_str.str().size() - 1) << std::endl;
    }

    std::cout << std::endl;
}

void Statistics::print_summary(const std::vector<Process> &processes)
{
    std::cout << "######################" << std::endl;
    std::cout << "Summary" << std::endl;

    float wt_avg = 0.0;
    float tt_avg = 0.0;
    for (auto &p: processes)
    {
        std::cout << "P" << p.get_pid()
                  << " : WT=" << p.get_wait_time()
                  << " : TT=" << p.get_turnaround_time() << std::endl;

        wt_avg += p.get_wait_time();
        tt_avg += p.get_turnaround_time();
    }

    m_wt_avg = wt_avg / processes.size();
    m_tt_avg = tt_avg / processes.size();

    std::cout << "AVG  " << m_wt_avg << "    " << m_tt_avg << std::endl;
    std::cout << std::endl;


    std::stringstream result;
    m_context_switches = 0;
    for (unsigned long i = 1; i < m_run_sequence.size(); i++)
    {
        if ( i == 1 )
        {
            result << m_run_sequence.at(i);
        }
        else if ( m_run_sequence.at(i) != m_run_sequence.at(i-1) )
        {
            result << " " << m_run_sequence.at(i);
            m_context_switches++;
        }
    }

    std::cout << "Sequence: " << result.str() << std::endl;
    std::cout << "Context switches: " << m_context_switches << std::endl;
    std::cout << std::endl;
    std::cout << "######################" << std::endl;
    std::cout << std::endl;
}


/*
 * Scheduling algorithms
 *
 */


// State variables
unsigned long SJF_INDEX = 0;
bool SJF_LOCKED = false;

unsigned long RR_INDEX = 0;
bool RR_INITIAL = true;

unsigned long NPP_INDEX = 0;
bool NPP_LOCKED = false;


void run_sjf(int sim_index, std::vector<Process> &processes, Statistics &stats)
{
    // If we are unlocked (current process is complete) find a new process to run
    if ( ! SJF_LOCKED )
    {
        // Find a ready process with the shortest remaining burst time
        int smallest_burst = INT32_MAX;
        for (unsigned long i = 0; i < processes.size(); i++)
        {
            Process* p = &processes.at(i);

            // If the process is ready, and has a smaller burst time, pick it
            if (p->ready(sim_index) && p->get_burst_remaining() < smallest_burst)
            {
                smallest_burst = p->get_burst_remaining();
                SJF_INDEX = i;
                SJF_LOCKED = true;
            }
        }
    }

    // If we have a ready process run until complete
    if ( SJF_LOCKED )
    {
        Process* p = &processes.at(SJF_INDEX);

        if ( p->ready(sim_index) )
        {
            p->run(sim_index);
            stats.run(p->get_pid());

#ifdef DEBUG_PRINTS
            std::cout << sim_index
                      << ":  P" << p->get_pid()
                      << " rem: " << p->get_burst_remaining() << std::endl;
#endif
        }

        // If this process is done, signal to pick a new one
        if ( p->done() )
        {
            SJF_LOCKED = false;
        }
    }
}

void run_fcfs(int sim_index, std::vector<Process> &processes, Statistics &stats)
{
    // For each process
    for ( auto &p: processes )
    {
        // If the process is ready, run it
        if ( p.ready(sim_index) )
        {
            p.run(sim_index);
            stats.run(p.get_pid());

#ifdef DEBUG_PRINTS
            std::cout << sim_index << ":  P" << p.get_pid() << " rem: " << p.get_burst_remaining() << std::endl;
#endif

            return;
        }
    }
}

void run_rr(int sim_index, std::vector<Process> &processes, Statistics &stats)
{
    // Find a new process if time quantum is met
    bool schedule = (sim_index-1) % 2 == 0;
    if ( schedule )
    {
        // Save the starting point
        unsigned long orig = RR_INDEX;

        // Increment
        if ( ! RR_INITIAL )
        {
            RR_INDEX = (RR_INDEX + 1) % processes.size();
        }
        else
        {
            orig = processes.size() - 1;
            RR_INITIAL = false;
        }

        // Search for a new ready process
        bool found = false;
        while( RR_INDEX != orig )
        {
            Process* p = &processes.at(RR_INDEX);

            // If this process is ready then run it
            if ( p->ready(sim_index) )
            {
                p->run(sim_index);
                stats.run(p->get_pid());

#ifdef DEBUG_PRINTS
                std::cout << " T= " << sim_index
                          << ":  P" << p->get_pid()
                          << " rem: " << p->get_burst_remaining() << std::endl;

                found = true;
#endif
                break;
            }
            else
            {
                // increment the RR index to try the next process
                RR_INDEX = (RR_INDEX + 1) % processes.size();
            }
        }

        // If not found log a missed cycle
#ifdef DEBUG_PRINTS
        if ( ! found )
        {
            std::cout << " T= " << sim_index
                      << ":  none" << std::endl;
        }
#endif
    }
    else // inter-quantum, do not allow rescheduling
    {
        Process *p = &processes.at(RR_INDEX);

        // If this process is ready then run it
        if (p->ready(sim_index))
        {
            p->run(sim_index);
            stats.run(p->get_pid());
#ifdef DEBUG_PRINTS
            std::cout << " T= " << sim_index
                      << ":  P" << p->get_pid()
                      << " rem: " << p->get_burst_remaining() << std::endl;

        }
            // If done log a missed cycle
        else
        {
            std::cout << " T= " << sim_index
                      << ":  none" << std::endl;
        }
#else
        }
#endif

    }
}

bool run_stcf(int sim_index, std::vector<Process> &processes, Statistics &stats)
{
    // Find a ready process with the shortest remaining burst time
    bool found = false;
    unsigned long p_ind = 0;
    int smallest_burst = INT32_MAX;
    for (unsigned long i = 0; i < processes.size(); i++)
    {
        Process* p = &processes.at(i);

        // If the process is ready, and has a smaller burst time, pick it
        if (p->ready(sim_index) && p->get_burst_remaining() < smallest_burst)
        {
            found = true;
            smallest_burst = p->get_burst_remaining();
            p_ind = i;
        }
    }

    // If we have a ready process run once
    if ( found )
    {
        Process* p = &processes.at(p_ind);

        p->run(sim_index);
        stats.run(p->get_pid());

#ifdef DEBUG_PRINTS
        std::cout << sim_index
                  << ":  P" << p->get_pid()
                  << " rem: " << p->get_burst_remaining() << std::endl;
#endif
    }
}

void run_npp(int sim_index, std::vector<Process> &processes, Statistics &stats)
{
    // If scheduling is allowed (aka, prevent preemption)
    if ( ! NPP_LOCKED )
    {
        // Find a ready process with the highest priority
        int highest_priority = INT32_MAX;
        for (unsigned long i = 0; i < processes.size(); i++)
        {
            Process *p = &processes.at(i);

            // If the process is ready
            if (p->ready(sim_index) && p->get_priority() < highest_priority)
            {
                highest_priority = p->get_priority();
                NPP_INDEX = i;
                NPP_LOCKED = true;
            }
        }
    }

    // If a process was found or if we are still finishing a process
    if ( NPP_LOCKED )
    {
        Process *p = &processes.at(NPP_INDEX);

        if ( ! p->done() )
        {
            p->run(sim_index);
            stats.run(p->get_pid());
#ifdef DEBUG_PRINTS
            std::cout << sim_index
                      << ":  P" << p->get_pid()
                      << " rem: " << p->get_burst_remaining() << std::endl;
#endif
        }

        if ( p->done() ) // allow rescheduling if this process is done
        {
            NPP_LOCKED = false;
        }
    }
#ifdef DEBUG_PRINTS
    else // empty cycle
    {
        std::cout << " T= " << sim_index
                  << ":  none" << std::endl;
    }
#endif
}

