#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_write(int file,const void *buf,size_t n);
int sys_read(int fd, void *buf, size_t buflen, int * retval);
int sys_fork(struct trapframe *tf, int *retval);
int sys_wait(int tid, int *status, int option, int * retval);
int sys_exit(int ret, int *retval);
int sys_getpid(int *retval);
int sys_execv(char* progname, char** arg);
int sys_sbrk(intptr_t amount, int * retval);

#endif /* _SYSCALL_H_ */
