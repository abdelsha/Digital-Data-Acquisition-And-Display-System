#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "SCI.h"//modified baud rate 
#include <math.h>

void setClk(void);
void delayby1ms(int);
void ledflash(int);
void delayby1msv2(int);
void ledflash2(int);
void timerinterrupt (void);
void OutCRLF(void);
void serialtrans (void);
void ledprogress(void);
void calc(unsigned short s);
void binarytobcd(void);
//void binarytobcd(void);
int ix=0;
int i=0;
int t=0;
int k=0;
int m=0;
int MSD;
int LSD;
int MSDD;
int fat=0;
char string[20];
unsigned short n; 
int count=1; 
unsigned short Val1;
unsigned int Temp;
unsigned short val2;
unsigned int output = 0;
unsigned short input;
signed char a;
int flag=1;


void main(void) {
  //****************************************************** Port Configuration *************************************************//
  
  DDRJ = 0x01;          // Set pin D13 (on board LED) to an output
  PTJ = 0x00;
  DDRP=0xFF;
  PTP=0x00;           // Make sure the LED isn't on to begin with, since that would be confusing
  
  // Configure PAD0-PAD7 Pins
    DDR1AD = 0b11111111110111111;        // Configure all of the AD ports except port 6 as outputs
    ATDDIEN = 0xFFBF;     // Configure all of the AD ports except port 6 as digital outputs 
    //PER1AD = 0x40;        // Enable pull-up registers for input pints (A0-A3)
  
  SCI_Init(57600);
  
//****************************************************** IRQ Intterrupts setting *************************************************//

  //IRQCR=0b11000000;  // PT1 CONFIGURED FOR FALLING EDGE. THIS TAKES PROIORITY.  
  
//****************************************************** timer interrupts  *************************************************//
   TSCR1 = 0x90;    //Timer System Control Register 1
                    // TSCR1[7] = TEN:  Timer Enable (0-disable, 1-enable)
                    // TSCR1[6] = TSWAI:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[5] = TSFRZ:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[4] = TFFCA:  Timer Fast Flag Clear All (0-normal 1-read/write clears interrupt flags)
                    // TSCR1[3] = PRT:  Precision Timer (0-legacy, 1-precision)
                    // TSCR1[2:0] not used

  TSCR2 = 0x04;    //Timer System Control Register 2
                    // TSCR2[7] = TOI: Timer Overflow Interrupt Enable (0-inhibited, 1-hardware irq when TOF=1)
                    // TSCR2[6:3] not used
                    // TSCR2[2:0] = Timer Prescaler Select: See Table22-12 of MC9S12G Family Reference Manual r1.25 (set for bus/1)
  
                    
  TIOS = 0xFC;     //Timer Input Capture or Output capture
                    //set TIC[0] and input (similar to DDR)
  PERT = 0x03;     //Enable Pull-Up resistor on TIC[0]

  TCTL3 = 0x00;    //TCTL3 & TCTL4 configure which edge(s) to capture
  TCTL4 = 0x0A;    //Configured for falling edge on TIC[0]

/*
 * The next one assignment statement configures the Timer Interrupt Enable                                                   
 */           
   
  TIE = 0x03;      //Timer Interrupt Enable 
//****************************************************** Intterrupts enable *************************************************//
 
  
 //****************************************************** Main portion *************************************************//
 
 //****************************************************** clock set *************************************************//  	
	setClk(); //clock set to 8MGHz
	//****************************************************** ADC RESOLUTION *************************************************//

	ATDCTL1 = 0x3F;//0b00101111		// set for 10-bit resolution AND CHANNEL AN6
	//ATDCTL2=0x00
	ATDCTL3 = 0x88;		// right justified, only one channel per sequence
	ATDCTL4 = 0x67;	// prescaler = 7; ATD clock = 8.0MHz / (2 * (1 + 1)) == 2MHz (calulation). sample time select= ATDCLK/4= 500kHZ(sampling time)
	ATDCTL5 = 0x26;		// continuous conversion on channel 6( if we want  multiple channels then we do 	ATDCTL5 = 0x36;) 
  //ATDDRn stores the conversion results 
  //PTT=0b11111111;
  EnableInterrupts;
  for(;;) {
    //ledflash(10);
    //ledflash2(10);
    //ledflash(10);
  } /* loop forever */
  /* please make sure that you never leave main */
} 

 //****************************************************** Set Clock *************************************************//

