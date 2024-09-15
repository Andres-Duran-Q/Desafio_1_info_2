#include <Adafruit_LiquidCrystal.h>

// Asignación de GPIO
const int boton_1 = 3;  // Pin del primer botón
const int boton_2 = 2;  // Pin del segundo botón
const int generador = A0;  // Pin analógico A0

// Variables de estado
int estado_boton_1 = 0;
int estado_boton_2 = 0;
float valor_generador = 0;  // Variable para almacenar el valor leído del pin analógico
float voltaje = 0;          // Variable para almacenar el voltaje actual
int capacidad = 10;         // Capacidad inicial del arreglo de voltajes
int contador = 0;           // Contador para las muestras de voltaje
float minimo = 5;           // Valor mínimo de voltaje (inicializado al máximo posible)
float maximo = 0;           // Valor máximo de voltaje (inicializado al mínimo)
float amplitud = 0;         // Variable para almacenar la amplitud de la señal

// Banderas de control
bool bandera_boton_1 = false;  // Control para iniciar la toma de datos
bool bandera_boton_2 = false;  // Control para finalizar la toma de datos

// Puntero para el arreglo de voltajes
float* voltajes = nullptr;  // Se inicializa el puntero en nulo

// Inicializa el objeto LCD con I2C (usando el bus I2C por defecto con dirección 0x20)
Adafruit_LiquidCrystal lcd_1(0);

void setup() {
  // Configuración de los pines
  pinMode(boton_1, INPUT);   // Botón 1 configurado como entrada
  pinMode(boton_2, INPUT);   // Botón 2 configurado como entrada
  pinMode(generador, INPUT); // Pin del generador configurado como entrada analógica

  // Inicialización del LCD y del puerto serial
  lcd_1.begin(16, 2);        // LCD con 16 columnas y 2 filas usando I2C
  Serial.begin(9600);        // Inicia la comunicación serial para depuración
  
  // Mensaje inicial en el LCD
  lcd_1.print("Desafio_1");

  // Reserva memoria dinámica para almacenar los voltajes
  voltajes = new float[capacidad];
}

void loop() {
  // Lee el estado de los botones
  estado_boton_1 = digitalRead(boton_1);  
  estado_boton_2 = digitalRead(boton_2);

  // Si el botón 1 está presionado, se inicia la captura de datos
  if (estado_boton_1 == HIGH) {
    lcd_1.clear();
    lcd_1.print("Tomando Datos");
    bandera_boton_1 = true;  // Activa la toma de datos
    contador = 0;            // Reinicia el contador de muestras
    capacidad = 10;          // Reinicia la capacidad del arreglo
    minimo = 5;              // Reinicia el valor mínimo
    maximo = 0;              // Reinicia el valor máximo
    
    // Libera la memoria actual y reserva un nuevo arreglo
    delete[] voltajes;
    voltajes = new float[capacidad];
  }

  // Si la bandera del botón 1 está activa, se toma la señal del generador
  while (bandera_boton_1) {
    
    // Lee el valor del generador (señal analógica en A0)
    valor_generador = analogRead(generador);
    
    // Convierte el valor analógico a voltaje
    voltaje = (valor_generador / 1023.0) * 5.0;
    
    // Imprime el valor de voltaje en el monitor serial para depuración
    Serial.println(voltaje);

    // Si se excede la capacidad del arreglo, se redimensiona
    if (contador >= capacidad) {
      capacidad *= 2;  // Duplica la capacidad del arreglo
      float* nuevo_voltajes = new float[capacidad];  // Reserva nuevo arreglo

      // Copia los valores antiguos al nuevo arreglo
      for (int i = 0; i < contador; i++) {
        nuevo_voltajes[i] = voltajes[i];
      }

      // Libera la memoria antigua y actualiza el puntero
      delete[] voltajes;
      voltajes = nuevo_voltajes;
    }

    // Almacena el voltaje en el arreglo
    voltajes[contador] = voltaje;
    contador++;

    // Si el botón 2 se presiona, finaliza la toma de datos
    estado_boton_2 = digitalRead(boton_2);
    if (estado_boton_2 == HIGH) {
      bandera_boton_1 = false;  // Detiene la toma de datos
      bandera_boton_2 = true;   // Activa el proceso de análisis
    }

    delay(100);  // Pequeña demora para evitar lecturas demasiado rápidas
  }
  
  // Si el botón 2 ha sido presionado, se calcula la amplitud
  if (bandera_boton_2) {
        
    // Recorre el arreglo de voltajes para encontrar el mínimo y el máximo
    for (int i = 0; i < contador; i++) {
      if (voltajes[i] > maximo) {
        maximo = voltajes[i];
      }
      if (voltajes[i] < minimo) {
        minimo = voltajes[i];
      }      
    }
    
    // Calcula la amplitud de la señal
    amplitud = maximo - minimo;
    
    // Muestra la amplitud en el LCD
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("AMP:");
    lcd_1.setCursor(4, 0);
    lcd_1.print(amplitud, 2);

    // Desactiva la bandera del botón 2 después de mostrar el resultado
    bandera_boton_2 = false;
    delay(100);
  }
}
