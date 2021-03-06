/*
 *  fasttrig.c
 *
 *  Created on: Aug 10, 2013
 *      Author: ala42
 */
#include <math.h>
#include "fasttrig.h"
#include "utils.h"
#include "comio.h"

//@HackOS: M_TWOPI
#define M_TWOPI 6.2831853071796f

//@HackOS: 正弦函数查表
short int sinDataI16[SINARRAYSIZE];

//@HackOS: 初始化正弦函数数组
void InitSinArray(void)
{
    for (int i = 0; i < SINARRAYSIZE; i++)
    {
        float x = i * M_TWOPI / SINARRAYSIZE;
        sinDataI16[i] = (short int)Round(sinf(x) * SINARRAYSCALE);
        //print("i %3d  x %f  sin %d\r\n", i, x, (int)sinDataI16[i]);
    }
}

//@HackOS: 查表法正弦函数
float fastSin(float x)
{
    if (x >= 0)
    {
        int ix = ((int)(x / M_TWOPI * (float)SINARRAYSIZE)) % SINARRAYSIZE;
        return sinDataI16[ix] / (float)SINARRAYSCALE;
    }
    else
    {
        int ix = ((int)(-x / M_TWOPI * (float)SINARRAYSIZE)) % SINARRAYSIZE;
        return -sinDataI16[ix] / (float)SINARRAYSCALE;
    }
}

