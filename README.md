### TAREFA 1 DIA 27/01: Interrupções no RP2040 com Matriz WS2812 e LED RGB 

#### 📌 Descrição
Este projeto implementa interrupções no microcontrolador **RP2040**, utilizando a placa **BitDogLab**. O objetivo é exibir números de 0 a 9 em uma matriz **5x5 de LEDs WS2812**, controlada via GPIO. Além disso, um **LED RGB** pisca continuamente na cor vermelha e dois botões permitem a troca dos números exibidos na matrix 5x5.

#### 🛠️ Componentes Utilizados
- **Microcontrolador**: Raspberry Pi RP2040 (Placa BitDogLab)
- **Matriz de LEDs WS2812** (endereçáveis) - GPIO 7
- **LED RGB** - GPIOs: 11 (Verde), 12 (Azul), 13 (Vermelho)
- **Botão A** - GPIO 5
- **Botão B** - GPIO 6

#### 🔥 Funcionalidades
✅ O **LED Vermelho** do LED RGB pisca continuamente **5 vezes por segundo**.
✅ O **botão A** incrementa o número exibido na matriz de LEDs.
✅ O **botão B** decrementa o número exibido na matriz de LEDs.
✅ A matriz **5x5 WS2812** exibe números de 0 a 9.
✅ Implementa **debounce via software** para evitar leituras erradas dos botões.
✅ Utiliza **interrupções (IRQ)** para capturar os eventos dos botões sem bloqueios.

#### 📜 Estrutura do Projeto
- `main.c` → Implementação principal do código.
- `ws2812.pio.h` → Arquivo para controlar os LEDs WS2812.
- `CMakeLists.txt` → Configuração do CMake para compilar o projeto.

#### 🖥️ Como Executar o Projeto

**Passo 1**: Clone o repositório do projeto para o seu ambiente local e abra-o no VS Code.

**Passo 2**: Antes de compilar o código, é necessário garantir que todas as extensões e ferramentas estão devidamente instaladas.

Instalar o Pico SDK e extensão Raspberry Pi Pico: Siga as instruções do Pico SDK para configurar o ambiente de desenvolvimento no VS Code e instale a extensão Raspberry Pi Pico.

**Passo 3**: Com o ambiente devidamente configurado, abra o código no VS Code e compile o projeto a partir do menu de compilação da extensão Raspberry Pi Pico.

**Passo 4**: Após a compilação, conecte a placa de desenvolvimento BitDogLab ao computador via USB e coloque-a no modo bootsel.

**Passo 5**: No mesmo menu da extensão Raspberry Pi Pico, selecione a opção "Run Project (USB)" para carregar o uf2 do código na placa e a partir daí já poderá ver o comportamento esperado do código.

#### 🎥 Demonstração em Vídeo
Para ver o projeto em funcionamento, assista ao vídeo abaixo:

➡️ **[https://drive.google.com/drive/folders/1DGsDD6rxZWGepMTtH5STJC7rLoly3Cmm?usp=sharing]**

#### 📌 Autor
Projeto desenvolvido por **Gabriel Menezes Carvalho** para a fase 1 do **EmbarcaTech**.