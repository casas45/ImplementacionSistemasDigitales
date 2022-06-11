/*Contador de 0 a 1000
 *el LED rojo se enciende cada vez que el
 *el contador tenga un número primo*/

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

bool NumPrimo (int num);

/*Variables globales*/
int i,j;


int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    //Configuración P1.0 (LED) como salida
    P1->SEL0 &= ~0x01;
    P1->SEL1 &= ~0x01;
    P1->DIR |= 0x01;

    P1->OUT &= ~0x01;   //se inicializa el LED rojo apagado

    /*Variable del contador*/
    int cont=0;

    while(1)
    {
        

        if(NumPrimo(cont)){     //si el contador es primo
            P1->OUT |= 0x01;    //se enciende el LED rojo
        }else{                  //si no
            P1->OUT &= ~0x01;   //se apaga o permanece apagado el LED rojo
        }

        __delay_cycles(3000000);    //retardo crudo, aprox 1 seg.

        if(cont>=1000){         //si el contador llega a 100o
            cont=0;             //se reinicia
        }else{
            cont++;             //si no, aumenta
        }





    }
}

bool NumPrimo (int num){
    if(num==0 || num==1 || num==4){ //Los números 0 y 1 no son considerados primos
        return false;               //y el 4 con el algoritmo que se usa lo detecta como primo y no lo es
                                    //por lo tanto se agrega aqui como una excepción
    }else{
        for(j=2; j<num/2; j++){     //si se puede dividir por cualquiera de estos numeros no es primo
            if((num % j) == 0){
                return false;
            }
        }
        return true;                //si no se puede dividir es primo
    }
}
