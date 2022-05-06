# AES

El transmisor se encarga de encriptar el dato y lo manda cuando se presiona el boton P1.1

El receptor una vez que recibe el dato encriptado completo (a travé del puerto UART2) lo desencripta y si este es el esperado
enciende el LED P1.0 y se puede observar el dato en la computadora ya que a su vez lo envia por el puerto UART0
