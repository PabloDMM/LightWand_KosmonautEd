/*
LightWand Kosmonaut Edition

Code based on Michael Ross original LightWand(2014) first wirtten in 2010 and enhaced by Is0-Mick in 2012.
Apart from the main structure, the following functions have been literally borrowed from the original code:
   setupSDcard()
   GetFileNamesFromSD(File dir)

http://mrossphoto.com/wordpress32/

This code has been written by Pablo de Miguel (pablodmm.isp@gmail.com) in 2015.

This code is intended to run with LightWand Kosmonaut Edition Controller, All the information about this
controller can be find in:
https://github.com/PabloDMM/LightWand_KosmonautEd

This code support direct reading of .pnm RAW files stored in the root directory of a microSD-Card
*/

// Libraries needed
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Adafruit_NeoPixel.h>

// Definition of LCD Pins
#define LCD_SCK 7
#define LCD_DIN 6
#define LCD_DC 5
#define LCD_CS 4
#define LCD_RST 3
#define LCD_BL 13

// Definition of SD Pins (Just for Info, already set in the SD library)
#define SD_MOSI 51
#define SD_MISO 50
#define SD_CLK 52
#define SD_CS 44

// Definition of SW pins and BUZZER
#define SWActive 8       // Activation Switch PIN
#define SWMenuUP 10      // Menu Up Switch PIN
#define SWMenuDW 11      // Menu Down Switch PIN
#define SWValUP 9        // Value Up Switch PIN
#define SWValDW 12       // Value Down Switch PIN
#define SWLight 15       // Light Switch PIN
#define SWStop 14        // Stop Emission Switch PIN
#define BUZZ 16          // Buzzer PIN


#define LED_COUNT 144    // Definition of LED strip longitude (144 for the WS2812 RGB LED strips)

// Definition of Parameters
#define INIMelodyNote 4   //Number of Notes in Start Melody
#define ENDMelodyNote 4   //Number of Notes in End Melody
#define STOPMelodyNote 4  //Number of Notes in Stop Melody
#define INIFREQ 500       //Initial Frequency for the Progressive Buzzer Mode
#define INIDELAY 1000     //Emission Initial Delay


const int chipSelect = 4; //SD card chipSelect PIN
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);  //LCD Display Declaration
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, 2, NEO_RGB + NEO_KHZ800);         //LED Strip Declaration  

// Flags for the Pulse Conformer
boolean SWActive_p = false;
boolean SWMenuUP_p = false;
boolean SWMenuDW_p = false;
boolean SWValUP_p = false;
boolean SWValDW_p = false;
boolean SWLight_p = false;
boolean SWMode_p = false;
boolean Light = true;
boolean ValDW=false;
boolean ValUP=false;

File root;
File dataFile;
String m_CurrentFilename = "";        //Root Filename
int m_FileIndex = 0;                  //Variable for the FileIndex  
int m_NumberOfFiles = 0;              //Varialbe for the number of  files in the SD Storage
String m_FileNames[200];              //Variable for the File names(Max 200)
int Bright = 5;                       //Predetermined Brigthness
int Delay = 5;                        //Predetermined Delay
bool Active = 0;                      //Active Option
int MenuIndex = 0;                    //Variable for the MenuIndex
int BrightLimit = 0;                  //Variable for the Maximum BrithLimit
int FreqTone = 0;                     //Variable for the Frequenzy Tone emitted 
int BuzzMode = 1;                     //Predetermined BuzzMode
int iniMelody[INIMelodyNote][2]={{2093,500},{1568,500},{0,300},{1319,600}}; //Start Melody
int endMelody[ENDMelodyNote][2]={{2093,500},{1568,500},{0,300},{1319,600}}; //End Melody
int stpMelody[STOPMelodyNote][2]={{1568,500},{1319,500},{0,300},{2093,600}}; //Stop Melody

void setup()
{
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  digitalWrite(43,HIGH);
  setupLCDdisplay();
  setupSDcard();
  strip.begin();
  strip.show();
}

void loop()
{
   DisplayControl();   
   SwitchControl();
   if(Active){DisplayStrip();};
   delay(100);
}

