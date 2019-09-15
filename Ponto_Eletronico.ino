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
bool pessoaNaoEncontrada = 0;
bool pessoaEncontrada = 0;
uint32_t unixAgora = 0; // Variavel para que o RTC nao se perca durante execução das tasks

int Ano;
int Mes;
int Dia;
int Hora;
int Min;

bool mudaHoraPff = false;

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
  Serial.println("Iniciei minha Serial");
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
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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
  EEPROM.begin(512);

  limpaTudo();

  // Serial.println(EEPROM.read(0));

  // novaPessoa("6969");
  // novaPessoa("1111");
  // novaPessoa("9999");
  // novaPessoa("2222");

  // imprimeAll();

  // while (1)
  // {
  //   /* code */
  // }

  //Tarefas
  xTaskCreatePinnedToCore(Tarefa1, "Tarefa1", 4096, NULL, 1, NULL, Nucleo);
  xTaskCreatePinnedToCore(Tarefa2, "Tarefa2", 4096, NULL, 1, NULL, Nucleo);
}

void loop()
{

  // send data only when you receive data:
  if (Serial.available() > 0)
  {
    Serial.println("Oi to aqui");
    // read the incoming byte:
    char incomingByte = Serial.read();

    if (incomingByte == 'D')
    {
      novaHoraData();
    }
  }
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

    verificaFuncionario(funcionario);
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

    //SALVA NOVO FUNCIONARIO
    novaPessoa(funcionario);
    imprimeAll();

    // EEPROM.writeString(address, funcionario);
    // delay(500);
    // EEPROM.commit();
    // address = address + 4;
    // Serial.println("Nego cadastrado !?");
    // //Le funcionario
    // Serial.println(EEPROM.readString(0));
    // Serial.println("Verificando memorias pra frente");
  }
  if (teclaClicada == 'C')
  {
    imprimeAll();
  }
}

