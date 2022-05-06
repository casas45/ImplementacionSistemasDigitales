#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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

/*Variables*/
static uint8_t Dato[16]={0x41, 0x42, 0x43, 0x44,
                         0x45, 0x46, 0x47, 0x48,
                         0x49, 0x4A, 0x4B, 0x4C,
                         0x4D, 0x4E, 0x4F, 0x50};

static uint8_t CipherKey [32] = {0x00, 0x01, 0x02, 0x03,
                                 0x04, 0x05, 0x06, 0x07,
                                 0x08, 0x09, 0x0A, 0x0B,
                                 0x0C, 0x0D, 0x0E, 0x0F,
                                 0x00, 0x01, 0x02, 0x03,
                                 0x04, 0x05, 0x06, 0x07,
                                 0x08, 0x09, 0x0A, 0x0B,
                                 0x0C, 0x0D, 0x0E, 0x0F};
static uint8_t DatoAESencrip [16];


int main(void)
{

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1); //se habilita el boton P1.1
    int i=0;


    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();
    /*Cargamos la llave al modulo AES con longitud 256*/
    AES256_setCipherKey(AES256_BASE, CipherKey, AES256_KEYLENGTH_256BIT);

    /*Encriptacion de Dato y almacenado en DatoAESencrip*/
    AES256_encryptData(AES256_BASE, Dato, DatoAESencrip);




    /*Configruación UART*/
    /* P1.2 and P1.3 en modo UART  */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Configuración UART con base a la estructura de arriba */
    UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Habilitamos UART */
    UART_enableModule(EUSCI_A0_BASE);

    /* Interrupciones */
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);

    while(1)
    {
        if(!(P1IN & BIT1)){         //Para comenzar a mandar el dato encriptado
            /*delay 15ms*/
            __delay_cycles(180000);
            while(!(P1IN & BIT1));  //se tiene que presionar y soltar el boton P1.1
            for(i=0;i<=15;i++){
                UART_transmitData(EUSCI_A0_BASE, DatoAESencrip[i]);
            }
        }
    }
}