void setupLCDdisplay() {
  display.begin();
  display.setContrast(40);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.display();          //Show SplashScreen
  delay(500);
  display.clearDisplay();
  display.println("LightWand");
  display.println("KOSMONAUT ED");
  display.println("INI SD card...");
  display.display(); 
}

void setupSDcard() {
  pinMode(SD_CS, OUTPUT);
  // SD Initialization Routine
  if (!SD.begin(chipSelect)) 
  {
    display.clearDisplay();
    display.println("LightWand");
    display.println("KOSMONAUT ED");
    display.println("INI SD FAILED");
    display.display();
    return;
  }
  display.clearDisplay();
  display.println("LightWand");
  display.println("KOSMONAUT ED");
  display.println("INI SD DONE");
  display.display();
  // Files Scan
  root = SD.open("/");
  display.clearDisplay();
  display.println("LightWand CAT");
  display.println("FILE SCAN");
  display.display();
  delay(500);
  GetFileNamesFromSD(root);
  isort(m_FileNames, m_NumberOfFiles);
  m_CurrentFilename = m_FileNames[0];
  DisplayControl();
}

void GetFileNamesFromSD(File dir) {
  int fileCount = 0;
  String CurrentFilename = "";
  while(1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      m_NumberOfFiles = fileCount;
    entry.close();
      break;
    }
    else {
      if (entry.isDirectory()) {
        //GetNextFileName(root);
      }
      else {
        CurrentFilename = entry.name();
        if (CurrentFilename.endsWith(".pnm") || CurrentFilename.endsWith(".PNM")) { //find files with our extension only
          m_FileNames[fileCount] = entry.name();
          fileCount++;
        }
      }
    }
    entry.close();
  }
}

void DisplayControl() {
  m_CurrentFilename = m_FileNames[m_FileIndex];
  display.clearDisplay();
  display.println("LightWand");
  
  if(MenuIndex==0){display.setTextColor(WHITE, BLACK);}
  else{display.setTextColor(BLACK, WHITE);}
  display.print("Bright: ");
  display.print(Bright);
  display.println(" %");
  
  if(MenuIndex==1){display.setTextColor(WHITE, BLACK);}
  else{display.setTextColor(BLACK, WHITE);}
  display.print("Delay: ");
  display.print(Delay);
  display.println(" ms");
  
  if(MenuIndex==2){display.setTextColor(WHITE, BLACK);}
  else{display.setTextColor(BLACK, WHITE);}
  display.println("Current File:");
  display.println(m_CurrentFilename);
  display.setTextColor(BLACK, WHITE);
  
  if(MenuIndex==3){display.setTextColor(WHITE, BLACK);}
  else{display.setTextColor(BLACK, WHITE);}
  display.print("Buzz: ");
  switch(BuzzMode)
  {
    case 0:
      display.println("OFF");
      break;
    case 1:
      display.println("LIM BEAT");
      break;
    case 2:
      display.println("PROGR");
      break;
  }
  display.setTextColor(BLACK, WHITE);
  display.display();
}

void isort(String *filenames, int n) {
  for (int i = 1; i < n; ++i) {
    String j = filenames[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < filenames[k]); k--) {
      filenames[k + 1] = filenames[k];
    }
    filenames[k + 1] = j;
  }
}

