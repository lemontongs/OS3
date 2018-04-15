#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "sched_sim.h"

bool get_processes(const std::string &filename, std::vector<Process> &processes)
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
        processes.emplace_back(Process(int(processes.size()), line));
    }

    return true;
}

void print_processes(std::vector<Process> &processes)
{
    std::cout << "######################" << std::endl;
    std::cout << "PID : Burst : Pri : Start" << std::endl;

    for (auto &p: processes)
    {
        std::cout << " P" << p.get_pid()
                  << " : " << p.get_burst_remaining()
                  << " : " << p.get_priority()
                  << " : " << p.get_start() << std::endl;
    }
    std::cout << "######################" << std::endl;
}

void print_overall_summary(std::vector<std::string> &types, std::vector<Statistics> &stats)
{
    std::cout << "***** OVERALL SUMMARY *****" << std::endl;
    std::cout << std::endl;
    std::cout << "Wait time comparison" << std::endl;
    for ( unsigned long i = 0; i < types.size(); i++ )
    {
        std::cout << i+1 << "  " << types.at(i) << "  " << stats.at(i).m_wt_avg << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Turnaround time comparison" << std::endl;
    for ( unsigned long i = 0; i < types.size(); i++ )
    {
        std::cout << i+1 << "  " << types.at(i) << "  " << stats.at(i).m_tt_avg << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Context switch comparison" << std::endl;
    for ( unsigned long i = 0; i < types.size(); i++ )
    {
        std::cout << i+1 << "  " << types.at(i) << "  " << stats.at(i).m_context_switches << std::endl;
    }
    std::cout << std::endl;

}


int main(const int argc, char** argv)
{
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <infile> <outfile> <print schedule>" << std::endl;
        return 1;
    }

    std::string infilename = argv[1];
    std::string outfilename = argv[2];

    int print_mod;
    std::stringstream ss(argv[3]);
    ss >> print_mod;

    // Redirect cout to output file
    std::ofstream out(outfilename);
    std::cout.rdbuf(out.rdbuf());

    std::vector<std::string> scheduling_types = {"fcfs", "sjf", "stcf", "rr", "npp"};
    std::vector<Statistics> stats(scheduling_types.size());

    for (unsigned long i = 0; i < scheduling_types.size(); i++)
    {
        // Get the processes from the input file
        std::vector<Process> processes;
        if (!get_processes(infilename, processes))
        {
            return 1;
        }

        std::cout << "****** " << scheduling_types.at(i) << " ******" << std::endl;

        // Print the info that was read from the input file
        print_processes(processes);

        // Run the simulation  fcfs, sjf, stcf, rr, npp

        int sim_index = 1;
        bool done = false;

        // Loop until finished
        while ( ! done )
        {
            switch (i)
            {
                case 0:
                    run_fcfs(sim_index, processes, stats.at(i));
                    break;
                case 1:
                    run_sjf(sim_index, processes, stats.at(i));
                    break;
                case 2:
                    run_stcf(sim_index, processes, stats.at(i));
                    break;
                case 3:
                    run_rr(sim_index, processes, stats.at(i));
                    break;
                case 4:
                    run_npp(sim_index, processes, stats.at(i));
                    break;
                default:
                    break;
            }

            // Check to see if we are done simulating or not
            done = true;
            for ( auto &p: processes )
            {
                if ( ! p.done() )
                {
                    done = false;
                    break;
                }
            }

            // Print what is happening
            if ( (sim_index-1) % print_mod == 0 )
            {
                stats.at(i).print_statistics(sim_index, processes);
            }

            sim_index++;
        }

        // Print summary
        stats.at(i).print_summary(processes);
    }

    // Print overall summary
    print_overall_summary(scheduling_types, stats);

    return 0;
}
