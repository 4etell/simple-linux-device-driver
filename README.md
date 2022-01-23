# Linux Device Driver
### Introduction
This is a simple example of using procfs in the device drivers. A user-level program receives input command-line arguments to identify the required path to the target structure(task_struct or pt_regs) from the system tables, transfers it to the kernel level, gets information from this structure and prints the structure. The kernel module code in the **[driver.c](driver.c)** file. User application code in the **[usr.c](usr.c)** file.
### Building and Using driver 
1.	Build the driver by using Makefile (**sudo make**)
2.	Load the driver using **sudo insmod driver.ko**
3.	Compile user application **gcc -o usr usr.c**
4.	Usage: **./usr *struct_id* *PID*** Avaliable struct_id: 0 - pt_regs, 1 - task_struct.
