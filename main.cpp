
#include <iostream>
#include <list>
#include <tuple>
#include <map>
#include <string>

//Process Control Block, stores each process' data
struct PCB
{
    int PID;
    std::string p_class; // interactive/non_interactive
    int arrival_time;
    std::string current_state;                               //running, ready, or blocked
    std::list<std::pair<std::string, int>> next_instruction; //hold instructions (SSD/CORE/TTY) on what to do next
    std::tuple<std::string, int, int> running_instruction;
};

class Simulation
{
private:
    std::list<PCB> interactive_queue;         //processes done with TTY
    std::list<PCB> non_interactive_queue;     //processes done with SSD or just started
    std::list<PCB> ssd_queue;                 //all processes waiting for ssd
    std::map<int, PCB> given_input;           //processes taken in from input file, key is arrival time
    std::multimap<int, PCB> active_processes; //all processes using a resource, key is processes end-time

    //report statistics
    int ssd_accesses;
    int total_processes;
    int busy_cores;
    int ssd_utilization;
    int simulated_time;

    //resources
    int SSD;
    int cores;
    int max_cores;

    int num_process;
    int terminated_id;             //process just terminated
    int timer;                     //used to check when to release a resource, or put process into memory
    int freed_time;                //time resource gets freed
    bool started;                  //indicates if a process got started
    bool print_terminated;         //flag to print system state when process' terminates
    std::list<PCB> *correct_queue; //decide which queue to use (non-interactive/interactive)

public:
    /* Constructor, takes in number of cores as parameter and initializes all data members */
    Simulation(int x);

    /*  Stores the process (passed in as a parameter) which a resource has been allocated to in the active_processes map,
        along with all it's active-time data. SUCH as:
            - Start time -  time resource was allocated
            - End time -  time process is to release resource
            - Running Instruction - Current instruction the process is executing : SSD, CORE, or TTY
        Also the process' instruction it's currently executing is removed from its list of instructions. */
    void StoreData(PCB &);

    /* Pushes all the processes that were taken in from input file to the given_input map, where the 
    key is the arrival time., removes the "START" and "PID" instructions, from process' list of instructions received as input*/
    void AddNewProcess(PCB &new_process);

    /* After deciding which queue to use (interactive/non-interactive), If that queue is not empty and cores
    are available, a request is made to allocate one core to the process in that queue, until all cores are used or
    the queue being used is empty. For each process that got allocated a core it then calls StoreData(PCB &), 
    passing in the process the core was just allocated to, in order for it to be stored in the active processes map. 
    Finally, it removes process from the queue it came from (interactive/non_interactive) */
    void CoreRequest();

    /* Checks whether or not if a process' start time has been reached, if it has, it then changes that process' state to ready,
    increases the num_process variable by one, sets the freed_time variable to that process' arrival time (will help process' that
    just get loaded when Simulation starts, get correct start time), and then pushes it to back of non_interactive queue and erases 
    it from given_input map. Releaseses as many process until the first process in the active process map has reached its 
    finish time.*/
    bool NewProcessLoadable();

    /* While loop is running until all processes have left the Simulation, checks this by seeing wheter all queues
    and maps are empty. First attempts to see if a new process can be loaded into the non_interactive queue, by calling 
    NewProcessLoadable(), if a process has been loaded into memory and the next process isn't ready it goes on and doesn't 
    keep trying. Then decided which queue to use (interactive/non_interactive), simply by checking if the interactive queue
    is empty. Then allocates as many available cores as it can by calling CoresRequest(), or until the queue is empty.
    Finally, checks if we can free some resources (SSD/CORE), that have been allocated, if all cores are being used
    and one can't be allocated to the next process, it keeps trying until one process releases a core.
    If a process terminated after it released the CORE, it goes on to print the current system state.
     */
    void BeginSimulation();

