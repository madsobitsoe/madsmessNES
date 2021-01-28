#ifndef LOGGER_H
#define LOGGER_H

#include "cpu.h"
// struct for logging in the nintendulator format
// For now, without the PPU cycles
/* typedef struct log_entry { */
/*   uint16_t PC; */
/*   uint8_t opcode; */
/*   int16_t operand1; // -1 indicates no operand */
/*   int16_t operand2; // -1 indicates no operand */
/*   uint8_t ACC; */
/*   uint8_t X; */
/*   uint8_t Y; */
/*   uint8_t P; */
/*   uint8_t SP; */
/*   uint32_t CYC; */
/* } log_entry; */

// Create a logger attached to a file
int logger_init_logger(char *filename);
// Write a line to the log. If not newline-terminated, one will be added.
void logger_log(nes_state *state);
/* // Write a log to disk. Returns 0 on success, errno on error. */
/* int  logger_write_log(char *filename); */
// Stop the logger and close the file
void logger_stop_logger();
//log_entry logger_state_to_log_entry(nes_state *state);
#endif
