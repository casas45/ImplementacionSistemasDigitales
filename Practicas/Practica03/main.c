/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define cero 0b0000001
#define uno 0b1001111
#define dos 0b0010010
#define tres 0b0000110
#define cuatro 0b1001100
#define cinco 0b0100100
#define seis 0b0100000
#define siete 0b0001111
#define ocho 0b0000000
#define nueve 0b0000100


static volatile float cont;
static volatile float cuentaTimer;
static volatile float cuentaInt;
static volatile float periodo;
static volatile float frecuencia;
char msg[30];
static volatile uint8_t i;
bool flg_mostrar=false;

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

void Display_init(void){
    /*Inicializar display decenas P5*/
    P5->SEL0 &= ~0X7F;
    P5->SEL1 &= ~0X7F;
    P5->DIR |= 0x7F;

    /*Inicializar display unidades P4*/
    P4->SEL0 &= ~0X7F;
    P4->SEL1 &= ~0X7F;
    P4->DIR |= 0x7F;
}

void displays7seg(int decenas, int unidades){
    switch(decenas){
        case 9:
            P5 -> OUT = nueve;
        break;

        case 8:
            P5 -> OUT = ocho;
        break;

        case 7:
            P5 -> OUT = siete;
        break;

        case 6:
            P5 -> OUT = seis;
        break;

        case 5:
            P5 -> OUT = cinco;
        break;

        case 4:
            P5 -> OUT = cuatro;
        break;

        case 3:
            P5 -> OUT = tres;
        break;

        case 2:
            P5 -> OUT = dos;
        break;

        case 1:
            P5 -> OUT = uno;
        break;

        case 0:
            P5 -> OUT = cero;
        break;
    }

    switch(unidades){
        case 9:
            P4 -> OUT = nueve;
        break;

        case 8:
            P4 -> OUT = ocho;
        break;

        case 7:
            P4 -> OUT = siete;
        break;

        case 6:
            P4 -> OUT = seis;
        break;

        case 5:
            P4 -> OUT = cinco;
        break;

        case 4:
            P4 -> OUT = cuatro;
        break;

        case 3:
            P4 -> OUT = tres;
        break;

        case 2:
            P4 -> OUT = dos;
        break;

        case 1:
            P4 -> OUT = uno;
        break;

        case 0:
            P4 -> OUT = cero;
        break;
    }
}




int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    cont=0;
    cuentaTimer=0;
    periodo=0;
    frecuencia=0;

    Display_init();


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

    int decenas=0, unidades=0;
    int frecuencia2;

    while(1)
    {
        if(flg_mostrar==true){      //se verifica si ya se habilitó la bandera para mostrar
                                    //el resultado

            frecuencia2=(int)frecuencia;    //cast a entero

            /*Se obtienen los digitos de la frecuencia*/
            decenas=frecuencia2/10;
            unidades=frecuencia2%10;

            /*Función para mostrar el resultado en los display de 7 segmentos*/
            displays7seg(decenas, unidades);

            flg_mostrar=false;  //se reinicia la bandera
        }
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
        GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);   //limpiamos bandera de la interrupción

        if(cuentaInt==10){//si se han contado 10 flancos de subida
            //se obtiene la cuenta actual del Timer
            cuentaTimer = Timer_A_getCounterValue(TIMER_A0_BASE);
            //y se realiza el cálculo del periodo
            periodo = (0.000000083)*((cuentaTimer+(cont*65534))/cuentaInt);
            frecuencia = 1/periodo; //y luego el de la frecuencia

            Timer_A_clearTimer(TIMER_A0_BASE);  //se limpia la cuenta del Timer

            flg_mostrar=true;  //habilitar bandera para mostrar digitos

            /*Reinicio de las variables usadas para el cálculo*/
            cuentaInt=0;
            cont=0;
            cuentaTimer=0;
            periodo=0;
            frecuencia=0;
        }else{
            //si aun no se cuentan los 10 flancos de subida
                        cuentaInt++;    //se aumenta el contador
        }
    }
}
