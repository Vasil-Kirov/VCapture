#pragma once
#include <windows.h>
#define vstd_printf(str, ...) _vstd_Printf(sizeof(str) - 1, str, __VA_ARGS__)
#define vstd_sprintf(buff, str, ...) _vstd_sPrintf(sizeof(str) - 1, buff, str, __VA_ARGS__)
typedef wchar_t wchar;

struct vstdRect 
{
	int x1;
	int y1;
	int x2;
	int y2;
};
static HANDLE STDOUT_HANDLE = GetStdHandle((DWORD)-11);

void vstd_strcat(wchar *dst, const wchar *src)
{
	while(*dst != '\0') dst++;
	while(*src != '\0')
	{
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\0';
}


void _vstd_IntToStr(int num, char* arr_to_fill)
{
    int CopyToDivide = num;
    int num_size = -1;           // -1 cuz we want the size - 1 to get the last index of the array.
    while (CopyToDivide != 0)
    {
        CopyToDivide /= 10;
        num_size++;
    }
    if (num < 0)
    {
        arr_to_fill[0] = '-';
        num_size++;
    }
    arr_to_fill[num_size + 1] = '\0';
    for (int i = num_size; num != 0; --i)
    {
        int tmp = (int)(num % 10);
        num /= 10;
        arr_to_fill[i] = (char)tmp + '0';
    }
}


//@Note: make it so it works with long and chars
void _vstd_Printf(int FormatSize, const char* str, ...)
{
#define V_WRITE_BUFFER_SIZE 8192
    va_list args;
    va_start(args, str);

    char Buffer[V_WRITE_BUFFER_SIZE];
    int BufferIndex = 0;
    for (int FormatIndex = 0; FormatIndex < FormatSize; ++FormatIndex)
    {
        char FormatChar = str[FormatIndex];
        if (FormatChar == '%')
        {
            ++FormatIndex;
            switch (str[FormatIndex])
            {
            case 's':
            {
                char* ToCopy = va_arg(args, char*);
                while (*ToCopy != 0)
                {
                    Buffer[BufferIndex++] = *ToCopy;
                    ++ToCopy;
                }
            }break;
            case 'd':
            {
                int Number = va_arg(args, int);
                char arr[20];
                char* ToCopy = arr;
                _vstd_IntToStr(Number, ToCopy);
                while (*ToCopy != 0)
                {
                    Buffer[BufferIndex++] = *ToCopy;
                    ++ToCopy;
                }
            }break;
            case '\\':
            {
                Buffer[BufferIndex++] = '%';
            }break;
            default:
            {
                Buffer[BufferIndex++] = '%';
                Buffer[BufferIndex++] = str[FormatIndex];
            }break;
            }
        }
        else
        {
            Buffer[BufferIndex++] = str[FormatIndex];
        }
    }
    va_end(args);
    DWORD BytesWritten;
    WriteFile(STDOUT_HANDLE, Buffer, BufferIndex, &BytesWritten, NULL);
#undef V_WRITE_BUFFER_SIZE
}


void _vstd_sPrintf(int FormatSize, char *buff, const char* str, ...)
{
    va_list args;
    va_start(args, str);

    int BufferIndex = 0;
    for (int FormatIndex = 0; FormatIndex < FormatSize; ++FormatIndex)
    {
        char FormatChar = str[FormatIndex];
        if (FormatChar == '%')
        {
            ++FormatIndex;
            switch (str[FormatIndex])
            {
            case 's':
            {
                char* ToCopy = va_arg(args, char*);
                while (*ToCopy != 0)
                {
                    buff[BufferIndex++] = *ToCopy;
                    ++ToCopy;
                }
            }break;
            case 'd':
            {
                int Number = va_arg(args, int);
                char arr[20];
                char* ToCopy = arr;
                if (Number != 0)
                {
                    _vstd_IntToStr(Number, ToCopy);
                    while (*ToCopy != 0)
                    {
                        buff[BufferIndex++] = *ToCopy;
                        ++ToCopy;
                    }
                }
                else buff[BufferIndex++] = '0';
            }break;
            case '\\':
            {
                buff[BufferIndex++] = '%';
            }break;
            default:
            {
                buff[BufferIndex++] = '%';
                buff[BufferIndex++] = str[FormatIndex];
            }break;
            }
        }
        else
        {
            buff[BufferIndex++] = str[FormatIndex];
        }
    }
    va_end(args);
}

void Error(const char *error_msg)
{
	char buf[4096];
	vstd_sprintf(buf, "%s\nError Code: %s\n", error_msg, GetLastError());
	MessageBoxA(NULL, buf, NULL, MB_OK);
	ExitProcess(1);
}