void setClk(void){
  //VCLOCk=Fvco-2*vref*(syndiv+1)
  CPMUPROT = 0x26;               //Protection of clock configuration is disabled
  CPMUCLKS = 0x80;//vref=1 PLLSEL=1
   
  CPMUSYNR=0x07;     //VCOFRQ=0,SYNDIV=7
  CPMUFLG=0x01;
  //CPMUPLL=0x01;
  CPMUPOSTDIV=0x00;
  CPMUPLL=0x01;
  
  //CPMUREFDIV = initREFDV+REFFRQ;//set PLL divider (0x80 = 10 000000)
  //CPMUPOSTDIV=0x00;           // set Post Divider
  CPMUOSC = 0x00;               // Enable external oscillator
 // while (CPMUFLG_LOCK == 0) {}  // wait for it
  //CPMUPLL = CPMUCLKS;           // Engage PLL to system
}

 //****************************************************** LED FLASH *************************************************//

void ledflash(int n){
  for(i=0;i<n;i++){
   PTJ=0x01;
   delayby1ms(1000);
   PTJ=0x00;
   delayby1ms(1000);
   
  }
}
//****************************************************** LED FLASH *************************************************//

void ledflash2(int n){
  for(i=0;i<n;i++){
   PTJ=0x01;
   delayby1msv2(1000);
   PTJ=0x00;
   delayby1msv2(1000);
   
  }
}

 //****************************************************** DelayBy1ms *************************************************//

void delayby1ms(int k){
  TSCR1= 0x90; //enable timer and fast timer flag clear
  TSCR2=0x00;//Disable timer interrupt
  TIOS|= 0x01; //enable OC0
  TC0=TCNT +8000;
  for(ix=0;ix<k;ix++){
    while(!(TFLG1_C0F));
    TC0+=8000;
  }
  TIOS &=-0x01;//DISABLE OC0
  timerinterrupt();// restoring the timeinterrut values
}


 //****************************************************** DelayBy1ms *************************************************//

void delayby1msv2(int k){
  TSCR1= 0x90; //enable timer and fast timer flag clear
  TSCR2=0x07;//Disable timer interrupt
  TIOS|= 0x01; //enable OC0
  TC0=TCNT; 
  for(ix=0;ix<k;ix++){
    while(!(TFLG1_C0F));
  }
  TIOS &=-0x01;//DISABLE OC0
  timerinterrupt();// restoring the timeinterrut values
}



 //****************************************************** Interrupt first Button setup 1 *************************************************//
interrupt VectorNumber_Vtimch0 void Vtimch(void){
   //unsigned int temp; //DON'T EDIT THIS
  /* DECLARE ALL YOUR LOCAL VARIABLES ABOVE*/   
  
    /* YOUR CODE GOES BELOW*/             
  ////////////////////////////////////////////////////
     serialtrans();
     Temp=TC0;
   

}
//****************************************************** Interrupt second Button setup 2 *************************************************//
interrupt VectorNumber_Vtimch1 void Vtimch01(void){
     
    
    if (flag==1){
     ledprogress();
     flag++;
    }
    if (flag==2){
      fat=1;
      binarytobcd(); 
   }
   PT1AD=0x0000;
   PTP=0x00;
   flag++;
   if (flag>2){
    
    flag=1;
   }
   delayby1ms(200);
   Temp=TC1;
}
  

//****************************************************** OUTCRLF *************************************************// 
void OutCRLF(void){
  SCI_OutChar(CR);
  SCI_OutChar(LF);
  PTJ ^= 0x20;          // toggle LED D2
}

