/*! Memory management */
#pragma once

/*! Kernel memory layout ---------------------------------------------------- */
void k_memory_init ();
void k_memory_info ();

void k_memory_fault (); /* memory fault handler */
