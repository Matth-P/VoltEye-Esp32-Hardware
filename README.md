# ⚡ VoltEye ESP32

O VoltEye é um sistema de monitoramento elétrico desenvolvido com ESP32 e integração com Supabase. O projeto realiza leituras de tensão e corrente elétrica, calcula a potência consumida e envia os dados para um banco de dados em nuvem utilizando conexão Wi-Fi.

A proposta do projeto é integrar hardware, IoT e banco de dados em tempo real utilizando uma arquitetura simples para estudos, prototipagem e aplicações de monitoramento energético.

---

## 🚀 Funcionamento

A ESP32 realiza leituras analógicas utilizando um sensor ACS712 para corrente elétrica e um divisor resistivo para leitura de tensão. Os valores são processados localmente para cálculo da potência elétrica e enviados para o Supabase através de requisições HTTP.

O sistema também utiliza um LED PWM conectado ao GPIO25 para indicar visualmente o nível de potência consumida.

---

## 🛠️ Tecnologias Utilizadas

- ESP32
- Supabase
- C++

---
