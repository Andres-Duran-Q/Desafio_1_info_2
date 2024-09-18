#include <Adafruit_LiquidCrystal.h>

// Asignación de pines GPIO
const int boton_1 = 3;      // Pin del primer botón
const int boton_2 = 2;      // Pin del segundo botón
const int generador = A0;   // Pin de entrada analógica para la señal generada

// Variables de control
int estado_boton_1 = 0;     // Variable que guarda el estado del botón 1
int estado_boton_2 = 0;     // Variable que guarda el estado del botón 2
int valorActual = 0; 	    // Almacena el valor leído en el pin A0 (señal analógica)

// Banderas de control
bool bandera_boton_1 = false;  // Indica si el botón 1 ha sido presionado
bool bandera_boton_2 = false;  // Indica si el botón 2 ha sido presionado
bool captura_nueva = false;    // Indica si se realizó una nueva captura de datos
bool detecto_fre = false;      // Indica si se ha detectado una frecuencia

// Variables para la amplitud
float amplitud = 0.0;  		  // Almacena la amplitud de la señal
float valorMinimo = 5000.0;   // Valor inicial alto para calibrar la señal mínima
float valorMaximo = -5000.0;  // Valor inicial bajo para calibrar la señal máxima

// Variables de tiempo
unsigned long tiempoInicio = 0;  // Almacena el tiempo del primer cruce por cero
unsigned long tiempoFin = 0;     // Almacena el tiempo del segundo cruce por cero
unsigned long periodo = 0;       // Almacena el período de la señal en microsegundos
int tiempoCalibracion = 1000;    // Tiempo de calibración (en milisegundos)
unsigned long t = 0;             // Variable auxiliar para gestionar el tiempo actual

// Variables para la frecuencia
int valorMedio = 0;              // Valor medio de la señal para detectar cruces de cero
float frecuencia = 0;            // Frecuencia calculada en Hz
bool cruceDetectado = false;     // Indica si se ha detectado un cruce ascendente de la señal
int histeresis = 5;              // Umbral de histéresis para evitar falsos cruces de señal

// Variables para memoria dinámica
int* arreglo = nullptr;      // Puntero para el arreglo dinámico de almacenamiento de valores
int contador = 0;            // Contador de los valores almacenados en el arreglo
int capacidad = 10;          // Capacidad inicial del arreglo (en elementos)
int incremento = 10;         // Cantidad de elementos que se agregarán cuando se expanda el arreglo
int maximo_elementos = 250;  // Máximo número de elementos permitidos en el arreglo

Adafruit_LiquidCrystal lcd_1(0);  // Inicializa el objeto LCD con el puerto I2C 0

void setup() {
  
  // Configuración de los pines de entrada
  pinMode(boton_1, INPUT);  	// Configura el botón 1 como entrada
  pinMode(boton_2, INPUT);   	// Configura el botón 2 como entrada
  pinMode(generador, INPUT); 	// Configura el pin del generador como entrada analógica
  
  // Configuración del LCD
  lcd_1.begin(16, 2);  			// Inicia el LCD con 16 columnas y 2 filas  
  Serial.begin(115200);  		// Inicia la comunicación serial a 115200 baudios
  
  // Mensaje inicial en la pantalla LCD
  lcd_1.print("Desafio_1");
  
  // Inicializa el arreglo dinámico con la capacidad inicial definida
  arreglo = new int[capacidad];
}

// Función para detectar la frecuencia de la señal
void detectarFrecuencia() {
  valorActual = analogRead(generador);  // Lee el valor analógico de la señal
  
  // Detectar cruce ascendente con histéresis para evitar falsos cruces
  if (valorActual >= valorMedio + histeresis && !cruceDetectado) {
    if (tiempoInicio == 0) {
      // Si es el primer cruce, almacena el tiempo
      tiempoInicio = micros();
    } else {
      // Si es el segundo cruce, calcula el período
      tiempoFin = micros();
      periodo = tiempoFin - tiempoInicio;
      frecuencia = 1000000.0 / periodo;  // Calcula la frecuencia en Hz
      tiempoInicio = tiempoFin;          // Reinicia el tiempo para el siguiente ciclo
      detecto_fre = true;                // Marca que se ha detectado la frecuencia
    }
    cruceDetectado = true;  // Marca que se ha detectado un cruce ascendente
  }
  
  // Permite detectar el siguiente cruce cuando la señal cae por debajo del valor medio menos la histéresis
  if (valorActual < valorMedio - histeresis) {
    cruceDetectado = false;
  }
}

