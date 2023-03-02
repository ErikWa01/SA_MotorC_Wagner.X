/* 
 * File:   adc_module.h
 * Author: erikw
 *
 * Created on 14. November 2022, 16:00
 */

#ifndef ADC_MODULE_H
#define	ADC_MODULE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    /* Definition von Datentypen */
    typedef struct Filter   // Filterinstanzen
    {
        long w;
        int k;
    } Filter;
    
    /* Funktionsdeklaration */
    void adc_init();
    void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void);
    //double calc_I_from_ADval();
    int get_I_motor_ADval();
    int get_control_2_ADval();
    int get_control_4_ADval();
    int tiefpass_adc(Filter *f, int x);
    void filterInit(Filter *f, int k, int x);


#ifdef	__cplusplus
}
#endif

#endif	/* ADC_MODULE_H */

