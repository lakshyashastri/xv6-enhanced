# **<center><ins><p style='color:GREY'>Modified XV6- riscv </p></ins></center>**
***Lakshya Shastri (2021101002) and Adnan Qidwai (2021101115)***

## **This project requires the knowledge of the following concepts related to operating systems:**
- **`Process Scheduling`** 
- **`Synchronization`**
- **`Deadlock`**
- **`Memory Management`**
- **`Virtual memory / Physical memory`**
- **`Paging`**
- **`C programming`**
- **`System calls`**

***
***

## **The project was divided into 4 parts:**
- **`Part 1: System calls`**
- **`Part 2: Process Scheduling`**
- **`Part 3: Page fault handling`**
- **`Part 4: Report`**
- **`Part 5: Analysis`**

***
***

# **System calls**

- ## Trace:
    - For the user-defined `strace` command, we first implement a system call *`trace`* , which will have the mask bits given as an input by the user.
    The input will be entered as:
        ```c
        strace <mask> <command>
        ```
    - The mask has the int value of the bits which will help us to get to know what sys calls we have to display, we compare it with the sys call number assigned to it in `syscall.h` and if the bit is set, we print the sys call name and the arguments passed to it. 
    
    - The `strace` command is implemented in `user.h` and `user.c` and the system call `trace` is implemented in `syscall.c` and `syscall.h`,  we also have to add the system call number in `syscall.h` and the function pointer in `syscall.c`, with 1 argument and `syscall.h` for the system call to work.

    - We also added the attribute maskbits in the proc struct so that every proocess has its own mask bits and we can trace the sys calls of each process separately.

    - We track syscalls in the `syscall()` function in `syscall.c` and print the syscalls.

