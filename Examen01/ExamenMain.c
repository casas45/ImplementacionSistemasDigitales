/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

uint8_t cont = 1;
uint8_t i=0, j=0, k=0;
uint8_t semaforo = 1;
uint8_t cambio=0;
bool modo=true;

void LED_init(void){
    P2->SEL0 &= ~0xB0;  //Configuración GPIOs 2.4, 2.5 Y 2.7
    P2->SEL1 &= ~0xB0;
    P2->DIR |= 0xB0;    //2.5, 2.4 Y 2.7 como salidas

    P3->SEL0 &= ~0xE0;  //Configuración GPIOs 3.5, 3.6 Y 3.7
    P3->SEL1 &= ~0xE0;
    P3->DIR |= 0xE0;    //3.5, 3.6 Y 3.7 como salidas

    P5->SEL0 &= ~0x07;  //Configuración GPIOs 5.0, 5.1, 5.2
    P5->SEL1 &= ~0x07;
    P5->DIR |= 0x07;    //5.0, 5.1, 5.2 como salidas

    P10->SEL0 &= ~0x1C;  //Configuración GPIOs 10.2, 10.3, 10.4
    P10->SEL1 &= ~0x1C;
    P10->DIR |= 0x1C;    //10.2, 10.3, 10.4 como salidas
}



void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    LED_init();             //función para inicialiar los LED's
    P5->OUT = 0x01;    //se inicializa los LED's apagado

    //Configurar botones
    P1->SEL0 &= ~0x12;          //Configuración de P1.1 y P1.4 GPIO
    P1->SEL1 &= ~0x12;

    P1->DIR &= ~0x12;           //GPIO P1.1 y P1.4 como entrada
    P1->REN |= 0x12;            //Habilitamos resistor pull en P1.1 y P1.4
    P1->OUT |= 0x12;           //Habilitamos pull-UP en P1.1 y P1.4

    /*Inicializar interrupciones en P2.1*/
    P1->IE |= (0x02);
    P1->IES |= 0x02;
    P1->IFG &= ~(0x02);

    /*Inicializar Semaforos en rojo*/
    P2 -> OUT = BIT4; //ROJO S1
    P3 -> OUT = BIT5; //ROJO S2
    P5 -> OUT = BIT0; //ROJO S3
    P10 -> OUT = BIT2; //ROJO S4

    //Habilitamos acceso a esritura al registro CS
    CS -> KEY = CS_KEY_VAL;

    //Deshabilitamos acceso a esritura al registro CS
    CS -> KEY = 0;

    //Timer A3 modo ascendente, fuente de reloj SMCLK = 3 MHz
    //Limpieza del TAR
    TIMER_A3 -> CTL = TIMER_A_CTL_MC__UP | TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_CLR | TIMER_A_CTL_IE;


    //Cargar base de tiempo en CCR0
    TIMER_A3 -> CCR[0] = 60000; //P10.4  //3MHz/60000 = 50Hz = 2 mS


    TIMER_A3 -> CCTL[0] = TIMER_A_CCTLN_CCIE;


    //Habilitamos interrupcion global
    __enable_irq();

    //
    NVIC -> ISER [0] |= 1 << ((TA3_0_IRQn) & 31);

    // SE HABILITA NVIC para P5
    NVIC->ISER[1] = 1 << ((PORT1_IRQn)&31);

    while(1){

    }
}

