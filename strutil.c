#include "strutil.h"
#include "common.h"

// 去除右边的\r\n
void str_trim_crlf(char *str)
{
    char *p = &str[strlen(str)-1];
    while(*p == '\r' || *p == '\n')
        *p-- = '\0';
}

// 分割字符串，根据c分割为两部分
void str_split(const char *str , char *left, char *right, char c)
{
    //strchr返回字符串str中第一次出现字符c的位置
    char *p = strchr(str, c);
    if (p == NULL)
        strcpy(left, str);
    else
    {
        //strncpy最多拷贝p-str个字符到字符串left中
        strncpy(left, str, p-str);
        //strcpy从p+1指向的位置拷贝字符串到right指向的开始位置
        strcpy(right, p+1);
    }
}

//判断字符串是否全部为空格
int str_all_space(const char *str)
{
    while (*str)
    {
        if (!isspace(*str))
            return 0;
        ++str;
    }
    return 1;
}

//把字符串转化为大写
void str_upper(char *str)
{
    while (*str)
    {
        *str = toupper(*str);
        ++str;
    }
}

//把八进制字符串转化为无符号整数
unsigned int str_octal_to_uint(const char *str)
{
    unsigned int result = 0;
    int seen_non_zero_digit = 0;

    while (*str)
    {
        int digit = *str;
        if (!isdigit(digit) || digit > '7')
            break;

        if (digit != '0')
            seen_non_zero_digit = 1;

        if (seen_non_zero_digit)
        {
            result <<= 3;
            result += (digit - '0');
        }
        ++str;
    }
    return result;
}

