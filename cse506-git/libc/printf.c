
#include <syscall.h>
#include <stdio.h>

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
//#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;


// Will return the length of the string
unsigned int strlen(const char *str)
{
        int i = 0;
        while(str[i] != '\0')
                i++;
        return i;
}

// Will set the int array to 0's
unsigned int int_array_reset(int array[], int cnt)
{
        int i = 0;
        for(i = 0; i < cnt; i++) {
                array[i] = 0;
        }
        return (1);
}

// Will set the char array to '\0'
unsigned int char_array_reset(char array[], int cnt)
{
        int i = 0;
        for(i = 0; i < cnt; i++) {
                array[i] = '\0';
        }
        return (1);
}

// will spit out the integer at the given line number
unsigned int printf_integer(int message, char* str) // the message and the line #
{
        unsigned int i = 0;

        int digits[12];
        unsigned int remainder = 0;
        unsigned int quotient = message;
        int cnt = 0;

        int_array_reset(digits, 12);

        while(quotient >= 10) {
                remainder = (int) (quotient % 10);
                quotient = (int) (quotient / 10);

                digits[cnt] = remainder;
                cnt++;
        }
        digits[cnt] = quotient;
        while(cnt >= 0) {
                *str++ = digits[cnt] + 48;
                cnt--;
                i++;
        }
        *str++ = '\0';
        return i;
}

// will spit out the hexadecimal at the given line number
unsigned int printf_hexadecimal(unsigned long message, char* str) // the message and the line #
{
        unsigned int i = 0;

        int cnt = 0;
        char result[8];
        char hex[] = "0123456789abcdef";
        unsigned long quotient = message;

        char_array_reset(result, 8);

        while(quotient > 0) {
                result[cnt++] = hex[(quotient % 16)];
                quotient = (quotient / 16);
        }

        *str++ = '0';
        *str++ = 'x';

        cnt--;
        while(cnt >= 0)
        {
                *str++ = result[cnt];
                cnt--;
        i++;
        }

        *str++ = '\0';
        return i;
}

/*
* Handles print function
*/
int printf(const char *fmt, ...)
{
        va_list args;

        int len = 0;
        // int str_len = 0;
        int cnt = 0;
        char str[1024];
        char str_temp[1024];
        int i = 0;

        va_start(args, fmt);

        // flush array
        char_array_reset(str, 1024);
        char_array_reset(str_temp, 1024);

        // TODO: write the code to print        
        for(;*fmt;)
        {
                if(*fmt != '%') {
                        str[cnt++] = *fmt++;
                        continue;
                }

                fmt++;
                switch(*fmt) {
                        case 'c':
                                str[cnt++] = (unsigned char) va_arg(args, int);
                                fmt++;
                                continue;
                        case 'd':
                                i = 0;
                                printf_integer(va_arg(args, int), str_temp);
                                len = strlen(str_temp);
                                while(i < len) {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 's':
                                i = 0;
                                char *str_t = (char *)va_arg(args, char *);
                                len = strlen(str_t);
                                while(*str_t!='\0')
                                {
                                        str[cnt++] = *str_t++;
                                }
                                fmt++;
                                continue;
                        case 'x':
                                i = 0;
                                printf_hexadecimal(va_arg(args, int), str_temp);
                                len = strlen(str_temp);
                                while(i < len)
                                {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 'p':
                                i = 0;
                                printf_hexadecimal(va_arg(args, unsigned long), str_temp);
                                len = strlen(str_temp);
                                while(i < len)
                                {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        default:
                                fmt++;
                                continue;
                }
        }
        str[cnt] = '\0';

        va_end(args);
  int r = 0;
  while(str[r] != '\0')
  __syscall1(2, (uint64_t)str[r++]);
    return 0;
}
