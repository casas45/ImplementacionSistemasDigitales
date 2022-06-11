/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

//Función configurar pines del LED RGB
void RGBinit (void){
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;
    P2->DIR |= 0x07;    //pines 2.0, 2.1, 2.2 como salidas
    P2->OUT &= 0x01;    //se inicializa solo el led rojo encendido
}
//Función para configurar el boton (S1) en P1.1
void PBinit (void){
    P1->SEL0 &= ~0x02;
    P1->SEL1 &= ~0x02;
    P1->DIR &= ~0x02;           //GPIO P1.1 como entrada
    P1->REN |= 0x02;            //Habilitamos resistor pull en P1.1
    P1->OUT |= 0x02;           //Habilitamos pull-UP en P1.1
}


int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    RGBinit();
    PBinit();

    char Estado='A';

    while(1)
    {
        if(!(P1IN & BIT1)){         //se detecta si se presiona el botón
            __delay_cycles(15000);  //retardo para evitar rebote mecánico
            while(!(P1IN & BIT1));  //while para cambiar de estado hasta que se suelte el boton

            switch(Estado){
            case 'A':
                P2->OUT = 0x01;    //Estado A = Color Rojo
                Estado='B';        //Se cambia de estado A->B
            break;
            case 'B':
                P2->OUT = 0x02;    //Estado B = Color Verde
                Estado='C';        //B->C
            break;
            case 'C':
                P2->OUT = 0x04;    //Estado C = Color Azul
                Estado='D';        //C->D
            break;
            case 'D':
                P2->OUT = 0x03;    //Estado D = Color Amarillo
                Estado='A';        //C->A
            break;
            }

        }
        

    }
}




