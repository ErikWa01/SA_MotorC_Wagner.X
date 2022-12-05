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
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt (void);
void calc_motor_position();
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt (void);
int get_drehzahl();
int get_drehwinkel();
int read_HallSensors();
int drehwinkel_is_valid();


#ifdef	__cplusplus
}
#endif

#endif	/* MOTOR_STAT_H */

