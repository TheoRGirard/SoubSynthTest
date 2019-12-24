#include "MenuFile.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

//Classe Menu --------------------------------------------------

Menu::Menu(LCD *ecran, string Repertoire)
{
Cursor = 0;
ActivePresetCursor = -1;
Ecran = ecran;
Dossier = Repertoire;

CreateListePreset();

}



void Menu::CreateListePreset()
{
	fstream monFlux(Dossier.c_str());  //Ouverture d'un fichier en lecture
	
	if(monFlux)
	{
	
	  string ligne;
	
	  while(getline(monFlux, ligne)) //Tant qu'on n'est pas à la fin, on lit
	  {
	    if(ligne.length() > 15)
	    {
	      ListePresets.push_back(ligne.substr(0,15));
	    }
	    else
	    {
	      ListePresets.push_back(ligne);
	    }
	     //Et on l'affiche dans la console
	     //Ou alors on fait quelque chose avec cette ligne
	     //À vous de voir
	  }
	}
	else
	{
	    cout << "ERREUR: Impossible d'ouvrir le fichier en lecture." << endl;
	}
	
}



string Menu::getActivePreset()
{
	return(ListePresets[ActivePresetCursor].substr(1,ListePresets[ActivePresetCursor].length()-1));
}



string Menu::getScreenFirstEntry()
{
	return(ListePresets[Cursor]);
}



void Menu::UpdateScreen()
{
	Ecran->Print( (string) ListePresets[Cursor], (string) ListePresets[(Cursor + 1) % ListePresets.size()]);
}



void Menu::Scroll(bool upOrDown)
{
	if(upOrDown)
	{
	  Cursor ++;
	  Cursor = Cursor%(ListePresets.size());
	}
	else
	{
	  Cursor --;
	  Cursor = (Cursor+ListePresets.size())%(ListePresets.size());
	}
}



void Menu::SelectPreset()
{
	if(ActivePresetCursor != -1)
	{
	  ListePresets[ActivePresetCursor] = ListePresets[ActivePresetCursor].substr(1,15);
	}
	
	ActivePresetCursor = Cursor;
	ListePresets[ActivePresetCursor] = ">"+ListePresets[ActivePresetCursor];
}






int const EnableKeyRepeat = 1000;
int const KeyRepeatPeriod = 500;

//Classe Bouton --------------------------------------------------------


Button::Button(int NumGPIO)
{
	PinIn.open(NumGPIO, INPUT);
	LoopCounter = 0;
	State = 0;
	Value = 0;
}

bool Button::CheckState()
{
	Value = PinIn.read();
	
	if(Value) //Si le GPIO est up
	{
	  LoopCounter ++; //On compte un tour de boucle appuyé
	
	  if(LoopCounter >= EnableKeyRepeat)
	  {
	    if(LoopCounter % KeyRepeatPeriod == 0)
	    {
	      State = 1;
	    }
	    else
	    {
	      State = 0;
	    }
	  }
	}
	else //Si le GPIO est down
	{
	  if(LoopCounter > 0)//Si on sort d'un appui
	  {
	  	
	    if(LoopCounter < EnableKeyRepeat) //C'était un appui court
	    {
	      State = 1;
	      
	    }
	    else //Si c'était un appui long
	    {
	      State = 0;
	      
	    }
	
	    LoopCounter = 0; //On reinitialise le compteur
	  }
	  else //Cas où rien n'a été appuyé
	  {
	  	
	    State = 0;
	  }
	}
	return(State);
}

