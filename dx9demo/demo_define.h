#pragma once

#define WINDOW_WIDTH 800 //Ϊ���ڿ�ȶ���ĺ꣬�Է����ڴ˴��޸Ĵ��ڿ��
#define WINDOW_HEIGHT 600 //Ϊ���ڸ߶ȶ���ĺ꣬�Է����ڴ˴��޸Ĵ��ڸ߶�
#define WINDOW_TITLE L"ʾ������" //Ϊ���ڱ��ⶨ��ĺ�
#define SAFE_RELEASE(p)     \
    {                       \
        if (p) {            \
            (p)->Release(); \
            (p) = NULL;     \
        }                   \
    } //����һ����ȫ�ͷź꣬���ں���COM�ӿ�ָ����ͷ