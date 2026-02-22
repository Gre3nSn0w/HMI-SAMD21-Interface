#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "definitions.h"

// Pines ADC
#define LM35_ADC_PIN 5 // PA05

// Rangos de temperatura
#define TEMP1 25.0f
#define TEMP2 50.0f
#define TEMP3 75.0f

// Número de muestras para promedio
#define ADC_SAMPLES 16

// Delay simple
void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < (ms * 4000); i++)
        __NOP();
}

// Leer temperatura LM35 usando ADC con promedio
float ReadTemperature(void)
{
    uint32_t sum = 0;
    for(uint8_t i = 0; i < ADC_SAMPLES; i++)
    {
        ADC_ChannelSelect(ADC_POSINPUT_PIN5, ADC_NEGINPUT_GND);
        ADC_ConversionStart();
        while(!ADC_ConversionStatusGet());
        sum += ADC_ConversionResultGet();
    }
    
    float avg = sum / (float)ADC_SAMPLES;
    
    float Vref = 3.3f / 2.0f; // Vdda/2 según tu configuración
    float voltage = (avg * Vref) / 4095.0f;
    float temperature = voltage * 100.0f; // LM35 10mV/°C
    return temperature;
}

// Obtener índice de LED según temperatura
uint8_t GetLedIndex(float temp)
{
    if(temp < TEMP1) return 0; // LED0
    else if(temp < TEMP2) return 1; // LED1
    else if(temp < TEMP3) return 2; // LED2
    else return 3; // LED3
}

// Actualizar LEDs
void UpdateLEDs(uint8_t ledIndex)
{
    LED0_Clear();
    LED1_Clear();
    LED2_Clear();
    LED3_Clear();
    
    switch(ledIndex)
    {
        case 0: LED0_Set(); break;
        case 1: LED1_Set(); break;
        case 2: LED2_Set(); break;
        case 3: LED3_Set(); break;
    }
}

// Enviar temperatura a Nextion con dos decimales
void SendNextion(float temperature)
{
    char buffer[40];
    
    sprintf(buffer, "tTemp.txt=\"%.2f Celsius\"", temperature); // linea para comunicar nextion via "sprintf"
    
    // Enviar string
    SERCOM0_USART_Write(buffer, strlen(buffer));
    
    // Enviar los 3 0xFF obligatorios
    uint8_t end_bytes[3] = {0xFF, 0xFF, 0xFF};
    SERCOM0_USART_Write(end_bytes, 3);
}

int main(void)
{
    SYS_Initialize(NULL);
    
    ADC_Initialize();
    ADC_Enable();
    
    // Inicializa LEDs apagados (Set)
    LED0_Set();
    LED1_Set();
    LED2_Set();
    LED3_Set();
    
    while(1)
    {
        float temperature = ReadTemperature();
        uint8_t ledIndex = GetLedIndex(temperature);
        
        UpdateLEDs(ledIndex);
        SendNextion(temperature);
        
        delay_ms(1000); // Actualiza cada segundo
    }
    return 0;
}
