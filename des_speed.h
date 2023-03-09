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
    void calc_new_duty_val();
    void calc_duty_from_AD();
    char getDesRichtung();
    int get_des_duty_val();
    char get_des_control_mode();
    int des_volt_is_pos();
    void set_des_duty_val_d(int des_duty_val);
    void set_control_mode(char des_mode);
    void set_volt_pos_d(char desVoltDir);

#ifdef	__cplusplus
}
#endif

#endif	/* DES_SPEED_H */

