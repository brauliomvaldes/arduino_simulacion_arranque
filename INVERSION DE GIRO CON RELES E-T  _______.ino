// RUTINA SIMULANDO ARRANQUE CONTACTORES UTILIZANDO RELES
// Autores: Patricio Tiznado - Braulio Valdés
//
// LED SE ENCIENDEN CON HIGH Y RELES DE ACTIVAN CON LOW
// CON INVERSION DE GIRO 
// ARRANQUE ESTRELLA TRIANGULO 
//
// constantes para identificar pines  
#define BTN_PARO 2
#define BTN_DERECHA 4
#define BTN_IZQUIERDA 6
#define DERECHA 8
#define IZQUIERDA 9
#define ESTRELLA 11
#define TRIANGULO 12

int arranqueder =0;
int arranqueizq =0;
int paro        =0;
int estado1     =0;
int estado2     =0;
unsigned long time;
bool estadoOperativo = false;       		// arranque estrella triangulo completo

volatile int estadoISR = false;			// valiable a modificar en ISR (Interruption Service Rutine) y en ciclo principal

void setup() {
  	// define interrupcion de paro de emergencia, RISING (LOW A HIGH)
  	attachInterrupt(digitalPinToInterrupt(BTN_PARO), interrupcion, RISING);

	// define modo de operacion de los pines segun acciones
  	pinMode(BTN_PARO,       INPUT_PULLUP);
  	pinMode(BTN_DERECHA,    INPUT_PULLUP);
  	pinMode(BTN_IZQUIERDA,  INPUT_PULLUP);
  	pinMode(DERECHA,	OUTPUT);              		// sentido derecha
  	pinMode(IZQUIERDA,	OUTPUT);            		// senstido izquierda
  	pinMode(ESTRELLA,	OUTPUT);         	  	// arranque estrella
  	pinMode(TRIANGULO,	OUTPUT);		        // arranque triangulo

    	// mensajes en terminal arduino
    	Serial.begin(9600);
	reinicio();
}

void arranque(int pin){
	// si arranque estrella triangulo no esta en operación
	if (estadoOperativo==false){
		digitalWrite(pin,       LOW);			// enciende derecha o ezquierda
		digitalWrite(ESTRELLA,  LOW);			// enciende estrella
    		estadoISR = false;

		// reemplaza uso de delay
		// tiempo delay 3 SEGUNDOS
		time=millis()+3000; 	
		// mientras no se presione el boton de paro espera 3 segundos
		while((time>millis())&&(estadoISR == false));

		digitalWrite(ESTRELLA,  HIGH);			// apaga estrella
    		// vuelve a preguntar si ha ocurrido interrupcion
		// si ocurre una interrupcion no espera ni ejecuta triangulo
   		if (estadoISR == false){                 		// si no ocurre una interrupcion
      			digitalWrite(TRIANGULO, LOW);		// enciende triangulo  
      			estadoOperativo = true;   
        		Serial.println("arranque OK");
    		}	
	}
}

void reinicio(){
  	digitalWrite(DERECHA,   HIGH);	
  	digitalWrite(IZQUIERDA, HIGH);
  	digitalWrite(ESTRELLA,  HIGH);	
	digitalWrite(TRIANGULO, HIGH);	
    	Serial.println("apagado OK");
}

void interrupcion(){
  	if(estadoISR == false){           // evita volver a mostrar el mensaje cuando se activa interrupcion 
    		estadoISR = true;
  		apagado();
  	}
}

void apagado(){
  	estado1 = 0;
  	estado2 = 0;
  	estadoOperativo = false;
	reinicio();
}
	
void loop() {
	// lee botones
  	paro        = digitalRead(BTN_PARO);
  	arranqueder = digitalRead(BTN_DERECHA); 
  	arranqueizq = digitalRead(BTN_IZQUIERDA);

  	if (arranqueder == LOW){   		// SE PRESIONO BOTON VERDE 
    		estado1 = 1;
  	}
  	if (estado1 == 1 && estadoOperativo == false){  
      		Serial.println("marcha --->"); 
    		arranque(DERECHA);  		//ENCENDER RELE
  	}
  	else{
      		if(estado1 == 0){
        		digitalWrite(DERECHA, HIGH);  	// APAGAR RELE
      		}
  	}
  	if (arranqueizq == LOW){     		// SE PRESIONO BOTON VERDE
 	   	 estado2 = 1;
 	}
 	if (estado2 == 1 && estadoOperativo == false){
       		Serial.println("marcha <---"); 
	     	arranque(IZQUIERDA); 		//ENCENDER RELE
	}else{
      		if(estado2 == 0){
        		digitalWrite(IZQUIERDA, HIGH);   // APAGAR RELE
      		} 
  	}

  	// reemplaza uso de delay
  	time=millis()+100; // tiempo delay
  	while((time>millis()));
}

