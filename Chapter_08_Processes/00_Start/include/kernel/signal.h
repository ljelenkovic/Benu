/*! Signals */
#pragma once

#include <types/signal.h>

/*! interface to threads */

int sys__sigqueue(pid_t pid, int signo, sigval_t sigval);
int sys__pthread_sigmask(int how, sigset_t *set, sigset_t *oset);
int sys__sigaction(int sig, sigaction_t *act, sigaction_t *oact);
int sys__sigwaitinfo(sigset_t *set, siginfo_t *info);