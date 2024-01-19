#pragma once

#define WINDOW_WIDTH 800 //为窗口宽度定义的宏，以方便在此处修改窗口宽度
#define WINDOW_HEIGHT 600 //为窗口高度定义的宏，以方便在此处修改窗口高度
#define WINDOW_TITLE L"示例程序" //为窗口标题定义的宏
#define SAFE_RELEASE(p)     \
    {                       \
        if (p) {            \
            (p)->Release(); \
            (p) = NULL;     \
        }                   \
    } //定义一个安全释放宏，便于后面COM接口指针的释放