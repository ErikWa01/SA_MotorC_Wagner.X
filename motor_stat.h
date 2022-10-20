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

void HallSensor_Init();
int read_HallSensors();


#ifdef	__cplusplus
}
#endif

#endif	/* MOTOR_STAT_H */

