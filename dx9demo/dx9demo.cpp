// dx9demo.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "dx9demo.h"
#include "D3D9Device.h"
#include <tchar.h>
#include <timeapi.h>
#include "demo_define.h"



//设计顶点格式
struct CUSTOMVERTEX {
    FLOAT x, y, z;
    DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE) //FVF灵活顶点格式

//全局变量的声明
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL; //Direct3D设备对象
ID3DXFont* g_pFont = NULL; //字体COM接口
float g_FPS = 0.0f; //一个浮点型的变量，代表帧速率
wchar_t g_strFPS[50]; //包含帧速率的字符数组
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL; //顶点缓冲区对象
LPDIRECT3DINDEXBUFFER9 g_pIndexBuffer = NULL; // 索引缓存对象

//全局函数声明，防止“未声明的标识”系列错误
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam); //窗口过程函数
HRESULT Direct3D_Init(HWND hwnd); //在这个函数中进行Direct3D的初始化
HRESULT Objects_Init(HWND hwnd); //在这个函数中进行要绘制的物体的资源初始化
VOID Direct3D_Render(HWND hwnd); //在这个函数中进行Direct3D渲染代码的书写
VOID Direct3D_CleanUp(); //在这个函数中清理COM资源以及其他资源
float Get_FPS(); //计算帧数的函数
VOID Matrix_Set(); //封装了四大变换的函数

//WinMain( )函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEX wndClass = { 0 }; //用WINDCLASSEX定义了一个窗口类
    wndClass.cbSize = sizeof(WNDCLASSEX); //设置结构体的字节数大小
    wndClass.style = CS_HREDRAW | CS_VREDRAW; //设置窗口的样式
    wndClass.lpfnWndProc = WndProc; //设置指向窗口过程函数的指针
    wndClass.cbClsExtra = 0; //窗口类的附加内存，取0就可以了
    wndClass.cbWndExtra = 0; //窗口的附加内存，依然取0就行了
    wndClass.hInstance = hInstance; //指定包含窗口过程的程序的实例句柄。
    wndClass.hIcon = NULL;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW); //指定窗口类的光标句柄。
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); //为hbrBackground成员指定一个黑色画刷句柄
    wndClass.lpszMenuName = NULL; //用一个以空终止的字符串，指定菜单资源的名字。
    wndClass.lpszClassName = L"Test"; //用一个以空终止的字符串，指定窗口类的名字。

    if (!RegisterClassEx(&wndClass)) //设计完窗口后，需要对窗口类进行注册，这样才能创建该类型的窗口
        return -1;

    HWND hwnd = CreateWindow(L"Test", WINDOW_TITLE, //创建窗口函数CreateWindow
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH,
        WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

    //Direct3D资源的初始化，调用失败用messagebox予以显示
    if (!(S_OK == Direct3D_Init(hwnd))) {
        MessageBox(hwnd, _T("Direct3D初始化失败！"), _T("Test"), 0); //使用MessageBox函数，创建一个消息窗口
    }

    MoveWindow(hwnd, 250, 80, WINDOW_WIDTH, WINDOW_HEIGHT, true); //调整窗口显示时的位置，使窗口左上角位于（250,80）处
    ShowWindow(hwnd, nShowCmd); //调用ShowWindow函数来显示窗口
    UpdateWindow(hwnd); //对窗口进行更新

    MSG msg = { 0 }; //初始化msg
    while (msg.message != WM_QUIT) //使用while循环
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) //查看应用程序消息队列，有消息时将队列中的消息派发出去。
        {
            TranslateMessage(&msg); //将虚拟键消息转换为字符消息
            DispatchMessage(&msg); //该函数分发一个消息给窗口程序。
        } else {
            Direct3D_Render(hwnd); //进行渲染
        }
    }
    UnregisterClass(L"Test", wndClass.hInstance); //程序准备结束，注销窗口类
    return 0;
}

//WndProc( )函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) //switch语句开始
    {
    case WM_PAINT: // 若是客户区重绘消息
        Direct3D_Render(hwnd); //调用Direct3D渲染函数
        ValidateRect(hwnd, NULL); // 更新客户区的显示
        break; //跳出该switch语句

    case WM_KEYDOWN: // 若是键盘按下消息
        if (wParam == VK_ESCAPE) // 如果被按下的键是ESC
            DestroyWindow(hwnd); // 销毁窗口, 并发送一条WM_DESTROY消息
        break; //跳出该switch语句

    case WM_DESTROY: //若是窗口销毁消息
        Direct3D_CleanUp(); //调用自定义的资源清理函数Game_CleanUp（）进行退出前的资源清理
        PostQuitMessage(0); //向系统表明有个线程有终止请求。用来响应WM_DESTROY消息
        break; //跳出该switch语句

    default: //若上述case条件都不符合，则执行该default语句
        return DefWindowProc(hwnd, message, wParam, lParam); //调用缺省的窗口过程
    }

    return 0; //正常退出
}

