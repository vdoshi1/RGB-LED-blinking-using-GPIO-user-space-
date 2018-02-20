#include <time.h>
#include <linux/input.h>

#define CYCLE_DURATIONms 20  // PWM cycle duration in milliseconds
#define NANOSECONDS_IN_500ms 500000000ul // converting 0.5 seconds to nanoseconds
#define LEFT 272
#define RIGHT 273

struct input_event ie;            // Structure to store mouse event values
int on_time,off_time,err;
int tstp = 0;	//Thread stop variable, is set to 1 on detection of a mouse click

int R_desc[4], G_desc[4], B_desc[4]; // Holds the file descriptors for gpio pins to select directions and mux select for IO pins on galileo board where  Red, Green and Blue leds are connected 

int j,fd;      // j is for loop variable and fd is file descriptor for /dev/input/mice file

char path[34]; // string to hold path of a particular file to be opened; eg /sys/class/gpio/gpio7/direction

int click,bytes;  // left == 1 when left click occurs and right == 1 when right click occurs. bytes holds the number of bytes read from mouse device file

const char *mDevice = "/dev/input/event3";//Storing path of mouse event. Change this event number if your machine shows different number for mouse event


char gparray[14][4][4]=  {	                                      // Lookup table to get gpio pin numbers to control corresponding IO pins on board
							{"11","32","IGN","IGN"},			  // Each row corresponds to a particular IO pin number
							{"12","28","45","IGN"},               // ROW 0 corresponds to IO0 .... ROW 13 corresponds to IO13 
							{"13","34","77","IGN"},               // Here "IGN" means IGNORE meaning their values don't affect behaviour of IO pin
 							{"14","16","76","64"},
							{"6","36","IGN","IGN"},
							{"0","18","66","IGN"},
							{"1","20","68","IGN"},
							{"38","IGN","IGN","IGN"},
							{"40","IGN","IGN","IGN"},
							{"4","22","70","IGN"},
							{"10","26","74","IGN"},
							{"5","24","44","72"},
							{"15","42","IGN","IGN"},
							{"7","30","46","IGN"}
						};


int check_gpio64to79(char str[])    // Checks whether a gpio pin belongs to the range gpio64-gpio79 because these pins don't require direction file to be set
{
	int ret = atoi(str);            // Converts string to integer to make comparison possible

	if((ret>=64) && (ret<=79))
		return 0;
	else
		return 1;

}


// Initialisation function for RED led pin

void R_init(int R)
{
	for(j=0;j<4;j++)
	{
		if(strcmp(gparray[R][j],"IGN") != 0)  // Initialises only if it's not to be IGNored
		{
			if(check_gpio64to79(gparray[R][j]))// Sets direction only if it doesn't belong to the range gpio64-gpio79
			{
				sprintf(path,"/sys/class/gpio/gpio%s/direction",gparray[R][j]);

				R_desc[j] = open(path, O_WRONLY);
					if(R_desc[j] < 0) 
						printf("ERROR: gpio%s direction open failed \n",gparray[R][j]);

				err = write(R_desc[j],"out",3);       //Setting direction to out
				if(err < 0)
					printf("ERROR in writing to gpio%s direction\n",gparray[R][j] );
			}

			sprintf(path,"/sys/class/gpio/gpio%s/value",gparray[R][j]);

			R_desc[j] = open(path, O_WRONLY);
				if(R_desc[j] < 0) 
					printf("ERROR: gpio%s value open failed \n",gparray[R][j]);

			err = write(R_desc[j],"0",1);  // Sets value to 0
			if(err < 0)
				printf("ERROR in writing gpio%s value\n",gparray[R][j]);
		}
	}

}


// Initialisation function for GREEN led pin

