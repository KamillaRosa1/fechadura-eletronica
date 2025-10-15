# Fechadura Eletrônica com Arduino

##  Visão Geral do Projeto

Este projeto implementa um sistema de controle de acesso utilizando um Arduino. O sistema oferece uma interface com um teclado matricial 4x4 para entrada de dados e um Display LCD 20x4 para feedback em tempo real e navegação em menu de configuração. O código é estruturado como uma Máquina de Estados e possui a persistência do PIN de acesso na memória EEPROM do microcontrolador.

---

## Hardware Necessário

| Componente | Quantidade | Observação |
| :--- | :--- | :--- |
| **Placa Microcontroladora** Arduino UNO | 1 | - |
| **Display LCD** 20x4 | 1 | com Módulo I2C. |
| **Teclado Membrana 4x4** | 1 | - |
| **Atuador (Simulação)** | 1 | Módulo de Relé 5V (Para controle de trava/destrava). |
| **Jumpers** | Variável | Macho-Macho e Macho-Femêa |

## Pinagem e Conexões

A pinagem abaixo é a que foi utilizada no código.

| Componente | Pino do Componente | Pino do Arduino Uno |
| :--- | :--- | :--- |
| **LCD I2C** | SDA | A4 |
| **LCD I2C** | SCL | A5 | 
| **Relé** | IN | D10 |
| **Relé** | VCC / GND | 5V / GND |
| **Teclado (Linhas)** | R1 (Pino 1) | D5 |
| **Teclado (Linhas)** | R2 (Pino 2) | D4 |
| **Teclado (Linhas)** | R3 (Pino 3) | D3 |
| **Teclado (Linhas)** | R4 (Pino 4) | D2 |
| **Teclado (Colunas)** | C1 (Pino 5) | D9 |
| **Teclado (Colunas)** | C2 (Pino 6) | D8 |
| **Teclado (Colunas)** | C3 (Pino 7) | D7 |
| **Teclado (Colunas)** | C4 (Pino 8) | D6 |

## Software e Dependências

### Bibliotecas

É necessário instalar as seguintes bibliotecas via Gerenciador de Bibliotecas do Arduino IDE:

1.  **`LiquidCrystal_I2C`**: Para o display.
2.  **`Keypad`**: Para o teclado.

### Fluxo de Operação e Comandos

O PIN estipulado como padrão é **`1234`**. O tempo de acesso concedido (relé ativo) é de $5$ segundos.

|Tecla | Função |	
| :--- | :--- | 
| 0-9	| Entrada de Dados - Digitação do PIN. |
| # |	Confirmação	- Finaliza a digitação para Acesso ou Salva/Confirma PIN em Configuração. |
| *	| Limpar/Cancelar	- Limpa a entrada atual e retorna ao modo BLOQUEADO. Não funciona no estado ABERTO. |
| A	| Configuração - Inicia o processo de mudança do PIN. |

### Estrutura da Máquina de Estados (Lógica Central)

| Estado | Descrição | Relé |
| :--- | :--- | :--- |
| `ESTADO_BLOQUEADO` | Aguardando entrada do PIN. | Desligado (Travado) |
| `ESTADO_ABERTO` | Acesso liberado. Duração de 5 segundos. | Ligado (Destravado) |
| `ESTADO_PEDIR_PIN_ATUAL` | Verifica o PIN atual antes de permitir a mudança. | Desligado (Travado) |
| `ESTADO_DEFINIR_NOVO_PIN` | Aguarda a digitação do novo PIN e o salva na EEPROM. | Desligado (Travado) |

### Persistência do PIN (EEPROM)

O PIN é armazenado na memória EEPROM do Arduino para que permaneça o mesmo após o desligamento:

* **Endereços Usados:** A partir do endereço `0`.
* **Flag de Validação:** Um valor de validação (`0xAA`) é armazenado no endereço `0` para confirmar se um PIN válido já foi escrito na memória.
* Se o Flag for inválido ou a EEPROM estiver virgem, o sistema salva e utiliza o **PIN Padrão** (`1234`).