//****************************************************** Interrupt One Button setup 1 *************************************************//
 void timerinterrupt (void){
    TSCR1 = 0x90;    //Timer System Control Register 1
                    // TSCR1[7] = TEN:  Timer Enable (0-disable, 1-enable)
                    // TSCR1[6] = TSWAI:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[5] = TSFRZ:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[4] = TFFCA:  Timer Fast Flag Clear All (0-normal 1-read/write clears interrupt flags)
                    // TSCR1[3] = PRT:  Precision Timer (0-legacy, 1-precision)
                    // TSCR1[2:0] not used

  TSCR2 = 0x04;    //Timer System Control Register 2
                    // TSCR2[7] = TOI: Timer Overflow Interrupt Enable (0-inhibited, 1-hardware irq when TOF=1)
                    // TSCR2[6:3] not used
                    // TSCR2[2:0] = Timer Prescaler Select: See Table22-12 of MC9S12G Family Reference Manual r1.25 (set for bus/1)
  
                    
  TIOS = 0xFC;     //Timer Input Capture or Output capture
                    //set TIC[0] and input (similar to DDR)
  PERT = 0x03;     //Enable Pull-Up resistor on TIC[0]

  TCTL3 = 0x00;    //TCTL3 & TCTL4 configure which edge(s) to capture
  TCTL4 = 0x0A;    //Configured for falling edge on TIC[0]

/*
 * The next one assignment statement configures the Timer Interrupt Enable                                                   
 */           
   
  TIE = 0x03;      //Timer Interrupt Enable
 }
//****************************************************** Serial transmission function **********************//
 void serialtrans (void){
   delayby1ms(250);
   PTJ^= 0x01;
   //delayby1ms(250);
   SCI_OutString("fuck off"); 
   OutCRLF();
   m=1;
   while (m){
    count++;
    Val1 = ATDDR0 ; 
    //SCI_OutUDec(Val1);
    //OutCRLF();
    calc (Val1); 
    if (val2>6000){
      val2=0;
    }
    if (val2>90 & val2<6000){
      val2=90;
    }
    SCI_OutUDec(val2);
    OutCRLF();
     
    //delayby1ms(205); 
    m=PTT&0b0000001;  //mask, this will determine if the button is pressed again
    delayby1ms(50); 
   }
   count=0;
   delayby1ms(250);
   SCI_OutString("shit"); 
   OutCRLF();
 }
 void calc (unsigned short s){
     if (s<=539){
      val2=((56*s)/100)-282;
     }
     
     else if (s<=617 & s>539){
      
      val2=((79*s)/100)-410;
     } 
     else if(s>617){
       val2=(5*s)-3010;
     }
 }
//****************************************************** linear light mode **********************// 
 void ledprogress(void){
      delayby1ms(250);
    t=1;
    //PT1AD=0x0010;
    //PTP=0x1111;
    while (t){
    count++;
    //PT1AD=0x0010;
    Val1 = ATDDR0 ;
    calc (Val1);
    if (val2>100){
      val2=0;
    }
    if (val2>90 & val2<100){
      val2=90;
    }
    MSDD=val2/10;
    if (MSDD==0){
       PTP= 0x00;
       PT1AD=0b0000000000000000;
    } 
    else if (MSDD==1){
        PTP= 0x00;
       PT1AD=0b0000000000000001;
    } 
    else if (MSDD==2){
       PTP= 0x00;
       PT1AD=0b0000000000000010;
    } 
    else if (MSDD==3){
       PTP= 0x00;
       PT1AD=0b0000000000000100;
    } 
    else if (MSDD==4){
       PTP= 0x00;
       PT1AD=0b0000000000001000;
    } 
    else if (MSDD==5){
       PTP= 0x00;
       PT1AD=0x0010;
    } 
    else if (MSDD==6){
       PT1AD= 0x00;
       PTP=0b00000100;
    } 
    else if (MSDD==7){
       PT1AD= 0x00;
       PTP=0b00001000;
    } 
    else if (MSDD==8){
       PT1AD= 0x00;
       PTP=0b00010000;
    } 
    else if (MSDD==9){
       PT1AD= 0x00;
       PTP=0b00100000;
    }
    
      
    //delayby1ms(205); 
    t=PTT&0b0000010;  //mask, this will determine if the button is pressed again
    delayby1ms(50);
    
    
    }
 }
//****************************************************** binary to bcd mode **********************//     
 void binarytobcd(void){
    //enter any number from 0 to 9999 here:
    delayby1ms(50);
    
    k=1;
    while(k){
      delayby1ms(100);
      Val1 = ATDDR0 ;
      calc(Val1);
      if (val2>100){
      val2=0;
    }
    if (val2>90 & val2<100){
      val2=90;
    }
      //input = val2;
      MSD = val2/10;
      LSD =val2%10;
      LSD = LSD<<2;
        PT1AD=MSD;  
        PTP = LSD;
        //delayby1ms(100); 
        k=PTT&0b0000010;  //mask, this will determine if the button is pressed again
        delayby1ms(50);
      }
 }