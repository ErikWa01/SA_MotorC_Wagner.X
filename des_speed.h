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
    void set_des_dir(char desRtg);
    void set_des_speed(float des_speed);
    void set_des_control_mode(char des_mode);

#ifdef	__cplusplus
}
#endif

#endif	/* DES_SPEED_H */

