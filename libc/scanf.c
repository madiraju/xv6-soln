#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void atoi(char *numberString, int *number, int base);

int scanf(const char *format, ...) {

	char buf[1];
	char *string;
	char numberString[20];
	va_list val;
	va_start(val, format);
	int i = 0, *number = 0;
	int num_iters=0;

	while(*format) {
		i=0;
		if(*format == '%')
		{
			//increment format to check the next character
			//it may be s, d, c or x
			++format; 
			if(*format == 's')
			{

				//store the next argument in string variable
				string = va_arg(val, char *);
				num_iters=0;
				while(read(0, buf, sizeof(buf))>0) {
			   		//read() here read from stdin character by character
			   		//the buf[0] contains the character got by read()
				  num_iters+=1;
		   			if(buf[0] == '\n')
			   		{
			   			//break the loop if input hits a new line character
			   			//also, terminate the string
			   			string[i] = '\0';
			   			break;
			   		}

			   		//store the input character by character to the input
			   		string[i] = buf[0];
			   		i++;
		   		}
				string[i]='\0';								  
				if(num_iters==0){
				  //EOF
				  return -1;
				}
				else{
				  //num_iters letters have been read
				  return num_iters;
				}
				
			}
			else if(*format == 'd') 
			{
				number = va_arg(val, int* );
				while(read(0, buf, sizeof(buf) > 0)) {

					if(buf[0] == '\n') {
						numberString[i] = '\0';
						atoi(numberString, number, 10);
						break;
					}

					numberString[i] = buf[0];
					i++;
				}
			}
			else if(*format == 'x')
			{
				number = va_arg(val, int*);
				while(read(0, buf, sizeof(buf) > 0)) {

					if(buf[0] == '\n') {
						numberString[i] = '\0';
						atoi(numberString, number, 16);
						break;
					}

					numberString[i] = buf[0];
					i++;
				}
			}
		}
		//check if the first argument of scanf is terminated or not
		++format;
		if(*format == '\0')
		    break;

	}
	va_end(val);
	return 0;
}

void atoi(char *numberString, int *number, int base) {
	int i;
	*number = 0;

	for (i = 0; numberString[i] != '\0'; i++) {
		if((numberString[i] >= '0') && (numberString[i] < 'a'))
			*number = (*number)*base + numberString[i] - '0';
		else if(numberString[i] >='a' && (numberString[i] <='f'))
			*number = (*number)*base + numberString[i] - 'a' + 10;
	}
}
