/* 
 * File:   motor_stat.h
 * Author: erikw
 *
 * Created on 17. Oktober 2022, 13:28
 */

#ifndef MOTOR_STAT_H
#define	MOTOR_STAT_H

#ifdef	__cplusplus
extern "C" {
#endif

void motor_stat_init();
int read_HallSensors();
void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void);
double calc_I_from_ADval();


#ifdef	__cplusplus
}
#endif

#endif	/* MOTOR_STAT_H */

