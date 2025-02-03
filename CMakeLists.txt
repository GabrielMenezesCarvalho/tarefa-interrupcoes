cmake_minimum_required(VERSION 3.13)

# Definições de padrão de C e C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Importa e inicializa o Pico SDK
include(pico_sdk_import.cmake)

project(tarefawsl C CXX ASM)

pico_sdk_init()

# Adicione todos os arquivos-fonte do seu projeto.
# Se você tiver mais arquivos .c (por exemplo, "ws2812.c"), inclua-os aqui também
add_executable(tarefawsl
    tarefawsl.c
    # ws2812.c  # Se houver um arquivo C para a lógica do WS2812
)

# Defina o nome e versão do seu programa
pico_set_program_name(tarefawsl "tarefawsl")
pico_set_program_version(tarefawsl "0.1")

# Habilite saída via UART/USB (depende do que você estiver usando para debug)
pico_enable_stdio_uart(tarefawsl 1)
pico_enable_stdio_usb(tarefawsl 1)

#
# Gera o cabeçalho a partir do arquivo ws2812.pio
# Isso criará um arquivo "ws2812.pio.h" que você pode incluir no seu código
#
pico_generate_pio_header(tarefawsl ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

#
# Vincula as bibliotecas necessárias:
# - pico_stdlib: funções básicas (GPIO, sleep, etc.)
# - hardware_pio: para usar PIO
# - hardware_timer: caso utilize add_repeating_timer() ou similares
# - hardware_irq: se manipular interrupções diretamente
#
target_link_libraries(tarefawsl
        pico_stdlib
        hardware_pio
        hardware_timer
        hardware_irq
)

# (Opcional) Se usar DMA ou outras libs, adicione-as da mesma forma:
# target_link_libraries(tarefawsl hardware_dma ... )

# Se precisar incluir pastas adicionais (.h externos), faça:
# target_include_directories(tarefawsl PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

# Gera arquivos extras (UF2, BIN, etc.) para o RP2040
pico_add_extra_outputs(tarefawsl)
