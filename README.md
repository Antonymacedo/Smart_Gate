# Smart-gate🤖
Circuito usando ESP8266 para automatizar portão eletrico residencial, pode ser aberto pelo telefone usando o navegador 
e o endereço de IP da placa ou pela Alexa (da amazon). Este projeto funciona com a seguinte lógica:

* O ESP8266 é o cérebro de tudo e permanece constantemente conectado à rede Wi-Fi, aguardando comandos externos. 
Esses comandos podem vir tanto da Alexa, por meio da integração com o serviço SinricPro, quanto do navegador do celular ou computador,
ao acessar o endereço IP local do ESP, como por exemplo “http://IP-do-seu-dispositivo/acionar”. 
Enquanto nenhum comando é recebido, o ESP fica em estado de espera, e o relé permanece ligado, abrindo o normalmente fechado(NC).

* Quando um comando de acionamento é enviado, seja pela Alexa, seja pelo navegador, o ESP reage desligando o relé por
exatamente um segundo. Esse curto tempo é o suficiente para simular o funcionamento do botão físico original do
portão. Esse botão, conhecido como BOT do portão, é aquele que você normalmente aperta manualmente para abrir
ou fechar o portão. A função do relé nesse sistema é justamente simular esse aperto de botão. Ao desligar o relé, o ESP
faz com que ele feche os dois terminais do BOT por um breve momento, enviando um pulso elétrico que "engana" o
portão, fazendo-o pensar que alguém apertou o botão físico. Após esse segundo de contato fechado, o ESP liga o
relé automaticamente, e tudo volta ao estado inicial de espera.

* O que acontece dentro do portão após o pulso é controlado pela própria lógica interna do equipamento. Ou seja, se o portão estiver
fechado e receber esse pulso, ele começa a abrir; se estiver aberto e receber o pulso, ele começa a fechar. É o mesmo princípio de
funcionamento do botão físico: um toque alterna entre abrir e fechar, sem que o usuário precise se preocupar com o estado atual, o
portão cuida disso sozinho. Com esse sistema, você consegue abrir ou fechar o portão remotamente, usando a Alexa ou o navegador, sem
perder a funcionalidade original do controle remoto e do botão físico, que continuam funcionando em paralelo.


## Materiais🛠️

**1.** ESP8266

**2.** Relé **5V**

**3.** Transistor UTC7805 (para abaixar de **12V** para **5V**)

<img width="512" height="377" alt="image" src="https://github.com/user-attachments/assets/2e75465a-54ff-416d-b51d-0b6045f2fc26" />

OBS: Também é necessário cabos jumper e ferramentas como chaves philips.

## Montagem 

<img width="683" height="500" alt="download" src="https://github.com/user-attachments/assets/2a7685b7-6f96-4904-8562-4dcf02c36a35" />

* Conecte o 12V do portão na "entrada" do transistor e o GND do portão no GND do transistor

* Conecte a saida de 5V do transistor no ESP8266 e um dos GND do portão tambem no ESP

* Conecte o 5V do ESP no Relé, e o GND do ESP no GND do Relé

*  Conecte o NC do Relé no BOT do portão

*  Conecte o COM do Relé no ANT (funciona da mesma forma do GND) do portão



## Conexões⚡

| Componente | Pino do ESP8266 |
|------------|-----------------|
| IN  -> Relé  | D1            |
| 5V  -> Relé  | VV -> ESP8266 |
| GND -> Relé  | GND           |
| 5V -> Transistor|VIN -> ESP8266 |


| Ligação      |Componente     |
|--------------|---------------|
| 12V -> Portão| Transistor    |
| GND -> Portão| Transistor    |
| GND -> Portão| GND -> ESP8266|


### Pinagens

**Pinagem do ESP8266**

<img width="683" height="800" alt="download" src="https://github.com/user-attachments/assets/d9429ecc-914d-4087-86bb-4556c279e6a0" />

_____________________________________________________________________________________________________________________________________________________________________________________

**Pinagem do Relé**

<img width="683" height="500" alt="download" src="https://github.com/user-attachments/assets/616e1037-55c0-430c-a4ef-791bc8b3089d" />

______________________________________________________________________________________________________________________________________________________________________________________

**Pinagem do Transistor**

<img width="683" height="500" alt="download" src="https://github.com/user-attachments/assets/001ded3d-dac3-454e-a458-8ce7cf0a140f" />



## Código💻

```
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

// --- Configurações WiFi e SinricPro
#define WIFI_SSID "Nome da sua rede de internet"
#define WIFI_PASS "senha da sua rede"
#define APP_KEY "sua chave no sinricPro"
#define APP_SECRET "sua app SECRET"
#define SWITCH_ID "ID do switch no sinricPro"

// --- Pinos
#define RELAY_PIN D1
#define BUTTON_PIN D6

ESP8266WebServer server(80);

bool estadoAnteriorBotao = HIGH; // pull-up
bool estadoRelé = false;

// --- Função para acionar o relé por 0.8 segundo
void acionarRele() {
  digitalWrite(RELAY_PIN, LOW);  // Ativa relé (nível baixo)
  delay(800);                   // Mantém acionado por 0.8s
  digitalWrite(RELAY_PIN, HIGH); // Desativa relé
  Serial.println("Relé acionado");
}

// --- Callback SinricPro (Alexa)
bool onPowerState(const String &deviceId, bool &state) {
  if (deviceId == SWITCH_ID) {
    acionarRele();
    return true;
  }
  return false;
}

// --- Requisição HTTP para acionar relé
void handleAcionar() {
  acionarRele();
  server.send(200, "text/html", "<h1>Portão acionado</h1><a href=\"/acionar\"><button>Acionar Novamente</button></a>");
}

// --- Configura Wi-Fi
void setupWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relé desativado inicialmente

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setupWiFi();

  server.on("/acionar", handleAcionar);
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  SinricProSwitch &meuSwitch = SinricPro[SWITCH_ID];
  meuSwitch.onPowerState(onPowerState);

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void loop() {
  SinricPro.handle();
  server.handleClient();

  // Leitura do botão com debounce simples
  bool estadoBotao = digitalRead(BUTTON_PIN);
  if (estadoBotao == LOW && estadoAnteriorBotao == HIGH) {
    Serial.println("Botão pressionado");
    acionarRele();
    delay(300); // debounce
  }
  estadoAnteriorBotao = estadoBotao;
}
```


## Resultado📲


https://github.com/user-attachments/assets/16e84ca1-dee2-4c12-9f7d-d13910c22081


## cosideraçôes❗

* Preste bastante atenção a lógica de funiconamento do seu portão, inclusive recomendo que leia o manual antes de iniciar o projeto em sua casa.

* Certifique-se de que o portão quando abre completamente e fecha completamente está parando, para não causar uma sobrecorrente no motor que levara a um provavel curto posteriormente


a placa do meu portão é essa:

![D_652620-MLA83383484435_032025-C](https://github.com/user-attachments/assets/1071c696-73e1-4029-b3f8-6f4783bb3f92)

mas acredito que a lógica de cada placa não deve mudar muito, então no maximo alguns nomes vão ser diferentes