***
- ## Sigalarm and sigreturn:
    - Sigalarm is implemented as the `sigalarm(interval, handler)` system call, which periodically alerts a given process as it uses CPU time. The system calls are properly defined in all relevant files.
    - This is very useful in cases where a process needs to take periodic actions.
    - After every `interval` CPU ticks, the kernel calls the `handler` function. The application will resume where it left off after `handler` returns.
    - `handler` is a pointer to the function that is supposed to be called.
    - We create a copy of the process trapframe in order to store the registers after the handler function expires. The variables are initialized in `trap.c`
    - We reset the the relevant `struct proc` members when an alarm is outstanding, and we also expire the handling function in `trap.c`
    - For `sigreturn`, the copied trapframe is copied to the main trapframe of the process (kernel related variables aren't directly restored to avoid errors), and the `is_sigalarm` flag is set to 0.

***
# **Process Scheduling**

- ## FCFS scheduling:
    - This is a scheduling algorithm which can be given at the time of compilation of the xv6 kernel as:
        ```
        make qemu SCHEDULER=FCFS
        ```
    - The makefile will hence define the macro `SCHEDULER` as `FCFS` and we can use this macro in the code to implement the FCFS scheduling algorithm to distinguish between the different algorithms.
    
    - The FCFS scheduling algorithm is implemented in `proc.c` and and the `scheduler()` function is modified to implement the FCFS algorithm.    

    - We also disable `yield()` to give up process in trap.c so that the next process has to wait.

    - We track the time it was added and the creation time `ctime` which we will be given as the ticks when process is allocated in `allocproc()` in which we define in the proc struct and compare them to implement the FCFS algorithm.

    - We use locks to make sure that the process is not allocated to more than one CPU at a time.

***

- ## LBS:
    - This is a scheduling algorithm that can be invoked at the time of compilation as such:

        ```ini
        make qemu SCHEDULER=LBS
        ```
    - After the compiling the code as such, the macro `SCHEDULER` is defined by the makefile as `LBS`, which is used in the code to check which scheduling algorithm is to be used.
    - The `LBS` scheduling algorithm is implemented in `proc.c` and and the `scheduler()` function is modified to implement the `LBS` algorithm.
    - Each process is allocated one ticket upon initialization, and it can request more tickets using the `settickets` system call (Even though this is not the best idea since it allows for easy exploitation of the CPU by any process.) 
    - Processes are scheduled according to the number of tickets they have. A process with more tickets is more likely to be scheduled.
    - Every time a process has to be scheduled, all tickets of all (runnable) processes is summed and the probability of a process with a given number of tickets is calculated with that value.
    - We use locks to make sure that the process is not allocated to more than one CPU at a time.
***

- ## PBS:
    - This is a scheduling algorithm which can be given at the time of compilation of the xv6 kernel as:
        ```
        make qemu SCHEDULER=PBS
        ```
    - The makefile will hence define the macro `SCHEDULER` as `PBS` and we can use this macro in the code to implement the PBS scheduling algorithm to distinguish between the different algorithms.

    - We add a new system call `set_priority` which will take the priority as an input and set the priority of the process to the given priority. And we also add the system call number in `syscall.h` and the function pointer in `syscall.c`, with 2 arguments and `syscall.h` for the system call to work.

    - We add a user command setpriority which will take the priority and pid as an input and call the system call `set_priority`. This will be given as follows:
        ```
        setpriority <priority> <pid>
        ```
    - The PBS scheduling algorithm is implemented in `proc.c` and and the `scheduler()` function is modified to implement the PBS algorithm.

    -  We create a new function `getDynamicPriority()` which will calculate the dynamic priority of the process and we will call this function in `scheduler()` to implement the PBS algorithm. We calculate niceness, then dynamic priority. The function is implemented as follows:
        ```c
            niceness=[(wait_time)/(wait_time + run_time)]*10;
            dynamic_priority=max(0,min(100,priority - niceness +5));
        ```

    - We increase waittime in `update_time()` function in `proc.c` and we increase runtime in `scheduler()` function in `proc.c` to calculate the niceness and dynamic priority.

    - We handle the case of equal dynamic priority by checking which process had a lower frequency of context switches and we give the process with lower frequency of context switches a higher priority. We even further, handle the case of equal frequency by checking which process had an earlier creation time and we give the process with earlier creation time a higher priority.

    - We use locks to make sure that the process is not allocated to more than one CPU at a time.

***

# Copy-on-Write:
- In this implementation, we basically create duplicate pages only when both the child and the parent need to modify it. This is done by using the `copyuvm()` function in `vm.c` which is called in `fork()` in `proc.c` to create a copy of the parent's page table and the pages. We also use the newly defined `alloc_cow()` function in `vm.c` which is called in `mappages()` in `vm.c` to create a copy of the page when both the parent and the child need to modify it.
b
- We define a new flag `PTE_COW` which will be set when the page is a copy on write page. 

- We set flags variable in `uvmcopy()` in `vm.c`, to bitwise `OR` of the `PTE_COW` and negation of `PTE_W` flags bitwise `&` with with the flags of the page table entry. This is done to check if the page table entry has no write access. After mapping, we assign the same flags to the childs page table entry. 

- In `copyout()` function, we alloc page table entries checking only if it has a `PTE_COW` flag.

- We add 2 macros `PA2REFERENCE_INDEX(va)` which will give the page index of a given virtual address , it calculates such by the getting the number of pages upto that virtual address from the `KERNBASE`. The second one being `NREFERENCE` which will give the index of the last page, i.e., till the `PHYSTOP`. We create a new array `references` in kmem struct, which will store the number of processes which are using the page. We initialize it to 1 in `allocuvm()` and increment it in `mappages()` and decrement it in `uvmunmap()` and `alloc_cow()`. We intialize it to 1.

- Now we check if more than 1 process is using the page in `kfree()` function and if it is, we just decrement the reference count. If it is 1, we free the page and decrement the reference count. 

- We increase and decrease the reference count by adding two new fucntions `kincr_refcount()` and `kcheck_adn_decr_refcount()` in `kalloc.c`.

- In this way we allocate a copy of page to only those children who need to modify them and not just read.

# **Analysis**

## Round Robin
### CPU = 3; nfork = 10; io = 0
avg rtime= 33

avg wtime= 95
***

### CPU = 3; nfork = 10; io = 5

avg rtime= 16

avg wtime= 115
***

### CPU=1; nfork =10; io=0

avg rtime= 33

avg wtime= 95
***

### CPU=1; nfork =10; io=5

avg rtime= 16

avg wtime= 115
***

## First come first served

### CPU=3; nfork =10; io=0

avg rtime= 39

avg wtime= 47

### CPU=3; nfork =10; io=5

avg rtime= 39

avg wtime= 47

### CPU=1; nfork =10; io=0

avg rtime= 41

avg wtime= 48

### CPU=1; nfork =10; io=5

avg rtime= 41

avg wtime= 48
***

## PBS

### CPU=3; nfork=10; io=5

avg rtime= 20

avg wtime= 108

### CPU=3; nfork =10; IO=0

avg rtime= 39

avg wtime= 47

### CPU=1; nfork=10; IO=5

avg rtime=19

avg wtime= 107

### CPU=1; nfork =10; IO=0
 
avg rtime= 40

avg wtime= 47

***

## LBS
CPU=3; nfork=10; io=5
avg rtime = 42

avg wtime = 127

Note: readtimes and writetimes are relatively high for Lottery based scheduling since xv6 was run using QEMU which was run on an Ubuntu virtual machine