void mostraDataHora()
{
  if (mudaHoraPff == true)
  {
    rtc.adjust(DateTime(Ano, Mes, Dia, Hora, Min, 0));

    mudaHoraPff = false;
  }
  DateTime now = rtc.now();
  unixAgora = now.unixtime();
  if (mostraMensagem == 1)
  {
    mensagemRecepcao();
    delay(1000);
    mostraMensagem = 0;
    lcd.clear();
  }
  else if (pessoaNaoEncontrada == 1)
  {
    mostraNaoEncontrada();
    delay(1000);
    pessoaNaoEncontrada = 0;
    lcd.clear();
  }
  else if (pessoaEncontrada == 1)
  {
    encontrada();
    delay(1000);
    pessoaEncontrada = 0;
    lcd.clear();
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

void mostraNaoEncontrada()
{
  naoEcontrada();
}

void salvaEEPROM(String valor)
{
  Serial.println("Salvando dado na EEPROM - Endereco 0: ");
  EEPROM.writeString(address, valor);
  //address += valor.length() + 1;
  delay(500);
  EEPROM.commit();
}

//Rahuana
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

void naoEcontrada()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pessoa nao");
  lcd.setCursor(0, 1);
  lcd.print("Encontrada!");
}

void encontrada()
{
  mensagemRecepcao();
}

//funcao que sempre passa a posição do proximo bloco de salvamento
int pessoaEEPROM(int x)
{
  return (x * 24) + 1;
}
// funcao de cadastro de pessoa
void novaPessoa(String matricula)
{
  int quantidadeDePessoas = quantidadePessoas();
  int enderecoParaGravar;
  enderecoParaGravar = pessoaEEPROM(quantidadeDePessoas);

  // Serial.print("ESTOU GRAVANDO EM:");
  // Serial.println(enderecoParaGravar);
  //Salvar nome
  for (int i = 0; i <= 3; i++)
  {
    EEPROM.write((enderecoParaGravar + i), matricula[i]);
  }

  for (int j = 4; j <= 23; j++)
  {
    EEPROM.write((enderecoParaGravar + j), 'A'); // Salva tudo como 0 por a pessoa estar sendo cadastrada
  }
  EEPROM.write(0, (quantidadeDePessoas + 1)); // Incrementa a quantidade de pessoas
  EEPROM.commit();
}

// Sempre le o endereço 0 que vai ser responsavel por armazenar a quantida de pessoas que fora cadastradas
int quantidadePessoas()
{
  return EEPROM.read(0);
}

// imprimi todo mundo
void imprimeAll()
{
  int quantidadedePessoas = quantidadePessoas();
  for (int i = 0; i < quantidadedePessoas; i++)
  {
    int posicao = pessoaEEPROM(i);
    char buffMatricula[5];
    char buffEntrada[11];
    char buffSaida[11];

    for (int j = 0; j <= 3; j++)
    {
      buffMatricula[j] = EEPROM.read(posicao + j);
    }

    buffMatricula[4] = '\0';
    Serial.print("Matricula: ");
    // Serial.print("M");
    Serial.println(buffMatricula);

    for (int j = 4; j <= 13; j++)
    {
      buffEntrada[j - 4] = EEPROM.read(posicao + j);
    }
    buffEntrada[10] = '\0';
    Serial.print("Entrada: ");
    // Serial.print("P");
    Serial.println(buffEntrada);

    for (int j = 14; j <= 23; j++)
    {
      buffSaida[j - 14] = EEPROM.read(posicao + j);
    }
    Serial.print("Saida: ");
    buffSaida[10] = '\0';
    // Serial.print("P");
    Serial.println(buffSaida);
  }
}

bool verificaFuncionario(char *matricula)
{
  int quantidadedePessoas = quantidadePessoas();
  for (int i = 0; i < quantidadedePessoas; i++)
  {
    int posicao = pessoaEEPROM(i);
    char buffMatricula[5];

    for (int j = 0; j <= 3; j++)
    {
      buffMatricula[j] = EEPROM.read(posicao + j);
    }
    buffMatricula[4] = '\0';

    if (strcmp(matricula, buffMatricula) == 0)
    {
      Serial.println("Entrei");
      pessoaEncontrada = 1;
      preenchePonto(matricula);
      imprimeAll();
      return true;
    }
  }
  Serial.print("Pessoa nao encontrada");
  pessoaNaoEncontrada = 1;
}

//Logica de preenchePonto
void preenchePonto(char *matricula)
{
  bool needPreenchimento = true;

  int quantidadedePessoas = quantidadePessoas();
  for (int i = 0; i < quantidadedePessoas; i++)
  {
    int posicao = pessoaEEPROM(i);
    char buffMatricula[5];

    for (int j = 0; j <= 3; j++)
    {
      buffMatricula[j] = EEPROM.read(posicao + j);
    }
    buffMatricula[4] = '\0';
    Serial.println(matricula);
    Serial.println(buffMatricula);

    if (strcmp(matricula, buffMatricula) == 0)
    {
      char buffEntrada[11];
      char buffSaida[11];

      //Varredura da entrada e saida
      for (int j = 4; j <= 13; j++)
      {
        buffEntrada[j - 4] = EEPROM.read(posicao + j);
      }
      buffEntrada[10] = '\0';

      for (int j = 14; j <= 23; j++)
      {
        buffSaida[j - 14] = EEPROM.read(posicao + j);
      }
      buffSaida[10] = '\0';

      char buff[11];
      itoa(unixAgora, buff, 10);

      if (buffEntrada[0] == 'A')
      {
        //bate ponto de entrada
        for (int j = 4; j <= 13; j++)
        {
          EEPROM.write(posicao + j, buff[j - 4]);
        }
        EEPROM.commit();
        needPreenchimento = false;
      }
      else if (buffSaida[0] == 'A')
      {
        //Bate ponto de saida
        for (int j = 14; j <= 23; j++)
        {
          EEPROM.write(posicao + j, buff[j - 14]);
        }
        EEPROM.commit();
        needPreenchimento = false;
      }
    }
  }

  if (needPreenchimento)
  {
    novaPessoa(matricula);
    preenchePonto(matricula);
  }
}

// Limpa toda a memória
void limpaTudo()
{
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, 0);
    EEPROM.commit();
  }
}

