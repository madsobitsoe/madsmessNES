#ifndef LOGGER_H
#define LOGGER_H

#include "cpu.h"

// Create a logger attached to a file
int logger_init_logger(char *filename);
// Write a line to the log.
void logger_log(nes_state *state);
// print a line to STDOUT
void print_log(nes_state *state);
// Stop the logger and close the file
void logger_stop_logger();

#endif
