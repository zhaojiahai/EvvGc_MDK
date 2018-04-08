/*
 *  utils.h
 *
 *  Created on: Jun 25, 2013
 *      Author: Denis caat
 */

#ifndef UTILS_H_
#define UTILS_H_

extern void LEDon(void);
extern void LEDoff(void);
extern void LEDtoggle(void);

extern void DEBUG_LEDon(void);
extern void DEBUG_LEDoff(void);
extern void DEBUG_LEDtoggle(void);

extern void Blink(void);

extern void Delay_ms(unsigned int ms);
extern void Delay_us(unsigned int us);

extern float Rad2Deg(float x);
extern float Deg2Rad(float x);
extern float Round(float x);
#endif /* UTILS_H_ */
