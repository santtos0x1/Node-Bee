# Noctua

**Noctua** é um dispositivo educacional baseado em **ESP32** projetado para **varredura passiva e coleta de dados de redes Wi-Fi e Bluetooth** no ambiente.  
O projeto tem como foco **aprendizado prático** em redes, segurança, protocolos sem fio, sistemas embarcados e organização de dados.

Inspirado na coruja, o Noctua observa silenciosamente, registra informações e permite análise posterior. Sem barulho, sem ataques, sem drama.

> ⚠️ **Disclaimer**  
> Noctua é estritamente **educacional**. O projeto não executa ataques, não explora vulnerabilidades e não interfere no funcionamento de redes ou dispositivos.  
> Qualquer modificação ou uso fora do escopo ético e legal **não é responsabilidade do projeto original**.

---

## Objetivos do Projeto

- Realizar **scan passivo** de redes Wi-Fi próximas e dispositivos BLE.  
- Identificar **redes abertas e protegidas**, sem tentativa de invasão.  
- Coletar metadados técnicos relevantes para estudo e análise.  
- Registrar dados em **microSD** para análise offline posterior.  
- Explorar conceitos de FSM, eventos, estados e logging em sistemas embarcados.  

---

## O que o Noctua Coleta

### Wi-Fi

- SSID  
- BSSID (MAC do Access Point)  
- Canal Wi-Fi  
- Banda (2.4 GHz / 5 GHz)  
- RSSI (intensidade do sinal)  
- Tipo de segurança  
- Timestamp do scan  

> ⚠️ Conexão à rede **não é obrigatória** e **não é o foco principal** do projeto.

---

### Bluetooth (BLE)

- MAC Address  
- Nome do dispositivo (quando disponível)  
- RSSI  
- Tipo de endereço  
- Canal BLE observado  
- Timestamp do scan  

---

### Dados de Log

- ID do arquivo de log  
- Data de criação  

---

## Componentes Necessários

- **1x ESP32 NodeMCU** – microcontrolador principal  
- **2x Botões táteis** – controle simples do fluxo  
- **1x Bateria 5V** – operação portátil  
- **1x Módulo microSD** – armazenamento dos dados coletados  

---

## Arquitetura do Projeto

### Hardware

- ESP32 conectado à:
  - Botões táteis  
  - Módulo microSD  
- Botões:
  - **Botão A** → iniciar novo scan WiFi
  - **Botão B** → iniciar novo scan Bluetooth

---

## Máquina de Estados (FSM)

1. **Idle**  
   - Dispositivo ligado, aguardando ação  

2. **Scan**  
   - Varredura passiva Wi-Fi ou Bluetooth  

3. **Process**  
   - Organização e filtragem dos dados coletados  

> A FSM é simples de propósito. Clareza > complexidade desnecessária.

---

## Ética e Uso Responsável

- Noctua **não executa sniffing ativo**, ataques ou exploração.  
- Não coleta credenciais, payloads ou dados pessoais.  
- O projeto existe para **entender como redes funcionam**, não para quebrá-las.  

---

## Possíveis Expansões Futuras

- Integração com GPS externo para mapeamento geográfico.  
- Modo de economia de energia.  
- Interface serial para análise em tempo real no PC.  

---

## Licença

Este projeto é **open source**, licenciado sob **Apache License 2.0**.  
Você pode estudar, modificar e distribuir o código, **desde que mantenha o uso educacional e ético**.
