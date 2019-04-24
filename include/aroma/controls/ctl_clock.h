#ifndef _CTL_CLOCK_H_
#define _CTL_CLOCK_H_

/******************************************************************************
** 枚举定义
******************************************************************************/

typedef enum
{
    E_WEEK_MON = 0,
    E_WEEK_TUE = 1,
    E_WEEK_WED = 2,
    E_WEEK_THR = 3,
    E_WEEK_FRI = 4,
    E_WEEK_SAT = 5,
    E_WEEK_SUN = 6,
    E_WEEK_BUTT,
}WEEK_E;


/******************************************************************************
** 宏定义
******************************************************************************/

#define MYPI 3.14159265358979323846f

#define CTL_CLK_JUDGE_AND_FREE(_p, _fun) if(NULL != (_p))\
{\
    _fun((_p));\
    (_p) = NULL;\
}

#define CTL_CLK_JUDGE_AND_FREE_CANVAS(_p) if(NULL != (_p))\
{\
    libaroma_canvas_free(_p);\
    (_p) = NULL;\
}


#define CLT_CLK_CHECK_INPUT_POINT_RET_NUL(_p) if(NULL == (_p))\
{\
    ALOGW("%s input param is NULL", __FUNCTION__);\
    return NULL;\
}


/******************************************************************************
** 函数声明
******************************************************************************/

#endif /* INCLUDE_AROMA_CONTROLS_CTL_CLOCK_H_ */

