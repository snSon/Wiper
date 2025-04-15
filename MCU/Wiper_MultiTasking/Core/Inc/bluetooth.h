#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

void Bluetooth_Init(void);
void Bluetooth_RxCallback(void);
void Parse_Command(const char* cmd);
uint16_t Bluetooth_GetSpeed(void);

#endif
