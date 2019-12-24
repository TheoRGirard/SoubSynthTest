#ifndef LCD_H_INCLUDED
#define LCD_H_INCLUDED

#include <vector>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <math.h>
#include <Bela.h>
#include <Gpio.h>








class LCD
{

  private:

    //Attributs :
    Gpio PinE;
    Gpio PinRe;
    Gpio DataPin[8];
    

	public:

    void OutputChar(char Lettre);
    void OutputCommand(int Commande);
    void setEntryMode();
    void ShotEnable();
    void WaitForUnbusy(int delayInUs);
    void PrintChar(char Lettre);
    

  

    LCD(int pinRe, int pinE, int dataPin[]);
    void Clear();
    void setHome();
    void GoSecondLine();
    void Print(std::string ligne1, std::string ligne2);
    
    
    


};

#endif // LCD_H_INCLUDED
