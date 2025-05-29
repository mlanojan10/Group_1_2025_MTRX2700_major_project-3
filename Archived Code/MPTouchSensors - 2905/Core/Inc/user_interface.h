#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdint.h>

void UI_PrintWelcomeMessage(void);
void UI_PrintFailureMessage(void);
void UI_PrintSuccessMessage(void);
void UI_PrintPrompt(uint8_t index);

#endif