// Función para almacenar el valor en un arreglo dinámico, expandiéndolo si es necesario
void almacenarValor() {
  valorActual = analogRead(generador);  // Lee el valor analógico de la señal
  
  // Verifica si hay espacio en el arreglo y si se permite almacenar más elementos
  if (contador < maximo_elementos) {
    if (contador >= capacidad) {
      // Expande el arreglo dinámico si se ha alcanzado la capacidad actual
      capacidad += incremento;
      int* temp = new int[capacidad]; // Crea un nuevo arreglo con mayor capacidad
      for (int i = 0; i < contador; i++) {
        temp[i] = arreglo[i]; // Copia los valores existentes al nuevo arreglo
      }
      delete[] arreglo;       // Libera la memoria del arreglo anterior
      arreglo = temp;         // Actualiza el puntero al nuevo arreglo
    }
    arreglo[contador] = valorActual; // Almacena el nuevo valor en el arreglo
    contador++;                      // Incrementa el contador de elementos almacenados
  }
}

void loop() {
  
  // Leer el estado actual del botón 1
  estado_boton_1 = digitalRead(boton_1);  
  // Leer el estado actual del botón 2
  estado_boton_2 = digitalRead(boton_2);
  
  // Si la bandera de nueva captura está activa, mostrar mensaje de espera
  if(captura_nueva){
    lcd_1.clear();                 // Limpia la pantalla del LCD
    lcd_1.setCursor(0, 0);         // Coloca el cursor en la primera fila
    lcd_1.print("Presione boton 1");  // Imprime mensaje en la primera fila
    lcd_1.setCursor(0, 1);         // Coloca el cursor en la segunda fila
    lcd_1.print("para capturar");  // Imprime mensaje en la segunda fila
    // Espera a que el botón 1 sea presionado para continuar
    while(!(digitalRead(boton_1))) {
    }
  }
  
  // Si el botón 1 está presionado, activa la bandera y reinicia variables
  if (estado_boton_1 == HIGH) {
    bandera_boton_1 = true;         // Activa la bandera para iniciar captura de datos

    // Libera memoria del arreglo dinámico anterior y reinicia variables
    delete[] arreglo;               // Libera el espacio de memoria ocupado por el arreglo anterior
    capacidad = 10;                 // Restablece la capacidad inicial del arreglo
    contador = 0;                   // Reinicia el contador de elementos del arreglo
    arreglo = new int[capacidad];   // Crea un nuevo arreglo con la capacidad inicial

    // Reinicia valores mínimos y máximos para calibración
    valorMinimo = 5000.0;           
    valorMaximo = -5000.0;
    detecto_fre = false;            // Restablece la bandera de detección de frecuencia
    
    // Muestra mensaje de captura de datos en el LCD
    lcd_1.clear();                  
    lcd_1.setCursor(0, 0);
    lcd_1.print("Capturando");
    lcd_1.setCursor(7, 1);
    lcd_1.print("datos...");
  }

  // Si el botón 2 está presionado, activa la bandera de finalización de captura
  if (estado_boton_2 == HIGH) {
    bandera_boton_2 = true;
  }

  // Si la bandera del botón 1 está activa, comienza la calibración y medición
  if (bandera_boton_1) {
    
    unsigned long inicioCalibracion = millis();  // Almacena el tiempo actual para calibración
    
    // Calibración para encontrar los valores mínimo y máximo de la señal
    while (millis() - inicioCalibracion < tiempoCalibracion) {
      valorActual = analogRead(generador);  // Lee el valor de la señal en el pin analógico
      if (valorActual > valorMaximo) {
        valorMaximo = valorActual;          // Actualiza el valor máximo si la lectura es mayor
      }
      if (valorActual < valorMinimo) {
        valorMinimo = valorActual;          // Actualiza el valor mínimo si la lectura es menor
      }
    }
    
    // Calcula la amplitud de la señal en voltios (asume una señal de 5V y 10 bits de resolución)
    amplitud = ((valorMaximo - valorMinimo) / 1023) * 5;
    valorMedio = (valorMaximo + valorMinimo) / 2;  // Calcula el valor medio de la señal para detectar cruces de cero

    // Mientras el botón 1 esté activo, sigue capturando datos
    while (bandera_boton_1) {
      
      detectarFrecuencia();  // Llama a la función para detectar frecuencia
      
      // Si se detecta una frecuencia, almacenar datos con un intervalo basado en la frecuencia
      if(detecto_fre) {
        unsigned long il = micros();  // Almacena el tiempo actual en microsegundos
        // Si ha pasado el tiempo suficiente basado en la frecuencia, almacenar valor
        if(il - t >= (10000/frecuencia)) {
          t = il;  // Actualiza el tiempo para el próximo almacenamiento
          almacenarValor();  // Almacena el valor actual en el arreglo dinámico
        }
      }
      
      // Lee el estado del botón 2
      estado_boton_2 = digitalRead(boton_2);
      // Si el botón 2 está presionado, desactiva la bandera del botón 1 y activa la del botón 2
      if (estado_boton_2 == HIGH) {
        bandera_boton_1 = false;
        bandera_boton_2 = true;
      }
    }
  }
  if (bandera_boton_2) {
    
    lcd_1.clear();               // Limpia el LCD
    lcd_1.setCursor(0, 0);
    lcd_1.print("Ampli:");       // Muestra la amplitud medida
    lcd_1.setCursor(7, 0);
    lcd_1.print(amplitud, 1);  	 // Imprime la amplitud con un decimal
    lcd_1.setCursor(14, 0);
    lcd_1.print("V");  			 // Muestra "V" para indicar que es en voltios
    lcd_1.setCursor(0, 1);
    lcd_1.print("Frecu:"); 		 // Muestra la frecuencia medida
    lcd_1.setCursor(7, 1);
    lcd_1.print(frecuencia, 1);  // Imprime la frecuencia con un decimal
    lcd_1.setCursor(14, 1);
    lcd_1.print("Hz");  		 // Muestra "Hz" para indicar la frecuencia
	
    // Variables para contar tipos de cambios
    int contador_C = 0;  
    int contador_T = 0;
    int contador_S = 0;
    
  
    for (int i = 1; i < contador - 1; i++) {
      
      
      int pendiente_actual = abs(arreglo[i+1]) - abs(arreglo[i]);
      int pendiente_anterior = abs(arreglo[i]) - abs(arreglo[i-1]);
      
      // Detección de onda cuadrada 
      if (arreglo[i] == abs(arreglo[i + 1])) {
        contador_C++;
      }
      
      if(arreglo[i+1]>arreglo[i]){
        if(abs(pendiente_actual-pendiente_anterior)>3){
          contador_T ++;
        }
      }
      else{
        if(abs(pendiente_anterior-pendiente_actual)>3){
          contador_T ++;
        }
      }
      
      if(arreglo[i+1]>arreglo[i]){
        if(abs(pendiente_actual-pendiente_anterior)<3){
          contador_S ++;
        }
      }
      else{
        if(abs(pendiente_anterior-pendiente_actual)<3){
          contador_S ++;
        }
      }
    }
    
    delay(2000);

    
    


    // Clasificación según los cambios detectados
    if (contador_C > 100) {
      lcd_1.clear();  					 // Limpia el LCD
      lcd_1.setCursor(0, 0);
      lcd_1.print("Senal Cuadrada"); 	 // Muestra el tipo de señal
    } 
    else if (contador_S > 100) {
      lcd_1.clear();  					 // Limpia el LCD
      lcd_1.setCursor(0, 0);
      lcd_1.print("Senal Senoidal");     // Muestra el tipo de señal
    } 
    else if (contador_T > 100) {
      lcd_1.clear();  					 // Limpia el LCD
      lcd_1.setCursor(0, 0);
      lcd_1.print("Senal Triangular");   // Muestra el tipo de señal
    }
    else{
      lcd_1.clear();  					 // Limpia el LCD
      lcd_1.setCursor(0, 0);
      lcd_1.print("Senal Desconocida");  // Muestra el tipo de señal
    
    }

    
    
    // Espera hasta que el botón 1 sea presionado para realizar una nueva captura
    while (bandera_boton_2) {
      estado_boton_1 = digitalRead(boton_1);  // Lee el estado del botón 1
      // Si el botón 1 se presiona, desactiva la bandera del botón 2 y activa la nueva captura
      if (estado_boton_1 == HIGH) {
        bandera_boton_2 = false;
        captura_nueva = true;
      }
    }
  }
}
