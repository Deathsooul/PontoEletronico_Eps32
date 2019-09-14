#include "RTClib.h"
#include "EEPROM.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <time.h>

// Variáveis Rahuana
int ultimoendereco = 0;
int address = 0;
String matricula;
String matriculaSalva;
String teste;
int testeINT;
int i, j = 0;
int MAXMEM = 100;
boolean encontrado = false;
const byte qtdLinhas = 4;  //QUANTIDADE DE LINHAS DO TECLADO
const byte qtdColunas = 3; //QUANTIDADE DE COLUNAS DO TECLADO

struct tm data;         //Cria a estrutura que contem as informacoes da data.
char tecla_pressionada; //VERIFICA SE ALGUMA DAS TECLAS FOI PRESSIONADA

// -----------------------

bool cadastro = 0;
bool batePonto = 0;
bool mostraMensagem = 0;

static int Nucleo = 1; // Nucleo que as Task vao rolar

char data_formatada[64];
char hora_formatada[64];

// Modulo RTC no endereco 0x68
RTC_DS1307 rtc;

// Dias da semana
char daysOfTheWeek[7][12] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};

//Linhas e colunas LCD
int lcdColumns = 16;
int lcdRows = 2;

// Modulo I2C display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//Mapeamento do Teclado 4x4
const byte linhas = 4;
const byte colunas = 4;

char Teclas[linhas][colunas] =
    {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}};

byte pinosLinhas[linhas] = {12, 15, 16, 17};
byte pinosColunas[colunas] = {18, 23, 25, 26};

Keypad teclado = Keypad(makeKeymap(Teclas), pinosLinhas, pinosColunas, linhas, colunas);

// Trefas FreeRTOS
void Tarefa1(void *Parametro)
{
  while (1)
  {
    mostraDataHora();
  }
}

void Tarefa2(void *Parametro)
{
  while (1)
  {
    leTeclado();
  }
}

void setup()
{
  // Inicia I2C
  Wire.begin();
  // Inicia Serial
  Serial.begin(115200);
  Serial.print("Iniciei minha Serial");
  delay(1000);
  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();
  if (!rtc.begin())
  {
    Serial.println("Nao achei o RTC");
    while (1)
      ;
  }

  if (!rtc.isrunning())
  {
    Serial.println("Deu alguma zica");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2019, 9, 13, 18, 4, 0));
  }

  //Inicia a EEPROM
  EEPROM.begin(64);

  for (int ii = 0; ii < 10; ii++)
  {
    Serial.println(EEPROM.readString(ii));
  }

  //Tarefas
  xTaskCreatePinnedToCore(Tarefa1, "Tarefa1", 4096, NULL, 2, NULL, Nucleo);
  xTaskCreatePinnedToCore(Tarefa2, "Tarefa2", 4096, NULL, 2, NULL, Nucleo);
}

void loop()
{
}

void leTeclado()
{
  char teclaClicada = teclado.getKey();

  if (teclaClicada == 'A')
  {
    int i = 0;
    bool x = 0;
    char funcionario[5];

    while (x == 0)
    {
      // lcd.clear();
      // lcd.setCursor(0, 1);
      Serial.print("To aqui");
      // lcd.print("Insira Matricula");
      batePonto = 1;

      for (i = 0; i <= 3; i++)
      {
        char tecla = NULL;
        while (tecla == NULL)
        {
          tecla = teclado.getKey();
        }
        funcionario[i] = tecla;
      }
      x = 1;
    }
    batePonto = 0;
    // lcd.clear();
    funcionario[4] = '\0';
    Serial.println("Quero Bate o ponto !?");

    verificaEEPROM(funcionario); // funcao Rahuana busca na EEPROM
  }
  if (teclaClicada == 'C')
  {
    int i = 0;
    bool x = 0;
    char funcionario[5];

    while (x == 0)
    {
      // lcd.clear();
      // lcd.setCursor(0, 1);

      // lcd.print("Cadastro");
      cadastro = 1;
      for (i = 0; i <= 3; i++)
      {
        char tecla = NULL;
        while (tecla == NULL)
        {
          tecla = teclado.getKey();
        }
        funcionario[i] = tecla;
      }
      x = 1;
    }
    cadastro = 0;
    // lcd.clear();
    funcionario[4] = '\0'; //Setamos \0 no FINAL da String pra a funcao .readString() saber aonde termina o registro
    //SALVA NOVO FUNCIONARIO
    EEPROM.writeString(address, funcionario);
    delay(500);
    EEPROM.commit();
    address = address + 4;
    Serial.println("Nego cadastrado !?");
    //Le funcionario
    Serial.println(EEPROM.readString(0));
    Serial.println("Verificando memorias pra frente");
  }
}

void mostraDataHora()
{
  DateTime now = rtc.now();
  if (mostraMensagem == 1)
  {
    mensagemRecepcao();
    delay(1000);
    mostraMensagem = 0;
  }
  else
  {
    while (cadastro == 1)
    {
      lcd.setCursor(0, 1);
      lcd.print("Cadastre!");
    }
    while (batePonto == 1)
    {
      lcd.setCursor(0, 1);
      lcd.print("Matricula:");
    }

    lcd.setCursor(0, 0);
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print(now.second(), DEC);

    lcd.setCursor(0, 1);
    lcd.print(now.year(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.day(), DEC);
  }
}
void salvaEEPROM(String valor)
{
  Serial.println("Salvando dado na EEPROM - Endereco 0: ");
  EEPROM.writeString(address, valor);
  //address += valor.length() + 1;
  delay(500);
  EEPROM.commit();
}
boolean verificaEEPROM(String valor)
{
  Serial.print("Lendo EEPROM: ");
  Serial.println(valor);
  for (int ii = 0; ii < 50; ii++)
  {
    /*Serial.print("Endereço ");
      Serial.print(ii);
      Serial.print(": ");
      Serial.println(EEPROM.readString(ii));*/

    String batata = EEPROM.readString(ii);
    Serial.println(batata);
    if (batata == valor)
    {
      Serial.println("Matricula Encontrada!");
      // ii = 50;
      // imprimeMSG2(valor);

      mostraMensagem = 1;
      //Salva o ponto
      return true;
    }

    // else
    // {
    //   if (ii >= 49)
    //   {
    //     Serial.println("Matricula não encontrada!");
    //     //salvaEEPROM(valor);
    //     imprimeMSG();
    //     delay(500);
    //     return false;
    //   }
    //   else
    //   {
    //   }
    // }
  }
  Serial.println("Nao achou a Matricula");
}
void imprimeMSG2(String msg)
{
  /*lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cadastrado com");
    lcd.setCursor(0, 1);
    lcd.print("Sucesso!");*/
  Serial.print("Matrícula: ");
  Serial.print(msg);
  Serial.print("Data: ");
  Serial.print(data_formatada);
  delay(1000);
}

void imprimeMSG()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nao Cadastrado!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("# - Cadastrar");
  lcd.setCursor(0, 1);
  lcd.print("* - Sair");
}

void mensagemRecepcao()
{

  DateTime now = rtc.now();
  if (now.hour() >= 12 && now.hour() < 18)
  {
    teste = "Boa tarde!";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(teste);
  }
  else if (now.hour() >= 18 && now.hour() <= 23)
  {
    teste = "Boa noite!";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(teste);
  }
  else if (now.hour() >= 0 && now.hour() < 12)
  {
    teste = "Bom dia!";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(teste);
  }
}