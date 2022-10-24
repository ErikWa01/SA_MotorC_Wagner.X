/* 
 * File:   com_interface.h
 * Author: erikw
 *
 * Created on 20. Oktober 2022, 14:16
 */

#ifndef COM_INTERFACE_H
#define	COM_INTERFACE_H

#ifdef	__cplusplus
extern "C" {
#endif

    void com_interface_init();
    void __attribute__((interrupt, no_auto_psv)) _CNInterrupt (void);
    void handle_msg_rx(char *msg);


#ifdef	__cplusplus
}
#endif

#endif	/* COM_INTERFACE_H */

