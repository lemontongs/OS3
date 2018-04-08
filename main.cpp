#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


class Process
{
public:
    Process(const int pid, const std::string &line) :
            m_pid(0),
            m_burst(0),
            m_burst_remaining(0),
            m_priority(0),
            m_start(0)
    {
        m_pid = pid;

        std::stringstream ss(line);
        ss >> m_burst >> m_priority >> m_start;

        m_burst_remaining = m_burst;
    }

    int get_pid() const
    {
        return m_pid;
    }

    int get_priority() const
    {
        return m_priority;
    }

    int get_burst_remaining() const
    {
        return m_burst_remaining;
    }

    bool done() const
    {
        return m_burst_remaining == 0;
    }

    bool ready(const int index) const
    {
        return index > m_start;
    }

    void run()
    {
        if ( ! done() )
        {
            m_burst_remaining--;
        }
    }

private:
    int m_pid;
    int m_burst;
    int m_burst_remaining;
    int m_priority;
    int m_start;
};


bool getProcesses(const std::string &filename, std::vector<Process> &processes)
{
    std::ifstream infile(filename, std::ios_base::in);
    if ( ! infile.is_open() )
    {
        std::cout << "failed to open " << filename << '\n';
        return false;
    }

    processes.clear();
    for (std::string line; std::getline(infile, line); )
    {
        processes.emplace_back(Process(int(processes.size()) + 1, line));
    }

    return true;
}


void run_fcfs(std::vector<Process> &processes)
{
    int sim_index = 1;
    bool done = false;

    // Loop until finished
    while ( ! done )
    {
        // Need to know if no processes need this cycle
        bool none = true;

        // For each process
        for ( auto &p: processes )
        {
            // If the process is ready
            if ( p.ready(sim_index) )
            {
                // Mark the cycle as used
                none = false;

                // Non-premptive, run process until complete
                while ( ! p.done() )
                {
                    p.run();

                    std::cout << sim_index << ":  P" << p.get_pid() << " rem: " << p.get_burst_remaining() << std::endl;

                    sim_index++;
                }
            }
        }

        // Check to see if we are done or not
        done = true;
        for ( auto &p: processes )
        {
            if ( ! p.done() )
            {
                done = false;
                break;
            }
        }

        // No processes ready
        if (none)
        {
            sim_index++;
        }
    }
}


int main()
{
    std::string infilename = "../input.txt";
    std::string outfilename = "../output.txt";

    // Get the processes from the input file
    std::vector<Process> processes;
    if( ! getProcesses(infilename, processes) )
    {
        return 1;
    }

    // Run the simulation
    run_fcfs(processes);

    // Print summary


    return 0;
}
