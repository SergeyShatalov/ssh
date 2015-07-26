
#pragma once

#define BUILDING_LIBCHARSET
#define BUILDING_LIBICONV
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows

#include <windows.h>

#include <common\iconv.h>
#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable:4018)
#pragma warning(disable:4267)
#pragma warning(disable:4244)