    /* Checks whether or not the end time has been reached for the first process using a resource in the active_processes map.
        If the time has been met, and :
         - If the first process just got done with the SSD, the SSD variable is changed to 1
        to make it available again.
        -  If the process just got done with a core, the cores variable is incremented by one and the flag 
        freed is changed to true, so we can break out of a the loop of when a process needs  a core but none are freed
        - If the process just got done with the core, and is set to terminate. It removes it from the system, and sets
        the process_terminated flag to true so after this function the call the system state can be printed.
        The time the process released the resource is saved in the freed_time variable, to be used as the start_time for
        the next process that attains the resource. Finally, the MoveToQueue(PCB& ) is called, passing in the process
        to decide where to place the process next. */
    bool CheckFreeResource();

    /* Checks where to put the process (got passed in as a parameter) that just freed it's resources (SSD/CORE/TTY)
        - If the resource freed was a CORE, it then calls ResourceRequest(PCB& ) passing in that process to see whether or 
        not its next instrction orders it to use the SSD or TTY.
        - If the process just freed the SSD, it's state is changed to ready, and it gets  pushed to the back of the
        non_interactive queue, then if there is any processes waiting in the ssd_queue, the first one is woken up, allocates
        it the SSD, and then calls StoreData(PCB&), assing in the just woken  up process, so it's current active-time information
        can be stored in the active_processes map. 
        - If the process just got done with the TTY, it is then simply just pushed to the back of the interactive_queue.
        Finally after the decision for where to put that process is made, it's removed from the active_processes map */
    void MoveToQueue(PCB &ready_process);

    /*  After the process has released a  core, based on the process' next instruction, it's decided in this function
        whether to put that process.
            - IF the next instruction for the process is to use the SSD, if it's available the process gets allocated
            the SSD, and StoreData(PCB& ) is called, to store the process' active-time information for it's usage details
            of the SSD.
            - IF the SSD is not available, then to process is pushed to the back of the ssd_queue
            - IF the next instruction is the TTY, StoreData(PCB& ), is also called to store it's active-time information
            for it's usage of the TTY */
    void ResourceRequest(PCB);

    /* Prints report requested once final process has finished, displaying statistics such 
    as: 
        - simulated_time
        - total number of ssd_accesses,
        - total number of completed_processes
        - average number of busy cores
        - ssd utilization */
    void PrintReport();

    /* Prints the processes inside the given_input, interactive_queue, noninteractive_queue, and ssd_queue */
    void Print(std::list<PCB> print);

    /* Prints the processes inside  active_processes map */
    void PrintActive(std::multimap<int, PCB> print);

    /* If the queues or maps aren't empty, it calls the Print functions for each of the data structures
    and prints them all at once. Also used to print snapshot of the Simulation system when a process starts
    or terminates */
    void PrintAll();
};

Simulation::Simulation(int x)
{
    terminated_id = 0;
    print_terminated = 0;
    ssd_accesses = 0;
    total_processes = 0;
    busy_cores = 0;
    ssd_utilization = 0;
    freed_time = 0;
    started = false;
    num_process = 0;
    SSD = 1;
    max_cores = cores = x;
};

void Simulation::AddNewProcess(PCB &new_process)
{
    //retrieve start time and pid from input
    new_process.arrival_time = new_process.next_instruction.front().second;
    new_process.next_instruction.pop_front();
    new_process.PID = new_process.next_instruction.front().second;
    new_process.next_instruction.pop_front();

    given_input.insert(std::make_pair(new_process.arrival_time, new_process));
}

bool Simulation::NewProcessLoadable()
{
    bool started = false;
    timer = clock(); //dummy variable to release all proceess' until first one in active map is done, will be over-written
    while (!given_input.empty())
    {
        auto p = given_input.begin();
        if (timer >= p->first)
        {
            //make sure new process isn't loaded if its arrival time is > then first running process finish time
            if (active_processes.begin()->first <= p->second.arrival_time && !active_processes.empty())
            {
                started = true;
                break;
            }
            std::cout << "\nPID: " << p->second.PID << " Started at: " << p->second.arrival_time << " ms" << std::endl;

            //Update processes data
            p->second.current_state = "READY";
            p->second.p_class = "non_interactive";
            num_process++;
            total_processes++;
            freed_time = timer = p->second.arrival_time;

            //give first process' core immediately
            if (total_processes <= max_cores)
            {
                p->second.current_state = "RUNNING";
            }
            PrintAll(); //snapshot Simulation system
            non_interactive_queue.push_back(p->second);
            given_input.erase(p->first);
            started = true;
        }
        else
        {
            break;
        }
        ++p;
    }
    return started;
}

