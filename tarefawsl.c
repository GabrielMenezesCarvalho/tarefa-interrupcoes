#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// ----------------------------------------------------
// Definições de pinos e parâmetros
// ----------------------------------------------------
#define PIN_LED_R 13 // LED Vermelho
#define PIN_LED_B 12 // LED Azul
#define PIN_LED_G 11 // LED Verde

#define PIN_BUTTON_A 5 // Botão A
#define PIN_BUTTON_B 6 // Botão B

#define PIN_WS2812 7 // GPIO da matriz WS2812 (5x5)
#define WS2812_FREQ 800000
#define IS_RGBW false // 

// Matriz 5x5 => 25 LEDs
#define NUM_PIXELS 25

// Debounce
#define DEBOUNCE_MS 150

static inline uint32_t make_grb(uint8_t r, uint8_t g, uint8_t b)
{
    // Formato: G R B
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

static inline void put_pixel(PIO pio, int sm, uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// ----------------------------------------------------
// Variáveis globais
// ----------------------------------------------------
volatile int g_current_number = 0;       // Armazena o dígito atual (0..9)
volatile bool g_update_requested = true; // Flag para redesenhar a matriz

volatile uint64_t last_press_a = 0; // Timestamp último clique (botão A)
volatile uint64_t last_press_b = 0; // Timestamp último clique (botão B)

// ----------------------------------------------------
// Mapeamento 5x5 para dígitos 0..9
// Cada linha do array é 5 bits (1 = LED aceso, 0 = LED apagado)
//
// porém os Leds na BitdogLab estão assim:

// 1  2  3  4  5
// 10 9  8  7  6
// 11 12 13 14 15
// 20 19 18 17 16
// 21 22 23 24 25

//
// Formato: 5 linhas * 5 colunas = 25 posições
// ----------------------------------------------------
static const bool digitMap[10][NUM_PIXELS] = {
    {// 0
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 1
     0, 1, 1, 1, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 1, 0},
    {// 2
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 3
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 4
     0, 1, 0, 0, 0,
     0, 0, 0, 1, 0,
     1, 1, 1, 1, 1,
     0, 1, 0, 1, 0,
     0, 1, 1, 0, 0},
    {// 5
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},
    {// 6
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},
    {// 7
     0, 0, 0, 0, 1,
     0, 1, 0, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 1, 0,
     1, 1, 1, 1, 1},
    {// 8
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 9
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1}};

// ----------------------------------------------------
// Variáveis de PIO
// ----------------------------------------------------
static PIO pio = pio0; 
static int sm = 0;     
static uint offset;

// ----------------------------------------------------
// Protótipos de função
// ----------------------------------------------------
void init_led_rgb(void);
void init_buttons(void);
void init_ws2812(void);
void update_matrix(int digit);

bool blink_red_led(repeating_timer_t *rt);

void button_callback(uint gpio, uint32_t events);

// ----------------------------------------------------
// main()
// ----------------------------------------------------
int main()
{
    stdio_init_all();

    init_led_rgb();
    init_buttons();
    init_ws2812();

    // Cria um timer repetitivo para piscar LED "vermelho" (pino PIN_LED_R)
    // Frequência 5Hz -> período 200ms, mas chamado a cada 100ms para alternar ON/OFF.
    repeating_timer_t timer;
    add_repeating_timer_ms(-100, blink_red_led, NULL, &timer);

    // Desenha inicialmente o dígito 0
    update_matrix(g_current_number);

    while (true)
    {
        // Se algum botão pediu atualização, redesenha
        if (g_update_requested)
        {
            update_matrix(g_current_number);
            g_update_requested = false;
        }
        // Evita usar CPU 100%
        sleep_ms(10);
    }

    return 0;
}

// ----------------------------------------------------
// Inicializa pinos do LED RGB
// ----------------------------------------------------
void init_led_rgb(void)
{
    gpio_init(PIN_LED_R);
    gpio_set_dir(PIN_LED_R, GPIO_OUT);
    gpio_put(PIN_LED_R, 0);

    gpio_init(PIN_LED_G);
    gpio_set_dir(PIN_LED_G, GPIO_OUT);
    gpio_put(PIN_LED_G, 0);

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);
}

// ----------------------------------------------------
// Inicializa botões e ativa interrupções
// ----------------------------------------------------
void init_buttons(void)
{
    // Botão A
    gpio_init(PIN_BUTTON_A);
    gpio_set_dir(PIN_BUTTON_A, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_A);

    // Botão B
    gpio_init(PIN_BUTTON_B);
    gpio_set_dir(PIN_BUTTON_B, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_B);

    // Callback para os botões:
    //   - Usa gpio_set_irq_enabled_with_callback no pino do botão A.
    //   - No final das contas, registra a 'button_callback' como callback global.
    gpio_set_irq_enabled_with_callback(
        PIN_BUTTON_A,
        GPIO_IRQ_EDGE_FALL,
        true,
        &button_callback);



    gpio_set_irq_enabled(PIN_BUTTON_B, GPIO_IRQ_EDGE_FALL, true);
}

// ----------------------------------------------------
// Inicializa a PIO com o programa ws2812
// ----------------------------------------------------
void init_ws2812(void)
{
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, PIN_WS2812, (float)WS2812_FREQ, IS_RGBW);
}

// ----------------------------------------------------
// Desenha o dígito na matriz 5x5
// ----------------------------------------------------
void update_matrix(int digit)
{
    // Se digitMap[digit][i] = 1 => acende
    // Se digitMap[digit][i] = 0 => apaga
    // Ajuste a cor se quiser outro tom: make_grb(R,G,B)
    uint32_t onColor = make_grb(10, 10, 10); // Branco médio
    uint32_t offColor = make_grb(0, 0, 0);   // Apagado

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (digitMap[digit][i])
        {
            put_pixel(pio, sm, onColor);
        }
        else
        {
            put_pixel(pio, sm, offColor);
        }
    }
}

// ----------------------------------------------------
// Timer repetitivo para piscar LED 'vermelho'
// ----------------------------------------------------
bool blink_red_led(repeating_timer_t *rt)
{
    static bool led_state = false;
    led_state = !led_state;
    gpio_put(PIN_LED_R, led_state);
    return true; // repete
}

// ----------------------------------------------------
// Callback global - vamos tratar A e B na mesma
// pois o SDK só aceita UMA callback global por PIO
// ----------------------------------------------------
void button_callback(uint gpio, uint32_t events)
{
    uint64_t now_ms = to_ms_since_boot(get_absolute_time());

    if (gpio == PIN_BUTTON_A)
    {
        // Debounce para A
        if (now_ms - last_press_a < DEBOUNCE_MS)
        {
            return;
        }
        last_press_a = now_ms;

        // Incrementa
        g_current_number++;
        if (g_current_number > 9)
            g_current_number = 0;

        g_update_requested = true;
    }
    else if (gpio == PIN_BUTTON_B)
    {
        // Debounce para B
        if (now_ms - last_press_b < DEBOUNCE_MS)
        {
            return;
        }
        last_press_b = now_ms;

        // Decrementa
        g_current_number--;
        if (g_current_number < 0)
            g_current_number = 9;

        g_update_requested = true;
    }
}
