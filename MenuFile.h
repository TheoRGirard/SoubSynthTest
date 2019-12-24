#ifndef MENUFILE_H_INCLUDED
#define MENUFILE_H_INCLUDED

#include <iostream>
#include <fstream>
#include <vector>
#include <Gpio.h>

#include "LCDscreen.h"



class Button
{

private:
  Gpio PinIn;
  bool Value;
  int LoopCounter;

public:

  bool State;
  Button(int NumGPIO);
  bool CheckState();

};


class Menu
{
  private:
    
    int Cursor;
    int ActivePresetCursor;
    //InstancePureData
    LCD *Ecran;
    std::string Dossier;

    void CreateListePreset();
    std::string getScreenFirstEntry();
    

  public:
	
	std::vector<std::string> ListePresets;
	
	void UpdateScreen();
    Menu(LCD *ecran, std::string Repertoire);
    void Scroll(bool upOrDown);
    void SelectPreset();
	std::string getActivePreset();

};






#endif // MENUFILE_H_INCLUDED
