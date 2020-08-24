/*
 *         SDRuno SMART-SWITCH David Whitty G4FEV
 *
 *  https://github.com/G4FEV/SDRUno-Smart-Switch
 *  
 *  This software is given freely for non-commercial use with no
 *  guarantee given or implied.
 *  SDRUno is a product of SDRPlay Ltd of which the author has no
 *  connection other than as a satisfied customer.
 *   
 *  SDRuno outputs frequency information in the format of the
 *  Kenwood CAT Command set. The Arduino sends a request to SDRuno 
 *  which responds with the information required.
 *  The frequency data is decoded and the user may edit the 
 *  Band-Limits below. The RF spectrum may be split into as 
 *  many as 14 outputs to switch Antennas or Filters as required.
 *
 *  The decoded data is displayed on a 20x4  I2C LCD Display.
*/

String ver ("Ver 1.03 Aug 2020"); //Version

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
/* We will use SoftwareSerial for the comms input on pins 8 and 9
 leaving the normal Arduino comms port for debugging information via
 the IDE Serial Monitor. (If you use the Serial Monitor uncomment the
 Delay line at the start of the loop or data scrolls too fast.)
 
 Beware, there are several versions of SoftwareSerial which may not
 be compatable.
*/
SoftwareSerial mySerial(8, 9); // Pin 8=RX, Pin 9=TX  TTL level CAT.
//Connect to SDRUno Com port via an     FTDI USB to TTL FT232RL converter
//                                      Connect 8 to FTDI Tx, 9 to FTDI Rx
LiquidCrystal_I2C lcd(0x3F, 20,4); //Set LCD I2C address
 //LiquidCrystal_I2C lcd(0x27, 20,4); //For older displays
 
 //If you are unsure of the LCD address load the I2c scanner.
       
/*/////////////////////////////////////////////
//////// Band Limits///////////////////////////
We have 14 spare pins (If we include unused Analogue pins)which may
all be used for switching an Antenna or Filter.
Here is where we set the frequency allocations for each band (A to N).

//They are in the form of     BandXLow    is the low fequency end
//                            BandXHi     is the High fequency end
              
If you edit this list take GREAT care to note how each band starts 
and finishes.
You may edit the   FREQUENCIES AND THE ANTENNA NAME.
i.e Band C starts at 1.600001 MHz and ends at 2.5 MHz
*/
//Band A
char AntennaA[] = " VLF Antenna ";  //Fill all 13 characters
long BandALow = 1000;              //1KHz  Band A     Low Limit
long BandAHi = 150000;            //150KHz  Band A     High Limit

//Band B
char AntennaB[] = " Medium Wave ";  //Fill all 13 characters
long BandBLow = 150001;           // Band B   Low Limit
long BandBHi = 1600000;          //1.6MHz Band B     High Limit

//Band C
char AntennaC[] = " LF Antenna  ";  //Fill all 13 characters
long BandCLow = 1600001;           // Band C   Low Limit
long BandCHi = 2500000;           //2.5 MHz Band C   High Limit
//Band D
char AntennaD[] = "80M Long Wire";  //Fill all 13 characters
long BandDLow = 2500001;           // Band C  Low Limit
long BandDHi = 4700000;           //4.7 MHz Band D  High Limit

//Band E
char AntennaE[] = " 40M Dipole  ";  //Fill all 13 characters
long BandELow = 4700001;           // Band E   Low Limit
long BandEHi = 7500000;           //7.5 MHz Band E   High Limit

//Band F
char AntennaF[] = " 20M Vertical";
long BandFLow = 7500001;          // Band F   Low Limit
long BandFHi = 14500000;          //14.5 MHz Band F   High Limit

//Band G
char AntennaG[] = " HF Antenna  ";
long BandGLow = 14500001;        // Band G   Low Limit
long BandGHi = 21500000;        //21.5 MHz   Band G   High Limit

//Band H
char AntennaH[] = "  HF Beam    ";  //Fill all 13 characters
long BandHLow = 21500001;          // Band H   Low Limit
long BandHHi =33000000;           //33 MHz     Band H   High Limit

//Band I
char AntennaI[] = "   6M Beam   ";
long BandILow = 33000001;         // Band I   Low Limit
long BandIHi = 56000000;         //56 MHz    Band I   High Limit

//Band J
char AntennaJ[] = " VHF Antenna ";  // Fill all 13 characters
long BandJLow = 56000001;         // Band J   Low Limit
long BandJHi = 150000000;        //150 MHz    Band J   High Limit

//Band K
char AntennaK[] = " UHF Antenna ";
long BandKLow = 150000001;       // Band K   Low Limit
long BandKHi = 999000000;       //999 MHz  Band K   High Limit

