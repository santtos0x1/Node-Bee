# Noctua

**Noctua** é um dispositivo educacional baseado em **ESP32** projetado para **varredura e coleta de dados de redes Wi-Fi e Bluetooth** no ambiente.  
O projeto tem como foco **aprendizado prático** em redes, segurança, protocolos sem fio, sistemas embarcados e organização de dados.

Inspirado na coruja, o Noctua observa silenciosamente, registra informações e permite análise posterior. Sem barulho, sem ataques, sem drama.

> ⚠️ **Disclaimer**  
> Noctua é estritamente **educacional**. O projeto não explora vulnerabilidades e não interfere no funcionamento de redes ou dispositivos.  
> Qualquer modificação ou uso fora do escopo ético e legal **não é responsabilidade do projeto original**.

---

## Objetivos do Projeto

- Realizar **scan passivo** de redes Wi-Fi próximas e dispositivos BLE.  
- Identificar **redes abertas**, sem tentativa de invasão.  
- Coletar metadados técnicos relevantes para estudo e análise.  
- Registrar dados em **microSD** para análise offline posterior.  
- Explorar conceitos de FSM, eventos, estados e logging em sistemas embarcados.  

---

## Funcionalidades

- **Monitoramento Multi-protocolo:** Varredura passiva de redes Wi-Fi e dispositivos Bluetooth (Classic/BLE) em tempo real.

- **Captura de Metadados:** Coleta automática de informações públicas de redes abertas durante o processo de conexão.

- **Persistência de Dados Offline:** Registro estruturado de logs em cartão microSD (formato CSV), facilitando a análise posterior em softwares de planilhas.

- **Arquitetura Modular:** Código totalmente desacoplado em módulos específicos (FSM, Scanners, Logger, Indicators), garantindo escalabilidade e fácil manutenção.

- **Gerenciamento de Estados (FSM):** Lógica baseada em Máquina de Estados Finita para um fluxo previsível.

- **Resiliência de Hardware (Watchdog):** Sistema de verificação contínua para garantir a inicialização e disponibilidade do módulo microSD.

- **Interface de Diagnóstico:** debug via saída serial (suporte nativo para conversores CH340).

- **Feedback Visual Inteligente:** Sistema de sinalização via LED para indicação de estados, erros de gravação e intensidade de sinal.

- **Servidor de Acesso Remoto (HTTP):** Interface web integrada que permite a listagem e o download de logs diretamente via navegador, eliminando a necessidade de remoção física do cartão microSD.

- **Wardriving:** Captura contínua de SSIDs e níveis de potência (RSSI), permitindo a análise da variação da cobertura e densidade de redes ao longo de um percurso através de logs.

---

## O que o Noctua Coleta

### Wi-Fi

- SSID  
- BSSID (MAC do Access Point)  
- Canal Wi-Fi  
- Banda (2.4 GHz / 5 GHz)  
- RSSI (intensidade do sinal)  
- Tipo de segurança  
- O IP recebido pelo dispositivo
- Status do DHCP
- Máscara de sub-rede
- Hostname

---

### Bluetooth (BLE)

- MAC Address  
- Nome do dispositivo (quando disponível)  
- RSSI  
- Tipo de endereço  
- Canal BLE observado  

---

![SCM](/docs/schematics1.png)

## Componentes Necessários

- **1x ESP32 NodeMCU** – microcontrolador principal  
- **3x Botões táteis** – controle simples do fluxo  
- **1x Bateria 5V** – operação portátil (**Opcional**)
- **1x Módulo microSD** – armazenamento dos dados coletados  
- **2x Resitores de 330Ω** – Caso deseje ligar leds extras (**Opcional**)
- **2x LEDs** – Feedback visual mais polido (**Opcional**)

---

## Arquitetura do Projeto

### Hardware

- ESP32 conectado à:
  - Botões táteis  
  - Módulo microSD  
- Botões:
  - **Botão A** → iniciar novo scan WiFi
  - **Botão B** → iniciar novo scan Bluetooth
  - **Botão C** → iniciar o servidor HTTP

---

## Máquina de Estados (FSM)

1. **IDLE**  
   - Dispositivo ligado, aguardando ação  

2. **SCAN**  
   - Varredura passiva Wi-Fi ou Bluetooth  

3. **PROCESS**  
   - Organização e filtragem dos dados coletados  

4. **WEB_SERVER**
   - Inicio do Servidor HTTP

![Cycle](/docs/operational_cycle.png)

> A FSM é simples de propósito. Clareza > complexidade desnecessária.

---

## Licença

Este projeto é **open source**, licenciado sob **Apache License 2.0**.  
Você pode estudar, modificar e distribuir o código, **desde que mantenha o uso educacional e ético**.
