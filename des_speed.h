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
    void handleCommand(char command);
    char getDesRichtung();
    float getDesSpeed();

#ifdef	__cplusplus
}
#endif

#endif	/* DES_SPEED_H */

