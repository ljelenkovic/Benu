/*! Signals */
#pragma once

#include <types/signal.h>

/*! interface to threads (via syscall) */

/* int sys__sigqueue(pid_t pid, int signo, sigval_t sigval); */
int sys__sigqueue(void *p);

/*int sys__pthread_sigmask(int how, sigset_t *set, sigset_t *oset);*/
int sys__pthread_sigmask(void *p);

/*int sys__sigaction(int sig, sigaction_t *act, sigaction_t *oact);*/
int sys__sigaction(void *p);

/*int sys__sigwaitinfo(sigset_t *set, siginfo_t *info);*/
int sys__sigwaitinfo(void *p);
