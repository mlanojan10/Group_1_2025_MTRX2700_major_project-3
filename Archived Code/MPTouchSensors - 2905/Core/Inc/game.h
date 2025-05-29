#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>
#include "serial.h"

#define MAX_PATTERN_LENGTH 6

void Game_ShowInstructions(SerialPort *serial);
void Game_ShowResult(bool success, SerialPort *serial);
bool Game_CheckPattern(uint8_t *correct_pattern, uint8_t *user_input, uint8_t length);

#endif // GAME_H
