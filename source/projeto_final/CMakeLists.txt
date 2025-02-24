# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(display_oled C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Adiciona os arquivos ao executável, incluindo o arquivo de leitura do botão
add_executable(display_oled
    display_oled.c         # O arquivo principal do OLED
    inc/ssd1306_i2c.c      # Controlador OLED via I2C
    # button.c               # Arquivo para o botão (novo)
)

# Define o nome e a versão do programa
pico_set_program_name(display_oled "display_oled")
pico_set_program_version(display_oled "0.1")


# Configura o uso de UART e USB para depuração
pico_enable_stdio_uart(display_oled 0)
pico_enable_stdio_usb(display_oled 1)

# Adiciona a biblioteca padrão do Raspberry Pi Pico
target_link_libraries(display_oled
    pico_stdlib            # Biblioteca padrão do Pico
    hardware_i2c           # Suporte a comunicação I2C (OLED)
    hardware_gpio          # Suporte a GPIO (botão)
    hardware_pwm           # SUporte ao PWM 
    hardware_adc           # Suporte ao joystick
)

# Inclui os diretórios de cabeçalhos no build
target_include_directories(display_oled PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/cyw43-driver/include  # Inclua o caminho da biblioteca Wi-Fi
)

# Adiciona a geração de arquivos binários e UF2
pico_add_extra_outputs(display_oled)
