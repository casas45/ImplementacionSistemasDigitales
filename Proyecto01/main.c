/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "i2c_lcd.h"

#define SLAVE_ADDRESS   0x27
#define ENTER_KEY       13
#define BACK_KEY        8

/*Variables globales*/
static volatile float cont;
static volatile float cuentaTimer;
static volatile float cuentaInt;
static volatile float periodo;
static volatile float frecuencia;
static volatile float error;
char msg[30];
static volatile uint8_t i,j,k;
bool LecturaADC = false;
bool DatosUART = false;
bool ImpFrecuencia = false;

static volatile uint16_t ValorADC;
static volatile float VoltajeNormalizado;
static volatile float buffer[100];

char msg2[16];

uint8_t t;
uint8_t l=99;

const eUSCI_UART_ConfigV1 uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        156,                                     // BRDIV = 156
        4,                                       // UCxBRF = 4
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
        EUSCI_A_UART_8_BIT_LEN                  // 8 bit data length
};



int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    /*Se inicializan variables*/
    cont=0;
    cuentaTimer=0;
    periodo=0;
    frecuencia=0;

    /*Se iniciliza el display y se limpia*/
    LCD_init(SLAVE_ADDRESS);
    LCD_clear();
    LCD_home();

    /* P1.2 and P1.3 en modo UART  */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    /* DCO to 24MHz */
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);
    /* Configuración UART con base a la estructura de arriba */
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    /* Habilitamos UART */
    UART_enableModule(EUSCI_A0_BASE);

    //-----Inicio configuración del ADC

    /*Habilitamos FPU*/
    FPU_enableModule();
    FPU_enableLazyStacking();

    /*Configuración ADC*/
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_32, ADC_DIVIDER_8, ADC_NOROUTE);

    /*GPIO Configuracion*/
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    /*Configurar ADC Memoria*/
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    /*Configuración timer en modo manual*/
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    //-----Fin configuración del ADC


    //Timer A3 modo continuo, fuente de reloj SMCLK = 24 MHz
    //Limpieza del TAR
    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC_0; //Se selecciona la fuente de reloj SMCLK para el timer
    TIMER_A0->CCR[0] = 65534;                       //
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;         // se habilita el registro captura comparación
    TIMER_A0->CTL |= TIMER_A_CTL_MC_2;              //se configura el timer en modo ascendente
    // se habilita la interrupción del timer A0
    NVIC->ISER[0] |= 1 << ((TA0_0_IRQn)&31);

    //Configuración Interrupción P5.2
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_interruptEdgeSelect(GPIO_PORT_P5, GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION);
    Interrupt_enableInterrupt(INT_PORT5);

    //Configuración Interrupción P1.1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);
    Interrupt_enableInterrupt(INT_PORT1);

    /*Habilitar Interrupciones */
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);

    Interrupt_enableInterrupt(INT_ADC14);           //Se habilita la interrupción del ADC
    ADC14_enableInterrupt(ADC_INT0);

    Interrupt_enableMaster();



    while(1)
    {

        if(DatosUART == true){
            for(j=0; j<=l; j++){
                sprintf(msg,"%0.1f,\r\n",buffer[j]);
                for(k=0;k<=5;k++){         //se comienzan a transmitir los datos almacenados
                    UART_transmitData(EUSCI_A0_BASE, msg[k]);
                }
            }
        }

        if(ImpFrecuencia == true){
            sprintf(msg2,"%0.0fHz",frecuencia);
            LCD_home();     //se situa el cursor en la posición 0,0
            LCD_writeString((uint8_t*)"Frecuencia:",(uint8_t)11);       //se imprime frecuencia en el renglón 0
            LCD_setCursorPosition((uint8_t) 1, (uint8_t) 0); //se situa el cursor en la posición 1,0
            LCD_writeString((uint8_t*)msg2,(uint8_t)t);     //se imprime el valor de la frecuencia en el renglón 1
            ImpFrecuencia = false;
            Interrupt_enableInterrupt(INT_PORT5);   //se vuelve a habilitar interrupción
        }

    }
}

/*Función de servicio de interrupción Timer A3*/
void TA0_0_IRQHandler (void){
       TIMER_A0 -> CCTL[0] &= ~(TIMER_A_CCTLN_CCIFG); //Limpiamos la bandera
       cont++;     //Cuenta de desbordamientos
}

/*Función de interrupción P5.2*/
void PORT5_IRQHandler (void){
    uint8_t status = P5->IFG;
    if(status & BIT2){
        GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);   //limpiamos bandera de la interrupción

        if(cuentaInt==10){  //si se han contado 10 flancos de subida

            cuentaTimer = Timer_A_getCounterValue(TIMER_A0_BASE);   //se obtiene la cuenta actual del Timer

            periodo = (0.0000000416)*((cuentaTimer+(cont*65534))/cuentaInt);    //y se realiza el cálculo del periodo
            frecuencia = 1/periodo; //y luego el de la frecuencia

            error=((0.085196)*frecuencia)-(0.466465);   //calculo del error

            frecuencia=frecuencia+error;    //se suma el error a la frecuencia

            Timer_A_clearTimer(TIMER_A0_BASE);  //se limpia la cuenta del Timer


            /*Reinicio de las variables usadas para el cálculo*/
            cuentaInt=0;
            cont=0;
            cuentaTimer=0;
            periodo=0;
        }else{              //si aun no se cuentan los 10 flancos de subida
            cuentaInt++;    //se aumenta el contador
        }
    }
}

void PORT1_IRQHandler (void){
    uint8_t status2 = P1->IFG;
    if(status2 & BIT1){
        Interrupt_disableInterrupt(INT_PORT5);          //se deshabilita esta interrupción para evitar errores al imprimir frecuencia
        i=0;                                            //Se inicializa o reinicia el contador
        GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);   //limpiamos bandera de la interrupción
        LecturaADC=true;                                //se habilita la bandera para leer los datos del ADC
        Interrupt_enableInterrupt(INT_ADC14);           //Se habilita la interrupción del ADC
        ADC14_enableInterrupt(ADC_INT0);

        ImpFrecuencia = true;               //se habilita bandera para imprimir frecuencia
        LCD_clear();                        //se limpia el display

        if(frecuencia>9999){
            t=7;
        }else if(frecuencia>999 && frecuencia<=9999){
            t=6;
        }else if(frecuencia>99 && frecuencia<=999){
            t=5;
        }else if(frecuencia>9 && frecuencia<=99){
            t=4;
        }else if(frecuencia<=9){
            t=3;
        }
    }

}

void ADC14_IRQHandler(void){
    uint16_t status3 = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(ADC_INT0);
    if((ADC_INT0 & status3)){
        // se guarda el valor del ADC
        ValorADC = ADC14_getResult(ADC_MEM0);
        //Se normaliza el voltaje
        VoltajeNormalizado = (ValorADC*3.3)/16383;

        //se verifica si la bandera indica que ya debe guardar nuevos valores
        if(LecturaADC==true){
            buffer[i]=VoltajeNormalizado;   //se comienzan a guardar los valores
            i++;                            //en el buffer y aumenta el contador de la posisición
            if(i>=100){                     // si ya se guardaron 100 datos
                l=i-1;
                Interrupt_disableInterrupt(INT_ADC14);  //se deshabilita la int. del ADC
                ADC14_disableInterrupt(ADC_INT0);
                LecturaADC=false;           //Y se deshabilita la bandera
                DatosUART = true;
            }
        }
    }
}
