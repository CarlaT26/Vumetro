#include "mbed.h"

#define Ds  D4
#define STCP D3
#define SHCP D2

#define tiempo_1  1s
#define tiempo_2  200ms 

// Funciones
void shift_data(char32_t data);
void load_data(char32_t data);
void clear_data(void);
void send_data(void);

// Hilo (Prioridad, tamaño, atributos, argumentos)
Thread T_send_data(osPriorityNormal, 4096, NULL, NULL); 

// Pines y puertos
DigitalOut Serial_data(Ds); //Envia datos al registro de desplazamiento
DigitalOut Serial_clk(SHCP); // Pulso de reloj 
DigitalOut LatchClk(STCP); //Pulso de cierre 

int main()
{
    // Inicializar
    Serial_data = 0;
    Serial_clk = 0;
    LatchClk = 0;
    clear_data();
    
    // Inicia el hilo
    T_send_data.start(send_data); 

    while(true) 
    {
    }
}

void shift_data(char32_t data) //Enviar desplazamiento entre bits
{
    for (int i=0; i<24; i++)
    {
        Serial_data = ((data & 0x01) == 0x01) ? 1 : 0; 
        Serial_clk = 0; // Reloj de desplazamiento 
        wait_us(1);
        Serial_clk = 1;
        wait_us(1); 
        data = data >> 1; //Desplaza el bit 
    }
}

void load_data(char32_t data)
{
    LatchClk = 0; 
    shift_data(data); 
    LatchClk = 1; 
}

void clear_data(void)
{
    char32_t data = 0x000000;  // 24 bits en 0
    load_data(data); 
}

void send_data(void)
{
    char32_t patrones [5] = {
        0xAAAAAA, // Patrón 1: 101010101010101010101010
        0x555555, // Patrón 2: 010101010101010101010101
        0xF0F0F0, // Patrón 3: 111100001111000011110000
        0x0F0F0F, // Patrón 4: 000011110000111100001111
        0xFF00FF  // Patrón 5: 111111110000000011111111

    };

        // Nuevo patrón: escalera ascendente 
        char32_t acumulado = 0x000000;  // Inicializar con todos los bits 0

        for (int j = 0; j < 24; j++)
        {
            acumulado |= (1 << j);  // Encender el bit actual y mantener los anteriores encendidos
            load_data(acumulado);  
            ThisThread::sleep_for(tiempo_2);
        }
        // Escalera descendente
        for (int j = 23; j >= 0; j--)
        {
            acumulado &= ~(1 << j);  // Apagar el bit actual
            load_data(acumulado); 
            ThisThread::sleep_for(tiempo_2);
        }

    while(true)
    {
        
        for (int i = 0; i < 5; i++)  // Recorre cada patrón
        {
            load_data(patrones[i]);
            ThisThread::sleep_for(tiempo_1);
        }
    }
}