//Direct3D_Init( )函数
HRESULT Direct3D_Init(HWND hwnd)
{
    D3D9Device d3d9(hwnd);
    g_pd3dDevice = d3d9.device9();

    if (S_OK != Objects_Init(hwnd))
        return E_FAIL; //调用一次Objects_Init，进行渲染资源的初始化

    // 设置渲染状态
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE); //关闭光照
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW); //开启背面消隐

    return S_OK;
}

//Object_Init( )函数
HRESULT Objects_Init(HWND hwnd)
{
    //创建字体
    if (FAILED(D3DXCreateFont(g_pd3dDevice, 36, 0, 0, 1, false, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, _T("微软雅黑"), &g_pFont)))
        return E_FAIL;
    srand(timeGetTime()); //用系统时间初始化随机种子

    //创建顶点缓存
    if (FAILED(g_pd3dDevice->CreateVertexBuffer(8 * sizeof(CUSTOMVERTEX),
            0, D3DFVF_CUSTOMVERTEX,
            D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL))) {
        return E_FAIL;
    }
    // 创建索引缓存
    if (FAILED(g_pd3dDevice->CreateIndexBuffer(36 * sizeof(WORD), 0,
            D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIndexBuffer, NULL))) {
        return E_FAIL;
    }
    //顶点数据的设置，
    CUSTOMVERTEX Vertices[] = {
        { -20.0f, 20.0f, -20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },
        { -20.0f, 20.0f, 20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },
        { 20.0f, 20.0f, 20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },
        { 20.0f, 20.0f, -20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },
        { -20.0f, -20.0f, -20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },
        { -20.0f, -20.0f, 20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },
        { 20.0f, -20.0f, 20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },
        { 20.0f, -20.0f, -20.0f, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256) },

    };

    //填充顶点缓存
    VOID* pVertices;
    if (FAILED(g_pVertexBuffer->Lock(0, sizeof(Vertices), (void**)&pVertices, 0)))
        return E_FAIL;
    memcpy(pVertices, Vertices, sizeof(Vertices));
    g_pVertexBuffer->Unlock();

    // 填充索引数据
    WORD* pIndices = NULL;
    g_pIndexBuffer->Lock(0, 0, (void**)&pIndices, 0);

    // 顶面
    pIndices[0] = 0, pIndices[1] = 1, pIndices[2] = 2;
    pIndices[3] = 0, pIndices[4] = 2, pIndices[5] = 3;
    // 正面
    pIndices[6] = 0, pIndices[7] = 3, pIndices[8] = 7;
    pIndices[9] = 0, pIndices[10] = 7, pIndices[11] = 4;
    // 左侧面
    pIndices[12] = 0, pIndices[13] = 4, pIndices[14] = 5;
    pIndices[15] = 0, pIndices[16] = 5, pIndices[17] = 1;
    // 右侧面
    pIndices[18] = 2, pIndices[19] = 6, pIndices[20] = 7;
    pIndices[21] = 2, pIndices[22] = 7, pIndices[23] = 3;
    // 背面
    pIndices[24] = 2, pIndices[25] = 5, pIndices[26] = 6;
    pIndices[27] = 2, pIndices[28] = 1, pIndices[29] = 5;
    // 底面
    pIndices[30] = 4, pIndices[31] = 6, pIndices[32] = 5;
    pIndices[33] = 4, pIndices[34] = 7, pIndices[35] = 6;
    g_pIndexBuffer->Unlock();

    return S_OK;
}

//Matrix_Set( )函数

