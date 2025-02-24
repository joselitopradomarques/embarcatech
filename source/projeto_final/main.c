#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

// Definição dos pinos I2C
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
#define LED_BLUE 12   // GPIO conectado ao terminal azul do LED RGB
#define BUTTON_A 5    // GPIO conectado ao Botão A
#define BUTTON_B 6    // GPIO conectado ao Botão B
#define BUZZER_PIN 21 // Configuração do pino do buzzer
#define BUZZER_FREQUENCY 100 // Configuração da frequência do buzzer (em Hz)

// Parte do código para exibir as mensagens no display
char *text[] = {
    "   Bom trabalho   ",
    "    continue      ",
    "     focado       "
};

char *text1[] = {
    "   Vamos fazer     ",  // Primeira linha (21 caracteres)
    "   uma pausa?      ",  // Segunda linha (21 caracteres)
    "  vamos meditar    ",  // Terceira linha (21 caracteres)
    "  por 10 minutos   "   // Quarta linha (21 caracteres)
    "Clique A quando",
    "finalizar!!"
};

// Variável para controlar se o buzzer deve parar
bool buzzer_disabled = false;
absolute_time_t buzzer_disabled_until;

// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Definição de uma função para emitir um beep com duração especificada
void beep(uint pin, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Verificar se o buzzer está desativado
    if (buzzer_disabled && absolute_time_diff_us(get_absolute_time(), buzzer_disabled_until) > 0) {
        // Não emitir som se o buzzer estiver desativado
        return;
    }

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}

void clear_oled_display(uint8_t *ssd, struct render_area *frame_area) {
    // Criar uma string de espaços do tamanho da largura da tela
    char empty_line[ssd1306_width / 6]; // 6 pixels por caractere na largura
    memset(empty_line, ' ', sizeof(empty_line) - 1); // Preenche com espaços
    empty_line[sizeof(empty_line) - 1] = '\0'; // Adiciona o terminador nulo

    // Preencher todas as linhas com espaços
    int y = 0;
    for (int i = 0; i < ssd1306_n_pages; i++) {
        ssd1306_draw_string(ssd, 0, y, empty_line);  // Exibe a linha vazia
        y += 8;  // Avança 8 pixels na vertical (próxima página)
    }

    // Atualiza o display
    render_on_display(ssd, frame_area);
}


int main() {
    // Inicialização padrão dos pinos
    stdio_init_all();
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    adc_gpio_init(27);

    // Inicialização do I2C
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Configuração do GPIO do LED como saída
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, false);  // Inicialmente, o LED está apagado

    // Configuração do GPIO do Botão A como entrada com pull-up interno
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Inicialização do display OLED SSD1306
    ssd1306_init();

    // Configuração da área de renderização para o display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    // Inicializar o PWM no pino do buzzer
    pwm_init_buzzer(BUZZER_PIN);

    // Calcula o comprimento do buffer necessário para a renderização
    calculate_render_area_buffer_length(&frame_area);

    // Limpa o display
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Inicia o temporizador
    absolute_time_t start_time = get_absolute_time();

    int y = 0;
    for (uint i = 0; i < sizeof(text)/sizeof(text[0]); i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);  // Exibe as strings
        y += 8;  // Avança 8 pixels na vertical
    }
    render_on_display(ssd, &frame_area);

while (true) {
    tight_loop_contents();
    
    // Lê o estado dos Botões A e B
    bool button_b_state = gpio_get(BUTTON_B);  // HIGH = solto, LOW = pressionado

    y = 0;
    for (uint i = 0; i < sizeof(text)/sizeof(text[0]); i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);  // Exibe as strings
        y += 8;  // Avança 8 pixels na vertical
    }
    render_on_display(ssd, &frame_area);

    // Verificar se já passaram 10 segundos
if (absolute_time_diff_us(start_time, get_absolute_time()) >= 10000000) {  // 10s = 10.000.000µs
    gpio_put(LED_BLUE, true);   // Liga o LED
    sleep_ms(500);              // Mantém ligado por 500ms
    gpio_put(LED_BLUE, false);  // Desliga o LED
    sleep_ms(500);              // Pequeno delay antes de continuar

    int y = 0;
    for (uint i = 0; i < sizeof(text1)/sizeof(text1[0]); i++) {
        ssd1306_draw_string(ssd, 5, y, text1[i]);  // Exibe as strings
        y += 8;  // Avança 8 pixels na vertical
    }
    render_on_display(ssd, &frame_area);

    // Aguarda até que o Botão A seja pressionado
    while (gpio_get(BUTTON_A) == 1) {  // Assume que o botão está em nível alto quando não pressionado
        sleep_ms(10);  // Pequeno atraso para evitar sobrecarga de CPU
    }

    // Reinicia o temporizador para 10s novamente
    start_time = get_absolute_time();
}

    clear_oled_display(ssd, &frame_area);
    int y = 0;
    for (uint i = 0; i < sizeof(text)/sizeof(text[0]); i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);  // Exibe as strings
        y += 8;  // Avança 8 pixels na vertical
    }
    render_on_display(ssd, &frame_area);


    sleep_ms(5000);
    
}
}
