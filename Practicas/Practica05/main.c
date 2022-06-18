/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


static volatile float cont;
static volatile float cuentaTimer;
static volatile float cuentaInt;
static volatile float periodo;
static volatile float frecuencia;
char msg[30];
static volatile uint8_t i;

const eUSCI_UART_ConfigV1 uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                     // BRDIV = 78
        2,                                       // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
        EUSCI_A_UART_8_BIT_LEN                  // 8 bit data length
};

void LED_init(void){
    P1->SEL0 &= ~0X01;
    P1->SEL1 &= ~0X01;
    P1->DIR |= 0x01;
}

int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    LED_init();
    P1 -> OUT &= ~0x01;

    cont=0;
    cuentaTimer=0;
    periodo=0;
    frecuencia=0;

    /* P1.2 and P1.3 en modo UART  */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    /* DCO to 12MHz */
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
    /* Configuración UART con base a la estructura de arriba */
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    /* Habilitamos UART */
    UART_enableModule(EUSCI_A0_BASE);


    //Timer A3 modo continuo, fuente de reloj SMCLK = 12 MHz
    //Limpieza del TAR
    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC_0; // Modes which we use
    TIMER_A0->CCR[0] = 65534;                            // tics == 50, Compare and control Frequency
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;                  // Compare and control enable
    TIMER_A0->CTL |= TIMER_A_CTL_MC_2;
    // enable NVIC for Port 4 and TimerA
    NVIC->ISER[0] |= 1 << ((TA0_0_IRQn)&31);

    //Interrupción P5.0
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_interruptEdgeSelect(GPIO_PORT_P5, GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION);
    Interrupt_enableInterrupt(INT_PORT5);

    /* Interrupciones */
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);


    Interrupt_enableMaster();

    while(1)
    {
        
    }
}

/*Función de servicio de interrupción Timer A3*/
void TA0_0_IRQHandler (void){
       TIMER_A0 -> CCTL[0] &= ~(TIMER_A_CCTLN_CCIFG); //Limpiamos la bandera
       cont++;     //Cuenta de desbordamientos
}

void PORT5_IRQHandler (void){
    uint8_t status = P5->IFG;
    if(status & BIT2){
        GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);

        if(cuentaInt==30){
            cuentaTimer = Timer_A_getCounterValue(TIMER_A0_BASE);

            periodo = (0.000000083)*((cuentaTimer+(cont*65534))/cuentaInt);
            frecuencia = 1/periodo;

            sprintf(msg,"%0.7f\r\n",frecuencia);

            for(i=0;i<=15;i++){         //se comienzan a transmitir los datos almacenados
                UART_transmitData(EUSCI_A0_BASE, msg[i]);
            }

            Timer_A_clearTimer(TIMER_A0_BASE);

            cuentaInt=0;
            cont=0;
            cuentaTimer=0;
            periodo=0;
            frecuencia=0;
        }else{
            cuentaInt++;
        }
    }
}