//Band L
char AntennaL[] = " SHF Discone ";  // Fill all 13 characters
long BandLLow = 999000001;         // Band L   Low Limit
long BandLHi = 1500000000;        //1500 MHz   Band L   High Limit

//Band M
char AntennaM[] = " Satcom Ant  ";
long BandMLow = 1500000001;      // Band M   Low Limit
long BandMHi = 1750000000;      //1750 MHz   Band M   High Limit

//Band N
char AntennaN[] = "  Satcom 2   ";
long BandNLow = 1750000001;      // Band N   Low Limit
long BandNHi = 2000000000;      //2000 MHz   Band N   High Limit

/////////////////////////////END OF BAND LIMITS//////////////////////

////////////////////Pin definition for Bands.//////////////////////
//  Pin 0 & 1 Reserved for Arduno Serial Monitor
//  Pin 8 & 9 Reserved for Comms to SDRuno

int BandA = 10;    // Digi pin 10
int BandB = 11;   // Digi pin 11
int BandC = 12;
int BandD = 13;
int BandE = 4;
int BandF = 5;
int BandG = 6;
int BandH = 7;  
int BandI = A0;     //Analogue 0
int BandJ = A1;    //Analogue 1
int BandK = A2;
int BandL = A3; 
int BandM = 2;     // Digi pin 2
int BandN = 3;    // Digi pin 3      
// variables
char inByte;  // gives hex FA  FB   etc
String buffer = ""; // Write the incomming data to String for temporary storage.
boolean stringComplete = false;  // Do we have all the data
String ModeString;
String FrequencyString ;

