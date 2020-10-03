Description: 
        Simulate the execution of processes in a system which has one SSD, 
        a given number of cores, and one TTY(input/display). Move the processes to various 
        queues and other data structures based on where they are to go next, and make 
        scheduling decisions on the available resources based on certain given requirements. Report is printed in the end showing system resource usage
        
        EX: move process from core to SSD, allocate free core to next process, 
        move SSD process to inactive queue, etc...


How to Run:
        - USE Mac, Linux, or WSL (windows subsystem for linux)
        - g++ main.cpp -o sim
        - USES IO/Redirection --> ./sim < ./input/input11.txt
        - there are 4 test files in the input file 