void Simulation::BeginSimulation()
{
    while (!interactive_queue.empty() || !active_processes.empty() || !given_input.empty() || !ssd_queue.empty() || !non_interactive_queue.empty())
    {

        //load processes into main memory
        while (!NewProcessLoadable() && !given_input.empty())
        {
            //if processes already in memory, but more not loaded, break
            if (!active_processes.empty() &&
                !interactive_queue.empty() && !non_interactive_queue.empty() && !ssd_queue.empty() && !NewProcessLoadable())
            {
                break;
            }
            continue;
        }

        //use non_interactive queue if interactive is empty
        correct_queue = &interactive_queue;
        if (interactive_queue.empty() && !non_interactive_queue.empty())
        {
            correct_queue = &non_interactive_queue;
        }

        //use cores until all available cores are used, or queue is empty
        while (cores != 0 && !correct_queue->empty())
        {
            CoreRequest();
        }

        //if process that just released a core terminated,  print system state
        if (print_terminated)
        {
            PrintAll();
            std::cout << "PID: " << terminated_id << " , STATE: TERMINATED" << std::endl;
            print_terminated = 0;
        }

        //check if we can release some cores or SSD
        //if all cores being used, and none got free, keep trying
        if (!CheckFreeResource() && cores == max_cores)
        {
            continue;
        };

    }
}

void Simulation::MoveToQueue(PCB &ready_process)
{

    //After core move process to SSD or TTY
    if (std::get<0>(ready_process.running_instruction) == "CORE")
    {
        ResourceRequest(ready_process);
    }
    //if running process just got done with SSD instruction; release SSD
    else if (std::get<0>(ready_process.running_instruction) == "SSD")
    {
        ready_process.current_state = "READY";
        ready_process.p_class = "non_interactive";
        non_interactive_queue.push_back(ready_process);

        //Give SSD to first process in queue
        if (!ssd_queue.empty())
        {
            ssd_accesses++;
            SSD = 0;
            StoreData(ssd_queue.front()); //store process in active process map
            ssd_queue.pop_front();
        }
    } //process just got done with  TTY instructions
    else if (std::get<0>(ready_process.running_instruction) == "TTY")
    {
        ready_process.current_state = "READY";
        ready_process.p_class = "interactive";
        interactive_queue.push_back(ready_process);
    }

    //remove process from active_processes map
    active_processes.erase(active_processes.begin());
}

void Simulation::StoreData(PCB &process)
{

    //store process' current running instruction information
    auto current_request = process.next_instruction.front().first;
    auto run_time = process.next_instruction.front().second;
    auto end_time = freed_time + run_time;

    if (process.PID == correct_queue->front().PID)
    { //CORE REQUEST
        process.current_state = "RUNNING";
        busy_cores += run_time;
    }
    else
    { //TTY or SSD REQUEST
        if (current_request == "SSD")
        {
            ssd_utilization += run_time;
        }
        process.current_state = "BLOCKED";
    }

    //store process run time info in its PCB  EX:(TTY, 1500, 1700)
    process.running_instruction = std::make_tuple(current_request, freed_time, end_time);
    //clear current instruction being used  //EX: remove TTY
    process.next_instruction.pop_front();
    //store requesting_process in active_process map, with end time as key EX:(1700, PCB)
    active_processes.insert(std::make_pair(end_time, process));
}