void setup()
{
 // lcd.begin();   // Old I2c Library
  lcd.init(); //Latest I2c Library
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("SDRuno Smart-Switch");
  lcd.setCursor(7, 1);
  lcd.print("G4FEV"); //Your name in lights here (For 1 Second anyway)

  lcd.setCursor(1, 2);
  lcd.print(ver);
  delay (1000);
  lcd.clear();

  mySerial.begin(9600);  // CAT input from PC on Pin 8 output on pin 9 to PC
  Serial.begin(9600);   // For watching Debug messages on Arduino Serial Monitor

  buffer.reserve(64); // reserve buffer for serial input. 64 is maximum but we don't really need all this

  pinMode(BandA, OUTPUT);
  pinMode(BandB, OUTPUT);
  pinMode(BandC, OUTPUT);
  pinMode(BandD, OUTPUT);
  pinMode(BandE, OUTPUT);
  pinMode(BandF, OUTPUT);
  pinMode(BandG, OUTPUT);
  pinMode(BandH, OUTPUT);
  pinMode(BandI, OUTPUT);
  pinMode(BandJ, OUTPUT);
  pinMode(BandK, OUTPUT);
  pinMode(BandL, OUTPUT);
  pinMode(BandM, OUTPUT);
  pinMode(BandN, OUTPUT);
  
}
void loop() {

  //delay(1000); //so we can read the Serial Monitor while debugging

  lcd.setCursor(1, 0);
  lcd.print("SDRuno Smart-Switch");

  //////////////////////////////GET THE SERIAL DATA ///////////////////////////////
  /*
   We send  IF; to SDRuno. 
   It responds with  IF0145500000   +00000000002100000; 
                         145.5 MHz              ||
                                                | VFO
                                             Mode  
  */
  //////////////////////////////////////////////////

  mySerial.print("IF;");   //Send IF;

  mySerial.flush();

  delay(50); //Hang about a bit

  if (mySerial.available() > 0 ) {   // get the first byte
    
    inByte = (char) mySerial.read();

                       //  Serial.print("inByte = ");
                       // Serial.println(inByte);

    // If the first byte is 'I' then we continue reading the data
    if (inByte == 'I') {

      buffer += inByte;
      while (mySerial.available() > 0) {
        inByte = (char) mySerial.read();

                         //  Serial.print(inByte);
        buffer += inByte;
    
        if (inByte == ';') {   // Stop when we see the ';' Character
                          //  Serial.println("");
          stringComplete = true;
        }
      }
    }
  }
 
  String buff_store = buffer; // Copy the buffer to buffer_store

  FrequencyString = buff_store.substring(3, 13);
                        //    Serial.print("FrequencyString = ");
                        //   Serial.println(FrequencyString);

  ModeString = buff_store.substring(30, 31);

  //////////////////////Convert string to int///////////////
  //We can't perform mathematical functions on a string so we must turn it into an integer
  ////////////////////////Convert Frequency string to an integer///////////////////////////////////
  //We do this one byte at a time

  int  i, len; // Length of string

long Frequency = 0; // Allows an integer up to 2147483647
          // Which we need for a 2 GHz frequency.

  len = FrequencyString.length(); //Get the length of the string
                        //   Serial.print("len = ");
                        //    Serial.println(len);
  for (i = 0; i < len; i++) {     //step through the conversion

                        //    Serial.print("i = ");
                        //    Serial.print(i);
                        //    Serial.print(" FrequencyString = ");
                        //    Serial.print(FrequencyString[i]);
                         //   Serial.print("   Frequency_build = ");
                        //    Serial.println(Frequency);

    Frequency = Frequency * 10 + (FrequencyString[i] - '0'); //do the conversion
  }

                        Serial.print("   Frequency = ");
                        Serial.println(Frequency);
            if (Frequency ==0){
lcd.setCursor(0, 1);
  lcd.print("  Waiting For Data  ");
                          //Serial.println("Waiting For Data");
 
  lcd.setCursor(0, 2); //Blank lines while waiting for data
    lcd.print("                    ");                        
      lcd.setCursor(0, 3);
        lcd.print("                    ");

                          

            }
  ///////  DISPLAY THE FREQUENCY IN THE SAME FORMAT AS SDRuno //////////
else{
  lcd.setCursor(0, 1);
  lcd.print("Freq: ");
}
 if (Frequency > 0){
  lcd.setCursor(18, 1);
  lcd.print(FrequencyString[9]);
  lcd.setCursor(17, 1);
  lcd.print(FrequencyString[8]);
  lcd.setCursor(16, 1);
  lcd.print(FrequencyString[7]);
  lcd.setCursor(15, 1);
  lcd.print(".");
  lcd.setCursor(14, 1);
  lcd.print(FrequencyString[6]);
  lcd.setCursor(13, 1);
  lcd.print(FrequencyString[5]);
  lcd.setCursor(12, 1);
  lcd.print(FrequencyString[4]);
  lcd.setCursor(11, 1);
  lcd.print(".");
  lcd.setCursor(10, 1);
  lcd.print(FrequencyString[3]);
  lcd.setCursor(9, 1);
  lcd.print(FrequencyString[2]);
  lcd.setCursor(8, 1);
  lcd.print(FrequencyString[1]);
  lcd.setCursor(7, 1);
  lcd.print(".");
  lcd.setCursor(6, 1);
  lcd.print(FrequencyString[0]);

  //////////  BLANK LEADING ZEROS LIKE SDRuno DISPLAY  ///////
  if (Frequency < 100000) {
    lcd.setCursor(12, 1);
    lcd.print(" ");
  }
  
   if (Frequency < 1000000) {
    lcd.setCursor(10, 1);
    lcd.print(" ");
    lcd.setCursor(11, 1);
    lcd.print(" ");
  }
  
   if (Frequency < 10000000) {
     lcd.setCursor(9, 1);
     lcd.print(" ");
  }
 
   if (Frequency < 100000000) {
    lcd.setCursor(8, 1);
    lcd.print(" ");
  }
 
   if (Frequency < 1000000000) {
    lcd.setCursor(6, 1);
    lcd.print(" ");
    lcd.setCursor(7, 1);
    lcd.print(" ");
  }
 
  if (Frequency < 10000) {
    lcd.setCursor(13, 1);
    lcd.print(" ");
  }

   if (Frequency > 0 && Frequency < 1000)
           {
    //There is a small cosmetic bug in SDRuno V1.33. if the frequency goes below
    //   1KHz and the user continues to scroll lower, SDRuno will continue
    //     to reduce the frequency of the CAT output below 1KHz
    //        We can fix that with this bodge which locks the frequency
    //          at 1KHz despite the input;-
    lcd.setCursor(14, 1);
    lcd.print("1");
    lcd.setCursor(15, 1);
    lcd.print(".");
    lcd.setCursor(16, 1);
    lcd.print("0");
    lcd.setCursor(17, 1);
    lcd.print("0");
    lcd.setCursor(18, 1);
    lcd.print("0");
  }     }
  ////////////////  DECODE the MODE & VFO (Just because we can)   /////////////////////////////////////
 ///////In some circumstance we may wish to swap antennas dependant on the selected Mode
 
  if ((buff_store[29] == '0'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: DSB ");
  }
  if ((buff_store[29] == '1'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: LSB ");
  }

  if ((buff_store[29] == '2'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: USB ");
  }

  if ((buff_store[29] == '3'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: CW  ");
  }
  if ((buff_store[29] == '4'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: FM  ");
  }
  if ((buff_store[29] == '5'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: AM  ");
  }
  if ((buff_store[29] == '6'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: FSK ");
  }
  if ((buff_store[29] == '7'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: CWr");
  }
  if ((buff_store[29] == '8'))
  {
    lcd.setCursor(0, 3);
    lcd.print("Mode: Digi");
  }
  ///////////////////////// VFO A or B //////////////////////////////
  if ((buff_store[30] == '0'))
  {
    lcd.setCursor(12, 3);
    lcd.print("VFO: A ");
  }

  if ((buff_store[30] == '1'))
  {
    lcd.setCursor(12, 3);
    lcd.print("VFO: B ");
  }

  ///////////////////////////////////END OF COMMS SECTION/////////////////////////////////////

  ///////////////////////////////////BAND OUTPUTS////////////////
  ////////////////////////////////// Band A ///////////////////

 if (Frequency > BandALow && Frequency <= BandAHi)
 
  {
                 /*
                   Serial.println("WE ARE IN Band A");
                   Serial.print("Freq Limits are: ");
                   Serial.print (BandALow);
                   Serial.print(" - ");
                   Serial.println(BandAHi);
               */  
    digitalWrite(BandA, HIGH);  //D10 Switch it HIGH
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);

    lcd.setCursor(0, 2);
    lcd.print("Band:A ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaA);
  }

  /////////////////////////////// Band B ///////////////////

  if (Frequency > BandBLow && Frequency <= BandBHi)
  {
                       /*
                         Serial.println("WE ARE IN Band B");
                         Serial.print("Freq Limits are: ");
                         Serial.print (BandBLow);
                         Serial.print(" - ");
                         Serial.println(BandBHi);
                     */ 
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, HIGH);    //D11 Switch it HIGH
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);

    lcd.setCursor(0, 2);
    lcd.print("Band:B ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaB);
  }

  //////////////////////////////// Band C ///////////////////

  if (Frequency > BandCLow && Frequency <= BandCHi)

  {
                      /*
                        Serial.println("WE ARE IN Band C");
                        Serial.print("Freq Limits are: ");
                        Serial.print (BandCLow);
                        Serial.print(" - ");
                        Serial.println(BandCHi);
                      */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, HIGH);//D12 Switch it HIGH
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);

    lcd.setCursor(0, 2);
    lcd.print("Band:C "); 
    lcd.setCursor(7, 2);
    lcd.print(AntennaC);
  }

  /////////////////////////////// Band D ///////////////////

  if (Frequency > BandDLow && Frequency <= BandDHi)
  {
                      /*
                        Serial.println("WE ARE IN Band D");
                          Serial.print("Freq Limits are: ");
                        Serial.print (BandDLow);
                        Serial.print(" - ");
                        Serial.println(BandDHi);
                      */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, HIGH);//D13 Switch it HIGH
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);
    lcd.setCursor(0, 2);
    lcd.print("Band:D ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaD);
  }

  //////////////////////////////// Band E ///////////////////

  if (Frequency > BandELow && Frequency <= BandEHi)
  {
                        /*
                          Serial.println("WE ARE IN Band E");
                          Serial.print("Freq Limits are: ");
                          Serial.print (BandELow);
                          Serial.print(" - ");
                          Serial.println(BandEHi);
                       */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, HIGH);//D4 Switch it HIGH
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);

    lcd.setCursor(0, 2);
    lcd.print("Band:E ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaE); 
  }

  //////////////////////////////// Band F ///////////////////

  if (Frequency > BandFLow && Frequency <= BandFHi)
  {
                         /*
                           Serial.println("WE ARE IN Band F");
                           Serial.print("Freq Limits are: ");
                           Serial.print (BandFLow);
                           Serial.print(" - ");
                           Serial.println(BandFHi);
                         */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, HIGH);//D5 Switch it HIGH
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);
    
    lcd.setCursor(0, 2);
    lcd.print("Band:F ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaF);
  }

  /////////////////////////////// Band G ///////////////////

  if (Frequency > BandGLow && Frequency <= BandGHi)
  {
                     /*
                       Serial.println("WE ARE IN Band G");
                       Serial.print("Freq Limits are: ");
                       Serial.print (BandGLow);
                       Serial.print(" - ");
                       Serial.println(BandGHi);
                     */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, HIGH);//D6 Switch it HIGH
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);
    
    lcd.setCursor(0, 2);
    lcd.print("Band:G ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaG);
  }

  ////////////////////////////// Band H ///////////////////

  if (Frequency > BandHLow && Frequency <= BandHHi)
  {
                  /*
                   Serial.println("WE ARE IN Band H");
                   Serial.print("Freq Limits are: ");
                   Serial.print (BandHLow);
                   Serial.print(" - ");
                   Serial.println(BandHHi);
                  */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, HIGH);//D7 Switch it HIGH
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);
    
    lcd.setCursor(0, 2);
    lcd.print("Band:H ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaH);
  }

  ////////////////////////////// Band I ///////////////////

  if (Frequency > BandILow && Frequency <= BandIHi)
  {
                      /*    
                           Serial.println("WE ARE IN Band I");
                           Serial.print("Freq Limits are: ");
                           Serial.print (BandILow);
                           Serial.print(" - ");
                           Serial.println(BandIHi);
                    */     
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, HIGH);//A0 Switch it HIGH
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);
    lcd.setCursor(0, 2);
    lcd.print("Band:I ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaI);
  }

  /////////////////////////////// Band J ///////////////////

  if (Frequency > BandJLow && Frequency <= BandJHi)
  {
                   /*
                     Serial.println("WE ARE IN Band J");
                     Serial.print("Freq Limits are: ");
                     Serial.print (BandJLow);
                     Serial.print(" - ");
                     Serial.println(BandJHi);
                  */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, HIGH);//A1 Switch it HIGH
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);

    lcd.setCursor(0, 2);
    lcd.print("Band:J ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaJ);
  }

  ////////////////////////////// Band K ///////////////////

  if (Frequency > BandKLow && Frequency <= BandKHi)
  {
                 /*
                  Serial.println("WE ARE IN Band K");
                  Serial.print("Freq Limits are: ");
                  Serial.print (BandKLow);
                  Serial.print(" - ");
                  Serial.println(BandKHi);
                */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, HIGH);// A2 Switch it HIGH
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);
    
    lcd.setCursor(0, 2);
    lcd.print("Band:K ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaK);
  }

  /////////////////////////////// Band L ///////////////////

  if (Frequency > BandLLow && Frequency <= BandLHi)
  {
                /*
                 Serial.println("WE ARE IN Band L");
                 Serial.print("Freq Limits are: ");
                 Serial.print (BandLLow);
                 Serial.print(" - ");
                 Serial.println(BandLHi);
               */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, HIGH);//A3 Switch it HIGH
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, LOW);
 
    lcd.setCursor(0, 2);
    lcd.print("Band:L ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaL);
  }

  /////////////////////////////// Band M ///////////////////

  if (Frequency > BandMLow && Frequency <= BandMHi)
  {
                   /*
                    Serial.println("WE ARE IN Band M");
                    Serial.print("Freq Limits are: ");
                    Serial.print (BandMLow);
                    Serial.print(" - ");
                    Serial.println(BandMHi);
                  */
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, HIGH);//D2 Switch it HIGH
    digitalWrite(BandN, LOW);
    
    lcd.setCursor(0, 2);
    lcd.print("Band:M ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaM);
  }
 /////////////////////////////// Band N ///////////////////

  if (Frequency > BandNLow && Frequency <= BandNHi)
  {
                      /*
                        Serial.println("WE ARE IN Band N");
                        Serial.print("Freq Limits are: ");
                        Serial.print (BandNLow);
                        Serial.print(" - ");
                        Serial.println(BandNHi);
                     */ 
    digitalWrite(BandA, LOW);
    digitalWrite(BandB, LOW);
    digitalWrite(BandC, LOW);
    digitalWrite(BandD, LOW);
    digitalWrite(BandE, LOW);
    digitalWrite(BandF, LOW);
    digitalWrite(BandG, LOW);
    digitalWrite(BandH, LOW);
    digitalWrite(BandI, LOW);
    digitalWrite(BandJ, LOW);
    digitalWrite(BandK, LOW);
    digitalWrite(BandL, LOW);
    digitalWrite(BandM, LOW);
    digitalWrite(BandN, HIGH);  //D3 Switch it HIGH

    lcd.setCursor(0, 2);
    lcd.print("Band:N ");
    lcd.setCursor(7, 2);
    lcd.print(AntennaN);
  }

  /////////////////          END OF BAND SWITCHING       ////////
  ////////////////FLUSH THE BUFFER AND GET READY TO START AGAIN/////////

  if (stringComplete) {

    buffer = "";      // Clears the buffer

    stringComplete = false;

  }

            //   Serial.println("             ");  // Blank lines to seperate Debug Messages
            //   Serial.println("-------------");  // Blank lines to seperate Debug Messages
  
}
/*
And that's how we do that. David Whitty. G4FEV 2020 Lockdown Project.
*/


//Nothing else down here.
