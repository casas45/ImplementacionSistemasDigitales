/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/*Variables globales*/
static volatile uint16_t ValorADC;
static volatile float AnchoPulso01;
static volatile float AnchoPulso02;
static volatile float AnchoPulso03;

static volatile uint16_t servo1;
static volatile uint16_t servo2;
static volatile uint16_t servo3;

int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    //-----Inicio configuración del ADC
    /*Configuracion DCO 3 MHz*/
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_3);

    /*Habilitamos FPU*/
    FPU_enableModule();
    FPU_enableLazyStacking();

    /*Configuración ADC*/
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_64, ADC_DIVIDER_1, ADC_NOROUTE);

    /*GPIO Configuracion*/
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN3, GPIO_TERTIARY_MODULE_FUNCTION);

    /*Configurar ADC Memoria*/
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM2, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A1, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM2, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A2, ADC_NONDIFFERENTIAL_INPUTS);

    /*Configuración timer en modo automatico*/
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();
    //-----Fin configuración del ADC


    /*Inicio Configuración Timer A0*/
    P2 -> DIR |= BIT4 + BIT5 + BIT6; //se habilitan salidas (Timer A0.1, A0.2, A0.3)
    P2 -> SEL0 |= BIT4 + BIT5 + BIT6;

    //Habilitamos acceso a esritura al registro CS
    CS -> KEY = CS_KEY_VAL;
    //Deshabilitamos acceso a esritura al registro CS
    CS -> KEY = 0;


    //Limpieza del TAR
    TIMER_A0 -> CTL = TIMER_A_CTL_MC__UP | TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_CLR;


    //Cargar base de tiempo en CCR0
    TIMER_A0 -> CCR[0] = 50000; //7.3
    TIMER_A0 -> CCR[1] = 2500; //P2.4
    TIMER_A0 -> CCR[2] = 2500; //P2.5
    TIMER_A0 -> CCR[3] = 2500; //P2.6

    //TIMER_A0 -> CCTL[0] = TIMER_A_CCTLN_OUTMOD_4;
    TIMER_A0 -> CCTL[1] = TIMER_A_CCTLN_OUTMOD_7;
    TIMER_A0 -> CCTL[2] = TIMER_A_CCTLN_OUTMOD_7;
    TIMER_A0 -> CCTL[3] = TIMER_A_CCTLN_OUTMOD_7;

    /*Fin Configuración Timer A0*/


    /*Habilitar interrupciones*/

    ADC14_enableInterrupt(ADC_INT2);
    Interrupt_enableInterrupt(INT_ADC14);

    Interrupt_enableMaster();

    while(1)
    {
        
    }
}

void ADC14_IRQHandler(void)
{
    if (ADC14->IFGR0 & ADC14_IFGR0_IFG2) // check for MEM2
    {
        // se guarda el valor de cada canal del ADC
        uint16_t ch0 = ADC14->MEM[0];
        uint16_t ch1 = ADC14->MEM[1];
        uint16_t ch2 = ADC14->MEM[2];

        AnchoPulso01 = ((2500/16383)*ch0)+2500;
        AnchoPulso02 = ((2500/16383)*ch1)+2500;
        AnchoPulso03 = ((2500/16383)*ch2)+2500;

        servo1=AnchoPulso01;    //grado de libertad 1
        servo2=AnchoPulso02;    //grado de libertad 2
        servo3=AnchoPulso03;    //grado de libertad 3

        TIMER_A0 -> CCR[1] = servo1;
        TIMER_A0 -> CCR[2] = servo2;
        TIMER_A0 -> CCR[3] = servo3;
    }
}
