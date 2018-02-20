//onboard LED_blinking for intel galileo gen 2

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "led.h"

pthread_t pid;

int main(int argc, char** argv) 	// argv is a vector containing user input parameters , but in string format.  
{ 	int ter = 0;								// 4 user inputs are required PWM duty cycle, board pin numbers for Red,Green and Blue LEDs respectively
	
	if(argc != 5)                   // If there are fewer or more than 4 arguments, it prints error and terminates
		{
			printf("\n\nERROR: Only 4 inputs allowed  PWM R G B\n");
			ter = 1;
		}

	int PWM = atoi(argv[1]);       // Converts string into intergers (ASCII to integer)
	if((!(PWM>=0 && PWM <=100)) || ((atof(argv[1]) - (float)PWM) > 0.0))// If user enters PWM duty cycle outside of 0-100% range,or not an integer; it produces error message and program terminates
	{
		printf("\n\nERROR: Invalid value for PWM!\nPlease choose integer value from 0-100\n\n");
		ter = 1;;
	}


	int R = atoi(argv[2]);	
	if((!(R>=0 && R<=13)) || ((atof(argv[2]) - (float)R) > 0.0))      // If pin number is outside the acceptable range which is 0-13, or not an integer; produces error message and program terminates
	{
		printf("\n\nERROR: Invalid value for pin number of RED LED!\nPlease choose integer value from 0-13\n\n");
		ter = 1;;
	}


	int G = atoi(argv[3]);
	if((!(G>=0 && G<=13)) || ((atof(argv[3]) - (float)G) > 0.0))      // If pin number is outside the acceptable range which is 0-13,or not an integer; produces error message and program terminates
	{
		printf("\n\nERROR: Invalid value for pin number of GREEN LED!\nPlease choose integer value from 0-13\n\n");
		ter = 1;;
	}


	int B = atoi(argv[4]);
	if((!(B>=0 && B<=13)) || ((atof(argv[4]) - (float)B) > 0.0))      // If pin number is outside the acceptable range which is 0-13, or not an integer; produces error message and program terminates
	{
		printf("\n\nERROR: Invalid value for pin number of BLUE LED!\nPlease choose integer value from 0-13\n\n");
		ter = 1;;
	}

	if(ter == 1)
		goto TERMINATE;
	
	on_time = ((float)PWM/100) * 1000 * CYCLE_DURATIONms; // Calculates on time and off time of LEDs in microseconds, to control intensity
	off_time = (1-((float)PWM/100)) * 1000 * CYCLE_DURATIONms;
	printf("On time : %d microseconds\nOff time : %d microseconds\n\n",on_time,off_time);


	fd = open(mDevice,O_RDWR);     // Opens mice device file located in /dev/input/mice

    if(fd == -1) //Because open returns -1 on occurence of error
    {
    	printf("ERROR Opening %s\n", mDevice);
    	goto TERMINATE;
    }


	//GPIO EXPORT
    int fdexport;

	fdexport = open("/sys/class/gpio/export", O_WRONLY);  // Opens file in which we will export all the gpio pins necessary to use IO pins on Galileo board

	if(fdexport < 0) 
	{
		printf("gpio export open failed \n");
		goto TERMINATE;
	}

	
	for(j=0;j<4;j++)
	{
		write(fdexport,gparray[R][j],sizeof(gparray[R][j])); // Each IO pin on board requires a maximum of 4 gpio pins to control it 
		write(fdexport,gparray[G][j],sizeof(gparray[G][j])); // (1 for direction, 1 is pin which controls IO pin, 2 for selecting that gpio pin via multiplexing)
		write(fdexport,gparray[B][j],sizeof(gparray[B][j])); // these pins are selected from lookup table in led.h library
	}

   close(fdexport);

	R_init(R);  // Initialises 3 IO pins by setting corresponding gpio pin's direction and value
	G_init(G);
	B_init(B);
   
   // Creating pthread which will run parallelly to detect a mouse click event
   pthread_create(&pid,NULL,m_check,NULL);
	
	while(1)
	 {
		lightup(PWM , 1 , 0 , 0);   // lightup function lights leds which have 1 value with intensity controlled through PWM duty cycle mentioned by PWM argument
		if(tstp == 1)
			break;
		lightup(PWM , 0 , 1 , 0);	// after PWM, first argument is for lighting up RED, second for GREEN and third for BLUE
		if(tstp == 1)
			break;
		lightup(PWM , 0 , 0 , 1);	
		if(tstp == 1)               // m_check() function in parallel pthread checks between each step if there was a mouse click and sets tstp to 1 if detected
			break;					// if mouse click is detected; value of tstp is 1 and program terminates
		lightup(PWM , 1 , 1 , 0);    
		if(tstp == 1)
			break;
		lightup(PWM , 1 , 0 , 1);
		if(tstp == 1)
			break;
		lightup(PWM , 0 , 1 , 1);
		if(tstp == 1)
			break;
		lightup(PWM , 1 , 1 , 1);
		if(tstp == 1)
			break;
	 }

	 pthread_join(pid,NULL);

	 printf("Mouse-click detected! \n\n");

TERMINATE:
	 printf("System successfully terminated!\n\n");
	 printf("Good Bye!\n\n");


 return 0;
}
