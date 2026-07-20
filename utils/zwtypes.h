#ifndef _ZWTYPES_H_
#define _ZWTYPES_H_
#include <stdio.h>    // 提供 NULL
#include <stdbool.h>  // 提供 bool, true, false
#include <stdint.h>   // 标准整数类型
#include <stddef.h>   // 提供 size_t, ptrdiff_t
#include <unistd.h>   // 提供延时sleep, usleep
#include <stdlib.h>
#include <string.h>

// **布尔类型**
typedef bool zwb;  
#define ZW_TRUE  true
#define ZW_FALSE false

// **无符号整数**
typedef uint8_t  zwu8;
typedef uint16_t zwu16;
typedef uint32_t zwu32;
typedef uint64_t zwu64;

// **有符号整数**
typedef int8_t  zwi8;
typedef int16_t zwi16;
typedef int32_t zwi32;
typedef int64_t zwi64;

// **指针类型**
typedef uintptr_t zwuptr;
typedef intptr_t  zwiptr;

// **大小类型**
typedef size_t    zwsize;
typedef ptrdiff_t zwpdiff;

// **浮点数**
typedef float  zwf32;
typedef double zwf64;

// **NULL 定义**
#define ZW_NULL  NULL

#endif