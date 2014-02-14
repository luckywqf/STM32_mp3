#include "lib.h"
void reverse(char s[])
{
	int c, i, j;
	for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
uint8_t* itoa(int n, char s[])
{
	int i, sign;
	if ((sign = n) < 0) /* record sign */
	n = -n; /* make n positive */
	i = 0;
	do { /* generate digits in reverse order */
		s[i++] = n % 10 + '0'; /* get next digit */
	} while ((n /= 10) > 0); /* delete it */
	if (sign < 0)
	s[i++] = '-';
	s[i] = '\0';
	reverse(s);
	return (uint8_t*)s;
}

char* FillName(char *fileName,char* src)
{
	u8 i;
	if(fileName[1]=='p')
		strcpy(fileName,"/photo/");
	else 
		strcpy(fileName,"/music/");

	for(i=0;src[i]!=0;i++)fileName[i+7]=src[i];
	fileName[i+7]=0;
	return fileName;
}

