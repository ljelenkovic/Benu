/*! Signals */
#pragma once

#include <types/signal.h>

int sigaction(int sig, sigaction_t *act, sigaction_t *oact);
int sigwaitinfo(sigset_t *set, siginfo_t *info);
int sigqueue(pid_t pid, int signo, sigval_t sigval);
int pthread_sigmask(int how, sigset_t *set, sigset_t *oset);