// void relatorioIndependente(char *matricula)
// {
//   int quantidadedePessoas = quantidadePessoas();
//   for (int i = 0; i < quantidadedePessoas; i++)
//   {
//     int posicao = pessoaEEPROM(i);
//     char buffMatricula[5];

//     for (int j = 0; j <= 3; j++)
//     {
//       buffMatricula[j] = EEPROM.read(posicao + j);
//     }
//     buffMatricula[4] = '\0';
//     Serial.println(matricula);
//     Serial.println(buffMatricula);

//     if (strcmp(matricula, buffMatricula) == 0)
//     {

//       Serial.println(buffMatricula);

//       char buffEntrada[11];
//       char buffSaida[11];

//       //Varredura da entrada
//       for (int j = 4; j <= 13; j++)
//       {
//         buffEntrada[j - 4] = EEPROM.read(posicao + j);
//       }
//       // Serial.print("Entrada: ");
//       buffEntrada[10] = '\0';
//       Serial.print("P");
//       Serial.println(buffEntrada);

//       for (int j = 14; j <= 23; j++)
//       {
//         buffSaida[j - 14] = EEPROM.read(posicao + j);
//       }
//       // Serial.print("Saida: ");
//       buffSaida[10] = '\0';
//       Serial.print("P");
//       Serial.println(buffSaida);
//     }
//   }

//   // int quantidadedePessoas = quantidadePessoas();
//   // for (int i = 0; i < quantidadedePessoas; i++)
//   // {
//   //   int posicao = pessoaEEPROM(i);
//   //   char buffMatricula[5];
//   //   char buffEntrada[11];
//   //   char buffSaida[11];

//   //   for (int j = 0; j <= 3; j++)
//   //   {
//   //     buffMatricula[j] = EEPROM.read(posicao + j);
//   //   }
//   //   // Serial.print("Matricula: ");
//   //   buffMatricula[4] = '\0';
//   //   Serial.println(buffMatricula);
//   //   for (int j = 4; j <= 13; j++)
//   //   {
//   //     buffEntrada[j - 4] = EEPROM.read(posicao + j);
//   //   }
//   //   // Serial.print("Entrada: ");
//   //   buffEntrada[10] = '\0';
//   //   Serial.print("P");
//   //   Serial.println(buffEntrada);

//   //   for (int j = 14; j <= 23; j++)
//   //   {
//   //     buffSaida[j - 14] = EEPROM.read(posicao + j);
//   //   }
//   //   // Serial.print("Saida: ");
//   //   buffSaida[10] = '\0';
//   //   Serial.print("P");
//   //   Serial.println(buffSaida);
//   // }
// }

void novaHoraData()
{
  String buffAno;
  String buffMes;
  String buffDia;
  String buffHora;
  String buffMin;

  buffAno = Serial.readStringUntil(',');
  buffMes = Serial.readStringUntil(',');
  buffDia = Serial.readStringUntil(',');
  buffHora = Serial.readStringUntil(',');
  buffMin = Serial.readStringUntil('#');

  Ano = atoi(buffAno.c_str());
  Mes = atoi(buffMes.c_str());
  Dia = atoi(buffDia.c_str());
  Hora = atoi(buffHora.c_str());
  Min = atoi(buffMin.c_str());

  mudaHoraPff = true;

  Serial.println(Ano);
  Serial.println(Mes);
  Serial.println(Dia);
  Serial.println(Hora);
  Serial.println(Min);

  // rtc.adjust(DateTime(2019, 9, 13, 18, 4, 0));
  //  D2019, 10, 10, 10, 10, 10 #
}
