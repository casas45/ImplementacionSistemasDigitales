/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

static volatile uint16_t ValorADC;
static volatile uint16_t AnchoPulso;


int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();
    /*Inicializar variables*/
    ValorADC=0;
    AnchoPulso=10;


    //Configuración de salidas p10.4
    P10 -> DIR |= BIT4 | BIT5;
    //Reset pines 10.4 y 10.5
    P10 -> OUT &= ~(BIT4 | BIT5);
    //Seleccion de función periferica primaria
    P10 -> SEL0 |= BIT4 | BIT5;

    //Timer A3 modo ascendente, fuente de reloj SMCLK = 48 MHz
    //Limpieza del TAR
    TIMER_A3 -> CTL = TIMER_A_CTL_MC__UP | TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_CLR;

    //Cargar base de tiempo en CCR0
    TIMER_A3 -> CCR[0] = 100;   //SMCLK = 48MHz
                                //48MHz/(100*2) = 240kHz = 4.166us <- Periodo de la señal PWM

    //Cargar base de tiempo en CCR1
    TIMER_A3 -> CCR[1] = 70;    //Señal PWM


    //TIMER_A3 -> CCTL[0] = TIMER_A_CCTLN_OUTMOD_4;   //P10.4
    TIMER_A3 -> CCTL[1] = TIMER_A_CCTLN_OUTMOD_7;   //P10.5





    /*Configura Flash estado de espera*/
    FlashCtl_setWaitState(FLASH_BANK0,1);
    FlashCtl_setWaitState(FLASH_BANK1,1);

    /*Configuracion DCO 48 MHz*/
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);

    /*Habilitamos FPU*/
    FPU_enableModule();
    FPU_enableLazyStacking();

    /*Configuración ADC*/
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_DIVIDER_3, ADC_DIVIDER_4, ADC_NOROUTE);

    /*GPIO Configuracion*/
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    /*Configurar ADC Memoria*/
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    /*Configuración timer en modo manual*/
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    /*Interrupciones*/
    ADC14_enableInterrupt(ADC_INT0);
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();


    while(1)
    {
        //PCM_gotoLPM0();
    }
}

/*Servicio de interrupción
 * Cuanndo el registro ADC_MEM0 esta listo para leer*/
void ADC14_IRQHandler(void){
    uint16_t status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(ADC_INT0);
    if(ADC_INT0 & status){
        ValorADC = ADC14_getResult(ADC_MEM0);
        AnchoPulso = (ValorADC*100)/16383;
        TIMER_A3 -> CCR[1] = AnchoPulso;
    }
}