void DisplayStrip()
{
  // Set of Option Variables
  BrightLimit = map(Bright,0,100,0,255);      // Sets the maximum Brightness value
  digitalWrite(LCD_BL, LOW);                  //LCD Backlight Control OFF to avoid it appearing in the image;
  Light=false;                                //LCD Backlight Control OFF;
  if(BuzzMode!=0){for(int z=0; z<INIMelodyNote; z++){tone(BUZZ,iniMelody[z][0],iniMelody[z][1]);}} // Initial melody Play
  FreqTone=INIFREQ;                           // Initial Frequency Set
  delay(INIDELAY);                            // Initial Delay
  
  char temp[14];
  m_CurrentFilename.toCharArray(temp,14);
  File dataFile = SD.open(temp);
  // Rutine to avoid the header of the file
  int countRet=0; // Number of Carriage Return counted
  while(countRet < 4)
  {
    if(dataFile.read()=='\n'){countRet++;}
  }
  while(dataFile.available())
  {
    // Stop Control
    if(digitalRead(SWStop))
    {
      Active=0;
      for(int j=0; j<LED_COUNT; j++){  strip.setPixelColor(j,0,0,0);}
      strip.show();
      if(BuzzMode!=0){for(int z=0; z<STOPMelodyNote; z++){tone(BUZZ,stpMelody[z][0],stpMelody[z][1]);}} // Stop Melody Play
      return;
    }
    // Control of progressive beat (Only emmitted in Buzz Progressive Mode)
    if(BuzzMode==2){
       FreqTone=FreqTone+5;
        noTone(BUZZ);
        tone(BUZZ,FreqTone);
    }
    // File Row Read and Display
    for(int j=0; j<LED_COUNT; j++)
    {  
      int red = dataFile.read();
      int green = dataFile.read();
      int blue = dataFile.read();
      red = map(red,0,255,0,BrightLimit);     // Bright Adjustment
      green = map(green,0,255,0,BrightLimit); // Bright Adjustment
      blue = map(blue,0,255,0,BrightLimit);   // Bright Adjustment
      strip.setPixelColor(j, green,red,blue);
    }
    delay(Delay);
    strip.show();
  }  
  // File Close
  dataFile.close();
  Active=0;
  // LED Strip OFF
  for(int j=0; j<LED_COUNT; j++){  strip.setPixelColor(j,0,0,0);}
  strip.show();
  
  if(BuzzMode!=0){for(int z=0; z<ENDMelodyNote; z++){tone(BUZZ,endMelody[z][0],endMelody[z][1]);}} // End Melody Play  
}

void SwitchControl()
{
  // Active Switch Control
  if(digitalRead(SWActive)){
    if(!SWActive_p)
    {
      SWActive_p=true;
      Active=1;
    }
  }else{
    if(SWActive){SWActive_p=false;}
  }
  
  // Menu Index Control
  if(digitalRead(SWMenuUP)){
    if(!SWMenuUP_p)
    {
      SWMenuUP_p=true;
      if(MenuIndex>0){MenuIndex--;}
    }
  }else{
    if(SWMenuUP){SWMenuUP_p=false;}
  }
  
  if(digitalRead(SWMenuDW)){ 
    if(!SWMenuDW_p){
      SWMenuDW_p=true;
      if(MenuIndex<3){MenuIndex++;}
    }
  }else{
    if(SWMenuDW){SWMenuDW_p=false;}
  }
  
  // Light Control
  if(digitalRead(SWLight)){ 
    if(!SWLight_p){
      SWLight_p=true;
      Light=!Light;
    }
  }else{
    if(SWLight){SWLight_p=false;}
  }
  digitalWrite(LCD_BL, Light); //LCD Backlight Control
  
  // Value Control
  if(digitalRead(SWValUP)){ 
    if(!SWValUP_p){
      SWValUP_p=true;
      ValUP=true;
    }
  }else{
    if(SWValUP){SWValUP_p=false;}
  }
  
  if(digitalRead(SWValDW)){ 
    if(!SWValDW_p){
      SWValDW_p=true;
      ValDW=true;
    }
  }else{
    if(SWValDW){SWValDW_p=false;}
  }
  
  switch(MenuIndex){
  case 3: // BuzzMode Control
    if(ValUP){if(BuzzMode<2){BuzzMode++;}}
    if(ValDW){if(BuzzMode>0){BuzzMode--;}}
    break;
  case 2: // File Control
    if(ValUP){if(m_FileIndex<(m_NumberOfFiles-1)){m_FileIndex++;}else{m_FileIndex=0;}}
    if(ValDW){if(m_FileIndex>0){m_FileIndex--;}else{m_FileIndex=(m_NumberOfFiles-1);}}
    break;
  case 1: // Delay Control
    if(ValUP){if(Delay<100){Delay=Delay+5;}}
    if(ValDW){if(Delay>5){Delay=Delay-5;}}
    break;
  case 0: // Bright Control
    if(ValUP){if(Bright<100){Bright=Bright+5;}}
    if(ValDW){if(Bright>5){Bright=Bright-5;}}
    break;
  }
  ValUP=false;
  ValDW=false;
}
