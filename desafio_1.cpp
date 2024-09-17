#include <Adafruit_LiquidCrystal.h>

// Asignación de GPIO
const int boton_1 = 3;   // GPIO del primer botón
const int boton_2 = 2;   // GPIO del segundo botón
const int generador = A0; // GPIO del generador

// Variables
int estado_boton_1 = 0;
int estado_boton_2 = 0;
int valor_A0 = 0;        		 // Variable para almacenar el voltaje
int valorMedio = 0;              // Valor medio de la señal (mitad del rango de 10 bits)
float amplitud = 0.0;
float valorMinimo = 5000.0;      // Valor mínimo inicial para calibración
float valorMaximo = -5000.0;     // Valor máximo inicial para calibración
int tiempoCalibracion = 1000;

// Frecuencia
unsigned long tiempoInicio = 0;  // Tiempo del primer cruce por cero
unsigned long tiempoFin = 0;     // Tiempo del segundo cruce por cero
unsigned long periodo = 0;       // Período de la señal
float frecuencia = 0;            // Frecuencia calculada en Hz
bool cruceDetectado = false;     // Marca para indicar si se ha detectado un cruce
int histeresis = 5;  

// Banderas de control
bool bandera_boton_1 = false;
bool bandera_boton_2 = false;
bool calibrado = false;          // Indica si la calibración ha finalizado
bool captura_nueva = false;

Adafruit_LiquidCrystal lcd_1(0);  // Inicializa el objeto LCD

void setup() {
  // Inicializaciones
  pinMode(boton_1, INPUT);   // Configura el GPIO del botón 1 como entrada
  pinMode(boton_2, INPUT);   // Configura el GPIO del botón 2 como entrada
  pinMode(generador, INPUT); // Configura el GPIO del generador como entrada
  
  lcd_1.begin(16, 2);  // Inicia el LCD con 16 columnas y 2 filas  
  Serial.begin(9600);  // Inicia la comunicación serial
  
  lcd_1.print("Desafio_1");
}

void loop() {
  // Lee el estado de los botones
  estado_boton_1 = digitalRead(boton_1);  
  estado_boton_2 = digitalRead(boton_2);

  if(captura_nueva){
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Presione boton 1");
    lcd_1.setCursor(0, 1);
    lcd_1.print("para capturar");
    while(!(digitalRead(boton_1))){

    }

  }
  
  // Si el botón 1 está presionado, activa la bandera y reinicia los valores de calibración
  if (estado_boton_1 == HIGH) {
    bandera_boton_1 = true;
    valorMinimo = 5000.0;
    valorMaximo = -5000.0;
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Capturando");
    lcd_1.setCursor(7, 1);
    lcd_1.print("datos...");
  }

  // Si el botón 2 está presionado, activa la bandera
  if (estado_boton_2 == HIGH) {
    bandera_boton_2 = true;
  }

  // Calibración y medición si el botón 1 está activo
  if (bandera_boton_1) {
    unsigned long inicioCalibracion = millis();
    // Calibración inicial para determinar los valores mínimo y máximo de la señal
    while (millis() - inicioCalibracion < tiempoCalibracion) {
      int valorActual = analogRead(generador);
      if (valorActual > valorMaximo) {
        valorMaximo = valorActual;
      }
      if (valorActual < valorMinimo) {
        valorMinimo = valorActual;
      }
    }
    
    amplitud = ((valorMaximo - valorMinimo) / 1023) * 5;
    valorMedio = (valorMaximo + valorMinimo) / 2;
    calibrado = true;  // Indicar que la calibración ha terminado
  
    while (bandera_boton_1) {
      int valorActual = analogRead(generador);  // Leer el valor analógico de la señal
      // Detectar cruce ascendente con histéresis para evitar ruido
      if (valorActual >= valorMedio + histeresis && !cruceDetectado) {
        if (tiempoInicio == 0) {
          // Si es el primer cruce, iniciar el conteo de tiempo
          tiempoInicio = micros();
        } else {
          // Si es el segundo cruce, calcular el período
          tiempoFin = micros();
          periodo = tiempoFin - tiempoInicio;
          frecuencia = 1000000.0 / periodo;  // Calcular la frecuencia en Hz
          tiempoInicio = tiempoFin;          // Reiniciar el tiempo para el siguiente ciclo
        }
        cruceDetectado = true;  // Marcar que se ha detectado un cruce ascendente
      }
      // Permitir detectar el siguiente cruce cuando la señal caiga por debajo del valor medio menos la histéresis
      if (valorActual < valorMedio - histeresis) {
        cruceDetectado = false;
      }
      
      
      estado_boton_2 = digitalRead(boton_2);
      if (estado_boton_2 == HIGH) {
        bandera_boton_1 = false;
        bandera_boton_2 = true;
      }
    }
  }

  // Mostrar resultados en el LCD si el botón 2 está activo
  if (bandera_boton_2) {
    // Display
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Ampli:");
    lcd_1.setCursor(7, 0);
    lcd_1.print(amplitud, 1);
    lcd_1.setCursor(14, 0);
    lcd_1.print("V");
    lcd_1.setCursor(0, 1);
    lcd_1.print("Frecu:");
    lcd_1.setCursor(7, 1);  // Corregido para imprimir en la fila correcta
    lcd_1.print(frecuencia, 1);
    lcd_1.setCursor(14, 1);
    lcd_1.print("Hz");

    while (bandera_boton_2) {
      estado_boton_1 = digitalRead(boton_1);
      if (estado_boton_1 == HIGH) {
        bandera_boton_2 = false;
        captura_nueva = true;
      }
    }

    //delay(500);
  }
}