VOID Matrix_Set()
{
    //世界变换矩阵的设置
    D3DXMATRIX matWorld, Rx, Ry, Rz;
    D3DXMatrixIdentity(&matWorld); // 单位化世界矩阵
    D3DXMatrixRotationX(&Rx, D3DX_PI * (::timeGetTime() / 1000.0f / 3)); // 绕X轴旋转
    D3DXMatrixRotationY(&Ry, D3DX_PI * (::timeGetTime() / 1000.0f / 3)); // 绕Y轴旋转
    D3DXMatrixRotationZ(&Rz, D3DX_PI * (::timeGetTime() / 1000.0f / 3)); // 绕Z轴旋转
    matWorld = Rx * Ry * Rz * matWorld; // 得到最终的组合矩阵
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld); //设置世界变换矩阵

    //取景变换矩阵的设置
    D3DXMATRIX matView; //定义一个矩阵
    D3DXVECTOR3 vEye(0.0f, 0.0f, -200.0f); //摄像机的位置
    D3DXVECTOR3 vAt(0.0f, 0.0f, 0.0f); //观察点的位置
    D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f); //向上的向量
    D3DXMatrixLookAtLH(&matView, &vEye, &vAt, &vUp); //计算出取景变换矩阵
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView); //应用取景变换矩阵

    //投影变换矩阵的设置
    D3DXMATRIX matProj; //定义一个矩阵
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4.0f, 1.0f, 1.0f, 1000.0f); //计算投影变换矩阵
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj); //设置投影变换矩阵

    //视口变换的设置
    D3DVIEWPORT9 vp; //实例化一个D3DVIEWPORT9结构体，然后做填空题给各个参数赋值就可以了
    vp.X = 0; //表示视口相对于窗口的X坐标
    vp.Y = 0; //视口相对对窗口的Y坐标
    vp.Width = WINDOW_WIDTH; //视口的宽度
    vp.Height = WINDOW_HEIGHT; //视口的高度
    vp.MinZ = 0.0f; //视口在深度缓存中的最小深度值
    vp.MaxZ = 1.0f; //视口在深度缓存中的最大深度值
    g_pd3dDevice->SetViewport(&vp); //视口的设置
}

//Direct3D_Render( )函数
void Direct3D_Render(HWND hwnd)
{
    // 清屏操作
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    //定义一个矩形，用于获取主窗口矩形
    RECT formatRect;
    GetClientRect(hwnd, &formatRect);
    //开始绘制
    g_pd3dDevice->BeginScene(); // 开始绘制

    Matrix_Set(); //调用封装了四大变换的函数，对Direct3D世界变换，取景变换，投影变换，视口变换进行设置
    // 获取键盘消息并给予设置相应的填充模式
    if (::GetAsyncKeyState(0x31) & 0x8000f) // 若数字键1被按下，进行线框填充
        g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    if (::GetAsyncKeyState(0x32) & 0x8000f) // 若数字键2被按下，进行实体填充
        g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    g_pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof(CUSTOMVERTEX)); //把包含的几何体信息的顶点缓存和渲染流水线相关联
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); //指定我们使用的灵活顶点格式的宏名称
    g_pd3dDevice->SetIndices(g_pIndexBuffer); //设置索引缓存
    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12); //利用索引缓存配合顶点缓存绘制图形

    //在窗口右上角处，显示每秒帧数
    int charCount = swprintf_s(g_strFPS, 20, _T("FPS:%0.3f"), Get_FPS());
    g_pFont->DrawText(NULL, g_strFPS, charCount, &formatRect, DT_TOP | DT_RIGHT, D3DCOLOR_XRGB(255, 39, 136));

    // 结束绘制
    g_pd3dDevice->EndScene(); // 结束绘制
    // 显示翻转
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL); // 翻转与显示
}

//用于计算每秒帧速率的一个函数
float Get_FPS()
{
    //定义四个静态变量
    static float fps = 0; //我们需要计算的FPS值
    static int frameCount = 0; //帧数
    static float currentTime = 0.0f; //当前时间
    static float lastTime = 0.0f; //持续时间

    frameCount++; //每调用一次Get_FPS()函数，帧数自增1
    currentTime = timeGetTime() * 0.001f; //获取系统时间，其中timeGetTime函数返回的是以毫秒为单位的系统时间，所以需要乘以0.001，得到单位为秒的时间

    //如果当前时间减去持续时间大于了1秒钟，就进行一次FPS的计算和持续时间的更新，并将帧数值清零
    if (currentTime - lastTime > 1.0f) //将时间控制在1秒钟
    {
        fps = (float)frameCount / (currentTime - lastTime); //计算这1秒钟的FPS值
        lastTime = currentTime; //将当前时间currentTime赋给持续时间lastTime，作为下一秒的基准时间
        frameCount = 0; //将本次帧数frameCount值清零
    }

    return fps;
}

//资源清理函数，在此函数中进行程序退出前资源的清理工作
void Direct3D_CleanUp()
{
    //释放COM接口对象
    SAFE_RELEASE(g_pIndexBuffer)
    SAFE_RELEASE(g_pVertexBuffer)
    SAFE_RELEASE(g_pFont)
    SAFE_RELEASE(g_pd3dDevice)
}
