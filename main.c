#include <stdio.h> // Biblioteca padrão
#include <math.h> // Biblioteca de matemática (função "round" foi utilizada)

#include "pico/stdlib.h" // Biblioteca padrão pico
#include "hardware/gpio.h" // Biblioteca de GPIOs
#include "hardware/adc.h" // Biblioteca do ADC
#include "hardware/pwm.h" // Biblioteca do PWM
#include "ssd1306.h"


#define BOTAO_A 5
#define BOTAO_B 6
#define LED_R   13
#define LED_G   11
#define BUZZER  21
#define MICROFONE_ADC 2   // ADC2 → GPIO28

#define AMOSTRAS 8000     // 1 segundo a 8kHz
uint16_t buffer[AMOSTRAS];

void setup_gpio() {
    // Botões com pull-up
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    // LEDs
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);

    // Buzzer como saída
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
}

void setup_adc() {
    adc_init();
    adc_gpio_init(28); // GPIO28 = ADC2
    adc_select_input(MICROFONE_ADC);
}

void gravar_audio() {
    gpio_put(LED_R, 1);  // LED vermelho ON
    for (int i = 0; i < AMOSTRAS; i++) {
        buffer[i] = adc_read();  // valor de 0 a 4095
        sleep_us(125); // taxa de amostragem ~8kHz
    }
    gpio_put(LED_R, 0);  // LED vermelho OFF
}

void reproduzir_audio() {
    gpio_put(LED_G, 1);  // LED verde ON

    // Configura PWM
    uint slice = pwm_gpio_to_slice_num(BUZZER);
    pwm_set_wrap(slice, 255);  // resolução do PWM
    pwm_set_enabled(slice, true);

    for (int i = 0; i < AMOSTRAS; i++) {
        uint16_t duty = buffer[i] >> 4;  // escala 12 bits → 8 bits
        pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER), duty);
        sleep_us(125);  // mesma taxa de amostragem
    }

    pwm_set_enabled(slice, false);
    gpio_put(LED_G, 0);  // LED verde OFF
}

int main() {
    stdio_init_all();
    setup_gpio();
    setup_adc();

    while (true) {
        if (!gpio_get(BOTAO_A)) {
            gravar_audio();
            sleep_ms(300); // debounce
        }

        if (!gpio_get(BOTAO_B)) {
            reproduzir_audio();
            sleep_ms(300); // debounce
        }
    }
}
