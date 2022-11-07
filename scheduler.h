/* 
 * File:   scheduler.h
 * Author: erikw
 *
 * Created on 7. November 2022, 17:08
 */

#ifndef SCHEDULER_H
#define	SCHEDULER_H

#ifdef	__cplusplus
extern "C" {
#endif

void scheduler_init();
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt (void);


#ifdef	__cplusplus
}
#endif

#endif	/* SCHEDULER_H */

