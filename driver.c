#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/mutex.h>

#define BUF_SIZE 1024

static int pid = 1;
static int struct_id = 1;

static struct proc_dir_entry *parent;

struct mutex lab_mutex;
/*
** Function Prototypes
*/
static int __init

lab_driver_init(void);

static void __exit

lab_driver_exit(void);

/***************** Procfs Functions *******************/
static int open_proc(struct inode *inode, struct file *file);

static int release_proc(struct inode *inode, struct file *file);

static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length, loff_t * offset);

static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t *off);

static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};


static size_t write_pt_regs_struct(char __user *ubuf, struct task_struct *taskStruct) {
    char buf[BUF_SIZE];
    size_t len = 0;

    struct pt_regs *regs = task_pt_regs(taskStruct);

    len += sprintf(buf+len, "di: %ld\n", regs->di);
    len += sprintf(buf+len, "si: %ld\n", regs->si);
    len += sprintf(buf+len, "dx: %ld\n", regs->dx);
    len += sprintf(buf+len, "cx: %ld\n", regs->cx);
    len += sprintf(buf+len, "ax: %ld\n", regs->ax);
    len += sprintf(buf+len, "bx: %ld\n", regs->bx);
    len += sprintf(buf+len, "bp: %ld\n", regs->bp);
    len += sprintf(buf+len, "r8: %ld\n", regs->r8);
    len += sprintf(buf+len, "r9: %ld\n", regs->r9);
    len += sprintf(buf+len, "r10: %ld\n", regs->r10);
    len += sprintf(buf+len, "r11: %ld\n", regs->r11);
    len += sprintf(buf+len, "r12: %ld\n", regs->r12);
    len += sprintf(buf+len, "r13: %ld\n", regs->r13);
    len += sprintf(buf+len, "r14: %ld\n", regs->r14);
    len += sprintf(buf+len, "error code on interrupt: %ld\n", regs->orig_ax);
    len += sprintf(buf+len, "flags: %ld\n", regs->flags);

    if (copy_to_user(ubuf, buf, len)) {
        return -EFAULT;
    }
    return len;
}

static size_t write_task_struct(char __user *ubuf, struct task_struct *taskStruct) {
    char buf[BUF_SIZE];
    size_t len = 0;

    len += sprintf(buf+len, "state (-1 unrunnable, 0 runnable, >0 stopped) = %ld\n", taskStruct->state);
    len += sprintf(buf+len, "exit_state = %d\n", taskStruct->exit_state);
    len += sprintf(buf+len, "exit_code = %d\n", taskStruct->exit_code);
    len += sprintf(buf+len, "exit_signal = %d\n", taskStruct->exit_signal);
    len += sprintf(buf+len, "the signal sent when the parent dies = %d\n", taskStruct->pdeath_signal);
    len += sprintf(buf+len, "flags requiring atomic access = %ld\n", taskStruct->atomic_flags);
    len += sprintf(buf+len, "sessionid = %d\n", taskStruct->sessionid);


    if (copy_to_user(ubuf, buf, len)){
        return -EFAULT;
    }
    return len;
}

static int open_proc(struct inode *inode, struct file *file) {
    mutex_lock(&lab_mutex);
    pr_info("Proc file opend\t");
    return 0;
}

/*
** This function will be called when we close the procfs file
*/
static int release_proc(struct inode *inode, struct file *file) {
    mutex_unlock(&lab_mutex);
    pr_info("Proc file released\n");
    return 0;
}

/*
** This function will be called when we read the procfs file
*/
static ssize_t read_proc(struct file *filp, char __user *ubuf, size_t count, loff_t * ppos) {
    char buf[BUF_SIZE];
    int len = 0;

    printk(KERN_INFO "proc file read.....\n");
    if (*ppos > 0 || count < BUF_SIZE) {
        return 0;
    }

    struct task_struct *taskStruct = get_pid_task(find_get_pid(pid), PIDTYPE_PID);

    if (taskStruct == NULL) {
        len += sprintf(buf, "task_struct for pid %d is NULL!!!\n", pid);
        if (copy_to_user(ubuf, buf, len)) {
            return -EFAULT;
        }
        *ppos = len;
        return len;
    }

    switch(struct_id)   {
        default:
        case 0:
            len = write_pt_regs_struct(ubuf, taskStruct);
        break;
        case 1:
            len = write_task_struct(ubuf, taskStruct);
        break;
    }

    *ppos = len;
    return len;
}

/*
** This function will be called when we write the procfs file
*/
static ssize_t write_proc(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos) {
    int num_of_read_digits, c, a, b;
    char buf[BUF_SIZE];

    printk(KERN_INFO "proc file wrote.....\n");

    if (*ppos > 0 || count > BUF_SIZE) {
        return -EFAULT;
    }

    if(copy_from_user(buf, ubuf, count)) {
        return -EFAULT;
    }

    num_of_read_digits = sscanf(buf, "%d %d", &a, &b);

    if (num_of_read_digits != 2){
        return -EFAULT;
    }

    struct_id = a;
    pid = b;
    c = strlen(buf);
    *ppos = c;
    return c;
}

/*
** Module Init function
*/
static int __init lab_driver_init(void) {
    parent = proc_mkdir("lab", NULL);
    if (parent == NULL) {
        pr_info("Error creating proc entry");
        return -1;
    }

    mutex_init(&lab_mutex);

    proc_create("struct_info", 0666, parent, &proc_fops);
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;
}

/*
** Module exit function
*/
static void __exit lab_driver_exit(void) {
    mutex_destroy(&lab_mutex);
    proc_remove(parent);

    pr_info("Device Driver Remove...Done!!!\n");
}

module_init(lab_driver_init);
module_exit(lab_driver_exit);

MODULE_LICENSE("GPL");
