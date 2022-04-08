# ADC-PWM

Se configuró una fuente de reloj de 48 MHz
Para el ADC se usaron divisores de 3 y 4
Mientras que para generar la señal PWM se hizo uso del TIMER A3
La señal tiene un periodo de: 48MHz/(100*2) = 240kHz = 4.166us
Por lo tanto la señal de entrada en el puerto A0 (P5.5) tiene que tener un frecuencia
de 10 kHz para garantizar un buen funcionamiento



- Edwin Daniel Peralta Jijón
- Luis David Casas Hernandez
