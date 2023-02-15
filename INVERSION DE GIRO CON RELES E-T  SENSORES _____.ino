// RUTINA SIMULANDO ARRANQUE CONTACTORES UTILIZANDO RELES
// Autores: Patricio Tiznado - Braulio Valdés
//
// LED ENCIENDEN CON HIGH, RELES SE ACTIVAN CON LOW
// CON INVERSION DE GIRO 
// ARRANQUE ESTRELLA TRIANGULO 
// SENSORES DE PARADA Y ARRANQUE SENTIDO CONTRARIO
//
// constantes para identificar pines  
#define BTN_PARO 2
#define BTN_DERECHA 4
#define BTN_IZQUIERDA 6
#define DERECHA 8
#define IZQUIERDA 9
#define ESTRELLA 11
#define TRIANGULO 12

#define SENSOR_A_PARADA_IZQ 3
#define SENSOR_A_ARRANQUE_DER 5
#define SENSOR_B_PARADA_DER 7
#define SENSOR_B_ARRANQUE_IZQ 10

int arranqueder =0;
int arranqueizq =0;
int paro        =0;
int estado1     =0;
int estado2     =0;
unsigned long time;
bool estadoOperativo = false;       		// arranque estrella triangulo completo
bool estadoSensorDeParada = false;  		// estado activación del sensor de arranque
bool detencionPorSensor = false;      // indica activacion de sensor de parada
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

  	// sensores 
  	pinMode(SENSOR_A_PARADA_IZQ, 	INPUT_PULLUP);  		// sensor ladoA parada del sentido izquierdo    |<----
  	pinMode(SENSOR_A_ARRANQUE_DER, 	INPUT_PULLUP);  		// sensor ladoA arranque en sentido derecho     ----->
  	pinMode(SENSOR_B_PARADA_DER, 	INPUT_PULLUP);  		// sensor ladoB parada del sentido derecho     	---->|
  	pinMode(SENSOR_B_ARRANQUE_IZQ, 	INPUT_PULLUP);  		// sensor ladoB arranque en sentido izquierdo  	<-----

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
  	if(estadoISR == false){       // evita volver a mostrar el mensaje cuando se activa interrupcion 
    		estadoISR = true;
  		apagado();
  	}
}

void apagado(){
  	estado1 = 0;
  	estado2 = 0;
  	estadoOperativo = false;
    	detencionPorSensor = false;
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

        // lee sensores cuando estrella-triangulo esta ok y no se ha usado sensor de parada
  	if ((estadoOperativo)&&(detencionPorSensor == false)){
    		leeSensoresParada();
 	}
	// lee sensor de arranque sentido contrario si se acciono sensor de parada
  	if ((detencionPorSensor)&&(estadoSensorDeParada)){
    		leeSensoresDeArranque();
  	}

  	// reemplaza uso de delay
  	time=millis()+100; // tiempo delay
  	while((time>millis()));
}

// si arranque estrella triangulo esta en operación 
void leeSensoresParada(){
	// si estado1 = 1 esta en sentido derecha ---> 
  	if (estado1 == 1){
			// si hay lectura detener sentido derecho 	--->|  
			// activar sensor de arranque izquierdo  	<----
			if (digitalRead(SENSOR_B_PARADA_DER) == LOW){
        			Serial.println("detiene marcha derecha --->|          esperando arranque <<<");
				apagadoPorSensor(DERECHA); 					// apaga arranque sentido derechoe
			}
	}else{
			// sino sentido izquierda <---			 
			// si hay lectura detener sentido izquierdo 	|<--- 
			// activar sensor de arranque derecho  		---->
			if (digitalRead(SENSOR_A_PARADA_IZQ) == LOW){
        			Serial.println("detiene marcha izquiera |<---          esperando arranque >>>");
			  	apagadoPorSensor(IZQUIERDA);  					// apaga arranque izquierdo
			}
	}
}

// lee sensores de arranque si fueron activados por sensores de parada
void leeSensoresDeArranque(){

	// sentido derecho (sensorB)
	if (estado1 == 1){
    		if(digitalRead(SENSOR_B_ARRANQUE_IZQ) == LOW){			
			estado1 = 0;   				// para quitar estado encendido arranque derecho
			estado2 = 1;   				// para encendido arranque izquierdo
				
        		Serial.println("arranque izquierda <----");
        		estadoOperativo = false;
        		arranque(IZQUIERDA); 			// arranque izquierda
        		estadoSensorDeParada = false;  		// para no leer sensores de paro
        		detencionPorSensor = false;
    		}
	}else{
		// sentido izquierdo (sensorA)			 
		// si hay lectura iniciar arranque en sentido derecha  --->
		if (estado2 == 1){
        		if(digitalRead(SENSOR_A_ARRANQUE_DER) == LOW){
				estado1 = 1;   				// para encendido arranque derecho
				estado2 = 0;   				// para quitar estado encendido arranque izquierdo
				
          			//estadoOperativo = false;  // para poder reinicar arranque estrella triangulo
				Serial.println("arranque derecha --->");
          			estadoOperativo = false;
          			arranque(DERECHA); 			// arranque derecha
          			estadoSensorDeParada = false;  		// para no leer sensores de paro
          			detencionPorSensor = false;
			}
      		}
	}
}

// solo apaga pero quda a la espera de arranque en sentido CONTRARIO
void apagadoPorSensor(int pin){
    digitalWrite(pin, HIGH);     // apagar RELE sentido derecho/izquierdo
    digitalWrite(TRIANGULO, HIGH); // apagar RELE
    detencionPorSensor = true;
    estadoSensorDeParada = true;
}