bool Simulation::CheckFreeResource()
{

    bool freed = false;

    //check end time of first active process to see if we can free any resource
    auto itr = active_processes.begin();
    if (timer >= itr->first)
    {
        //ssd got freed
        if (std::get<0>(itr->second.running_instruction) == "SSD")
        {
            SSD = 1;
        }
        //core got freed
        if (std::get<0>(itr->second.running_instruction) == "CORE")
        {
            freed = true;
            cores++; //release core
        }
        freed_time = itr->first; //store time process freed resource

        //if after freeing core, running process terminates
        if (itr->second.next_instruction.empty())
        {
            num_process--;
            active_processes.erase(active_processes.begin()); //remove from map
            std::cout << "\nProcess: " << itr->second.PID << " terminated at time: " << freed_time << " ms" << std::endl;
            terminated_id = itr->second.PID; //recored ID of just terminated process
            //last process finished
            if (num_process == 0)
            {
                std::cout << "PID: " << terminated_id << " , STATE: TERMINATED" << std::endl;
                std::cout << "\nREPORT: " << std::endl;
                PrintReport();
            }
            else
            {
                print_terminated = 1; //flag to trigger print system state
            }
            return freed;
        }

        //put process in it's next queue
        MoveToQueue(itr->second);
    }

    return freed;
}

void Simulation::ResourceRequest(PCB process)
{

    //check if SSD available
    if (process.next_instruction.front().first == "SSD")
    {

        if (SSD == 1) //availabale
        {
            ssd_accesses++;
            SSD = 0;
        }
        else //unavailable
        {
            //push to the back of SSD_queue
            process.current_state = "BLOCKED";
            ssd_queue.push_back(process);
            return;
        }
    };
    //store process in active process map
    StoreData(process);
}

void Simulation::CoreRequest()
{
    cores--; //use one core for request
    StoreData(correct_queue->front());
    correct_queue->pop_front(); //remove from current_queue
}

void Simulation::Print(std::list<PCB> print)
{
    for (const auto &dq : print)
    {
        std::cout << "PID: " << dq.PID
                  << " , STATE: " << dq.current_state
                  << std::endl;
    }
}

void Simulation::PrintActive(std::multimap<int, PCB> print)
{
    for (const auto &dq : print)
    {
        std::cout << "PID: " << dq.second.PID
                  << " , STATE: "
                  << dq.second.current_state
                  << std::endl;
    }
}

void Simulation::PrintAll()
{
    std::cout << "Process Table: " << std::endl;
    if (!interactive_queue.empty())
    {
        Print(interactive_queue);
    }
    if (!active_processes.empty())
    {
        PrintActive(active_processes);
    }
    if (!non_interactive_queue.empty())
    {
        Print(non_interactive_queue);
    }
    if (!ssd_queue.empty())
    {
        Print(ssd_queue);
    }
}

void Simulation::PrintReport()
{
    std::cout << "Simulated Time: " << freed_time << " ms" << std::endl;
    std::cout << "SSD ACCESSES: " << ssd_accesses << std::endl;
    std::cout << "Completed Processes: " << total_processes << std::endl;   
    std::cout << "Average number of busy cores: " << (float)busy_cores / freed_time << std::endl;
    std::cout << "SSD utilization: " << (float)ssd_utilization / freed_time << std::endl;
}

int main()
{

    std::string inst;
    int t;
    std::cin >> inst;
    std::cin >> t; //num cores

    Simulation stim(t); //start simulation
    PCB *new_process = new PCB;
    int num_process = 0;
    while (1)
    {
        std::cin >> inst;
        std::cin >> t;
        if (inst == "START")
        {
            if (num_process != 0)
            {
                //add process with all it's instructions to simulation
                stim.AddNewProcess(*new_process);
            }
            num_process++;
            delete new_process; 
            //create new process in memory to be filled with it's instructions
            PCB *new_process = new PCB;
        }
        if (inst == "END")
        {
            //push last process and end input
            stim.AddNewProcess(*new_process);
            delete new_process;
            break;
        }

        new_process->next_instruction.push_back(std::make_pair(inst, t));
    };

    stim.BeginSimulation();
}