void G_init(int G)
{
	for(j=0;j<4;j++)
	{
		if(strcmp(gparray[G][j],"IGN") != 0)
		{
			if(check_gpio64to79(gparray[G][j]))
			{
				sprintf(path,"/sys/class/gpio/gpio%s/direction",gparray[G][j]);

				G_desc[j] = open(path, O_WRONLY);
					if(G_desc[j] < 0) 
						printf("ERROR: gpio%s direction open failed \n",gparray[G][j]);

				err = write(G_desc[j],"out",3);       //Setting direction to out
				if(err < 0)
					printf("ERROR in writing gpio%s direction\n",gparray[G][j]);
			}

			sprintf(path,"/sys/class/gpio/gpio%s/value",gparray[G][j]);

			G_desc[j] = open(path, O_WRONLY);
				if(G_desc[j] < 0) 
					printf("ERROR: gpio%s value open failed \n",gparray[G][j]);
			
			err = write(G_desc[j],"0",1);  // Sets value to 0
			if(err < 0)
				printf("ERROR in writing gpio%s value\n",gparray[G][j]);

		}
	}
}


// Initialisation function for BLUE led pin

void B_init(int B)
{
	for(j=0;j<4;j++)
	{
		if(strcmp(gparray[B][j],"IGN") != 0)
		{
			if(check_gpio64to79(gparray[B][j])) 
			{
				sprintf(path,"/sys/class/gpio/gpio%s/direction",gparray[B][j]);

				B_desc[j] = open(path, O_WRONLY);

					if(B_desc[j] < 0) 
						printf("gpio%s direction open failed \n",gparray[B][j]);

				err = write(B_desc[j],"out",3);       //Setting direction to out
				if(err < 0)
					printf("ERROR in writing gpio%s direction\n",gparray[B][j]);
			}

			sprintf(path,"/sys/class/gpio/gpio%s/value",gparray[B][j]);

			B_desc[j] = open(path, O_WRONLY);

			if(B_desc[j] < 0) 
				printf("gpio%s value open failed \n",gparray[B][j]);

			err = write(B_desc[j],"0",1);  // Sets value to 0
			if(err < 0)
				printf("ERROR in writing gpio%s value\n",gparray[B][j]);
		}		
	}
}


//Lights up LEDs according to specified pattern and intensity for 0.5 seconds

void lightup(int PWM,int r,int g, int b)
{
	struct timespec curtime,prevtime; //Creates 2 objects of type timespec; defined in <time.h>

	clock_gettime(CLOCK_MONOTONIC, &prevtime); // Initially both contain current value of time obtained from CLOCK_MONOTONIC
	clock_gettime(CLOCK_MONOTONIC, &curtime);

	
	while((curtime.tv_nsec - prevtime.tv_nsec) < NANOSECONDS_IN_500ms) // Runs while the difference between current and initial values of time is less than 0.5s
		{
			if(PWM == 0)
				goto SKIP_ON;
			if(r == 1)
				err = write(R_desc[0],"1",1);   // Turns ON corresponding combination of LEDs according to specified pattern; for on time duration
				if(err < 0)
					printf("ERROR writing to RED LED\n");

			if(g == 1)
				err = write(G_desc[0],"1",1);
				if(err < 0)
					printf("ERROR writing to GREEN LED\n");

			if(b == 1)
				err = write(B_desc[0],"1",1);
				if(err < 0)
					printf("ERROR writing to BLUE LED\n");

			usleep(on_time);              // Waits for on time duration before turning LEDs off


	SKIP_ON:

			if(r == 1)
				err = write(R_desc[0],"0",1);  // Turns LEDs which were on, for off time duration
				if(err < 0)
					printf("ERROR writing to RED LED\n");

			if(g == 1)
				err = write(G_desc[0],"0",1);
				if(err < 0)
					printf("ERROR writing to GREEN LED\n");

			if(b == 1)
				err = write(B_desc[0],"0",1);
				if(err < 0)
					printf("ERROR writing to BLUE LED\n");
 
			usleep(off_time);            // Waits for off time duration before turning LEDs on again

			clock_gettime(CLOCK_MONOTONIC, &curtime);   // Updates current time in each iteration
		}		
		
}


void* m_check(void* arg)                         // Checks for a mouse click event and return 1 on detection; returns 0 otherwise
{
	while(tstp == 0)
	{
		bytes = read(fd, &ie, sizeof(struct input_event)); 

	        if(bytes > 0)
	        {
	         click = ie.code;

	         if ((click == LEFT) || (click == RIGHT)) 
	              tstp = 1;           
	         }
	 }

	 pthread_exit(NULL);
}