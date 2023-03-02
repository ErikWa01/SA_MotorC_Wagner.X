/* 
 * File:   des_speed.h
 * Author: erikw
 *
 * Created on 17. Oktober 2022, 13:29
 */

#ifndef DES_SPEED_H
#define	DES_SPEED_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    /* Funktionsprototypen */
    char getDesRichtung();
    float getDesSpeed();
    char get_des_control_mode();
    int des_volt_is_pos();
    void set_des_dir(char desRtg);
    void set_des_speed(float des_speed);
    void set_des_control_mode(char des_mode);
    void set_volt_pos(char desVoltDir);

#ifdef	__cplusplus
}
#endif

#endif	/* DES_SPEED_H */

