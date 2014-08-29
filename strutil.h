#ifndef _STRUTIL_H_
#define _STRUTIL_H_

//去除右边的\r\n
void str_trim_crlf(char *str);
//分割字符串，根据c分割为两部分
void str_split(const char *str , char *left, char *right, char c);
//判断字符串是否全部为空格
int str_all_space(const char *str);
//把字符串转化为大写
void str_upper(char *str);
//把八进制字符串转化为无符号整数
unsigned int str_octal_to_uint(const char *str);

#endif  /*_STRUTIL_H_*/