void TA3_0_IRQHandler (void){
    TIMER_A3 -> CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; //Limpiamos la bandera


    if(modo==true){ //modo normal
        /*Parpadeos*/
        if(semaforo==1){ //semaforo 1 -> parpadeos
            //al llegar la cuenta a 60*20ms = 1.2s en los colores rojo y verde, el color va a parpadear cada 10*20ms=200ms
            if(i>=60 && (i%10)==0){
                if(cont==1){
                    P2 -> OUT ^= BIT4; //ROJO
                }
                else if(cont==2){
                    P2 -> OUT ^= BIT5; //VERDE
                }
            }
            /*El color amarillo va a parpadear cada 10*20ms=200ms*/
            if((i%10)==0){
                if(cont==3){
                    P2 -> OUT ^= BIT7; //AMARILLO
                }
            }
        }else if(semaforo==2){
            if(i>=60 && (i%10)==0){
                if(cont==1){
                    P3 -> OUT ^= BIT5; //ROJO
                }
                else if(cont==2){
                    P3 -> OUT ^= BIT6; //VERDE
                }
            }
            if((i%10)==0){
                if(cont==3){
                    P3 -> OUT ^= BIT7; //AMARILLO
                }
            }
        }else if(semaforo==3){
            if(i>=60 && (i%10)==0){
                if(cont==1){
                    P5 -> OUT ^= BIT0; //ROJO
                }
                else if(cont==2){
                    P5 -> OUT ^= BIT1; //VERDE
                }
            }
            if((i%10)==0){
                if(cont==3){
                    P5 -> OUT ^= BIT2; //AMARILLO
                }
            }
        }else if(semaforo==4){
            if(i>=60 && (i%10)==0){
                if(cont==1){
                    P10 -> OUT ^= BIT2; //ROJO
                }
                else if(cont==2){
                    P10 -> OUT ^= BIT3; //VERDE
                }
            }
            if((i%10)==0){
                if(cont==3){
                    P10 -> OUT ^= BIT4; //AMARILLO
                }
            }
        }



        /*Cambios de color*/
        if(i++ >= 100){ //al entrar 100 veces a la interrupcion se tiene un intervalo de 100*2ms = 2s
                        //que es lo que va a durar encendido cada color

            /*Se modifica la variable cont la cual nos ayudará a cambiar de color*/
            if(cont==3){
                cont=1;
            }else{
                cont++;
            }

            /*En el siguiente apartado se evalua la variable cont
             * para asignar el color correspondiente
             * al semaforo correspondiente*/
            if(semaforo==1){ //semaforo 1
                if(cont==1){
                    P2 -> OUT = BIT4; //ROJO
                }else if(cont==2){
                    P2 -> OUT = BIT5; //VERDE
                }else if(cont==3){
                    P2 -> OUT = BIT7; //AMARILLO
                }
                /*Los otros semaforos en rojo*/
                P3 -> OUT = BIT5; //ROJO S2
                P5 -> OUT = BIT0; //ROJO S3
                P10 -> OUT = BIT2; //ROJO S4
            }else if(semaforo==2){  //semaforo 2
                if(cont==1){
                    P3 -> OUT = BIT5; //ROJO
                }else if(cont==2){
                    P3 -> OUT = BIT6; //VERDE
                }else if(cont==3){
                    P3 -> OUT = BIT7; //AMARILLO
                }
                /*Los otros semaforos en rojo*/
                P2 -> OUT = BIT4; //ROJO S1
                P5 -> OUT = BIT0; //ROJO S3
                P10 -> OUT = BIT2; //ROJO S4
            }else if(semaforo==3){  //semaforo 3
                if(cont==1){
                    P5 -> OUT = BIT0; //ROJO
                }else if(cont==2){
                    P5 -> OUT = BIT1; //VERDE
                }else if(cont==3){
                    P5 -> OUT = BIT2; //AMARILLO
                }
                /*Los otros semaforos en rojo*/
                P2 -> OUT = BIT4; //ROJO S1
                P3 -> OUT = BIT5; //ROJO S2
                P10 -> OUT = BIT2; //ROJO S4
            }else if(semaforo==4){
                if(cont==1){
                    P10 -> OUT = BIT2; //ROJO
                }else if(cont==2){
                    P10 -> OUT = BIT3; //VERDE
                }else if(cont==3){
                    P10 -> OUT = BIT4; //AMARILLO
                }
                /*Los otros semaforos en rojo*/
                P2 -> OUT = BIT4; //ROJO S1
                P3 -> OUT = BIT5; //ROJO S2
                P5 -> OUT = BIT0; //ROJO S3
            }

            /*Se lleva la cuenta de la secuencia con la variable cambio
             * Cuando se hace la secuencia completa en un semaforo
             * Se hace el cambio de semaforo usando la variable semaforo
             * */
            if(cambio==2){
                cambio=0;
                if(semaforo==4){
                    semaforo=1;
                }else{
                    semaforo++;
                    cont=1; //se reinicia el cont para comenzar en color rojo
                }
            }else{
                cambio++;
            }

            i=0;
        }
    }else if(modo==false){ //modo "precaucion"
        if(i++ >= 101){
            i=0;
        }else{
            if((i%10)==0){ //i=20ms -> por lo tanto los LEDS amarillos parpadean cada 200 ms
                P2 -> OUT ^= BIT7; //AMARILLO S1
                P3 -> OUT ^= BIT7; //AMARILLO S2
                P5 -> OUT ^= BIT2; //AMARILLO S3
                P10 -> OUT ^= BIT4; //AMARILLO S4
            }
        }
    }

}

void PORT1_IRQHandler(void)
{
    uint8_t status = P1->IFG; // se guarda el estado de la interrupción P1->IFG
    /*Se comprueba en que pin se habilitó la interrupción*/

    if (status & BIT1){

        TIMER_A3->CTL &= ~(TIMER_A_CTL_MC_1); //detener el TIMER A3

        while(!(P1IN & BIT1)){  //si el boton s1 sigue presionado se ingresa al ciclo, se interrumpe si se suelta
            _delay_cycles(6000000);
                if(!(P1IN & BIT4)){ //se detecta si se ha presionado el boton s2
                    _delay_cycles(6000000);
                    while(!(P1IN & BIT4)){  //si el boton s2 sigue presionado se ingresa al ciclo, hasta que es soltado
                    }
                        if(!(P1IN & BIT1)){     //se comprueba que el botón s1 siga presionado
                            while(!(P1IN & BIT1)){}//se cicla hasta que se deje de presionar
                            if(modo==true){ //"normal" -> "precaución"
                                modo=false;
                                /*Inicializar Semaforos en amarillo*/
                                P2 -> OUT = BIT7; //AMARILLO S1
                                P3 -> OUT = BIT7; //AMARILLO S2
                                P5 -> OUT = BIT2; //AMARILLO S3
                                P10 -> OUT = BIT4; //AMARILLO S4
                            }else{          //"precaución" -> "normal"
                                modo=true;
                                /*Inicializar Semaforos en rojo*/
                                P2 -> OUT = BIT4; //ROJO S1
                                P3 -> OUT = BIT5; //ROJO S2
                                P5 -> OUT = BIT0; //ROJO S3
                                P10 -> OUT = BIT2; //ROJO S4
                            }
                            /*Se reinician variables al cambiar de modo*/
                            cambio=0;
                            semaforo=1;
                            cont=1;
                            i=0;
                        }
                }
        }

    }

    TIMER_A3->CTL |= TIMER_A_CTL_MC_1; //reanudar el TIMER A3
    P1->IFG &= ~(status); // se limpia la bandera de interrupciones que se hayan habilitado
}
