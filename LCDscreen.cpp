#include "LCDscreen.h"
#include <vector>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <math.h>
#include <Bela.h>
#include <Gpio.h>


//Test :
#include <iostream>

using namespace std;


void LCD::OutputChar(char Lettre)
{
  uint8_t Binaire(Lettre);

  for(int i(0); i<8; i++)
  {
    uint8_t masque(pow(2,i));//on genere 0001000 a la ieme position
    bool Value(Binaire & masque); //Value prend la valeur booleene du ieme bit
    DataPin[i].write(Value);//On update le bon pin à la bonne valeur.
  }

}

void LCD::OutputCommand(int Commande)
{
  uint8_t Binaire(Commande);

  for(int i(0); i<8; i++)
  {
    uint8_t masque(pow(2,i));//on genere 0001000 a la ieme position
    bool Value(Binaire & masque); //Value prend la valeur booleene du ieme bit
    DataPin[i].write(Value);//On update le bon pin à la bonne valeur.
  }
}

void LCD::PrintChar(char Lettre)
{

  PinRe.write(1);
  OutputChar(Lettre);
  ShotEnable();
  WaitForUnbusy(10000);
}

void LCD::ShotEnable()
{
  PinE.write(0);
  //asm volatile ("nop");
  //asm volatile ("nop");
   WaitForUnbusy(50000);
  PinE.write(1);
}

void LCD::WaitForUnbusy(int delayInUs)
{
  usleep(delayInUs);
}

void LCD::setEntryMode()
{
  PinRe.write(0);
  OutputCommand(56);
  ShotEnable();
  WaitForUnbusy(15000);
  
  PinRe.write(0);
  OutputCommand(15);
  ShotEnable();
  WaitForUnbusy(15000);
  
  PinRe.write(0);
  OutputCommand(6);
  ShotEnable();
  WaitForUnbusy(15000);
}


LCD::LCD(int pinRe, int pinE, int dataPin[])
{

  for(int i(0); i < 8; i++)
  {
    DataPin[i].open(dataPin[i], OUTPUT);
  }

  PinE.open(pinE, OUTPUT);
  PinRe.open(pinRe, OUTPUT);
  PinE.write(1);
  
  setEntryMode();

}

void LCD::Clear()
{
  PinRe.write(0);
  OutputCommand(1);
  ShotEnable();
  WaitForUnbusy(1500);
}

void LCD::setHome()
{
  PinRe.write(0);
  OutputCommand(2);
  ShotEnable();
  WaitForUnbusy(1500);
}

void LCD::GoSecondLine()
{
  PinRe.write(0);
  OutputCommand(128+40);
  ShotEnable();
  WaitForUnbusy(1500);
}

void LCD::Print(string ligne1, string ligne2)
{
  Clear();
  setHome();
  for(int i(0); i < min(16,ligne1.length()); i++)
  {
  	PrintChar((char) ligne1[i]);
  }
  GoSecondLine();
  for(int i(0); i < min(16,ligne2.length()); i++)
  {
  	PrintChar((char) ligne2[i]);
  }
  
}

