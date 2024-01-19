// dx9demo.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "dx9demo.h"
#include "D3D9Device.h"
#include <tchar.h>
#include <timeapi.h>
#include "demo_define.h"



//��ƶ����ʽ
struct CUSTOMVERTEX {
    FLOAT x, y, z;
    DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE) //FVF�����ʽ

//ȫ�ֱ���������
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL; //Direct3D�豸����
ID3DXFont* g_pFont = NULL; //����COM�ӿ�
float g_FPS = 0.0f; //һ�������͵ı���������֡����
wchar_t g_strFPS[50]; //����֡���ʵ��ַ�����
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL; //���㻺��������
LPDIRECT3DINDEXBUFFER9 g_pIndexBuffer = NULL; // �����������

//ȫ�ֺ�����������ֹ��δ�����ı�ʶ��ϵ�д���
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam); //���ڹ��̺���
HRESULT Direct3D_Init(HWND hwnd); //����������н���Direct3D�ĳ�ʼ��
HRESULT Objects_Init(HWND hwnd); //����������н���Ҫ���Ƶ��������Դ��ʼ��
VOID Direct3D_Render(HWND hwnd); //����������н���Direct3D��Ⱦ�������д
VOID Direct3D_CleanUp(); //���������������COM��Դ�Լ�������Դ
float Get_FPS(); //����֡���ĺ���
VOID Matrix_Set(); //��װ���Ĵ�任�ĺ���

//WinMain( )����
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEX wndClass = { 0 }; //��WINDCLASSEX������һ��������
    wndClass.cbSize = sizeof(WNDCLASSEX); //���ýṹ����ֽ�����С
    wndClass.style = CS_HREDRAW | CS_VREDRAW; //���ô��ڵ���ʽ
    wndClass.lpfnWndProc = WndProc; //����ָ�򴰿ڹ��̺�����ָ��
    wndClass.cbClsExtra = 0; //������ĸ����ڴ棬ȡ0�Ϳ�����
    wndClass.cbWndExtra = 0; //���ڵĸ����ڴ棬��Ȼȡ0������
    wndClass.hInstance = hInstance; //ָ���������ڹ��̵ĳ����ʵ�������
    wndClass.hIcon = NULL;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW); //ָ��������Ĺ������
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); //ΪhbrBackground��Աָ��һ����ɫ��ˢ���
    wndClass.lpszMenuName = NULL; //��һ���Կ���ֹ���ַ�����ָ���˵���Դ�����֡�
    wndClass.lpszClassName = L"Test"; //��һ���Կ���ֹ���ַ�����ָ������������֡�

    if (!RegisterClassEx(&wndClass)) //����괰�ں���Ҫ�Դ��������ע�ᣬ�������ܴ��������͵Ĵ���
        return -1;

    HWND hwnd = CreateWindow(L"Test", WINDOW_TITLE, //�������ں���CreateWindow
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH,
        WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

    //Direct3D��Դ�ĳ�ʼ��������ʧ����messagebox������ʾ
    if (!(S_OK == Direct3D_Init(hwnd))) {
        MessageBox(hwnd, _T("Direct3D��ʼ��ʧ�ܣ�"), _T("Test"), 0); //ʹ��MessageBox����������һ����Ϣ����
    }

    MoveWindow(hwnd, 250, 80, WINDOW_WIDTH, WINDOW_HEIGHT, true); //����������ʾʱ��λ�ã�ʹ�������Ͻ�λ�ڣ�250,80����
    ShowWindow(hwnd, nShowCmd); //����ShowWindow��������ʾ����
    UpdateWindow(hwnd); //�Դ��ڽ��и���

    MSG msg = { 0 }; //��ʼ��msg
    while (msg.message != WM_QUIT) //ʹ��whileѭ��
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) //�鿴Ӧ�ó�����Ϣ���У�����Ϣʱ�������е���Ϣ�ɷ���ȥ��
        {
            TranslateMessage(&msg); //���������Ϣת��Ϊ�ַ���Ϣ
            DispatchMessage(&msg); //�ú����ַ�һ����Ϣ�����ڳ���
        } else {
            Direct3D_Render(hwnd); //������Ⱦ
        }
    }
    UnregisterClass(L"Test", wndClass.hInstance); //����׼��������ע��������
    return 0;
}

//WndProc( )����
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) //switch��俪ʼ
    {
    case WM_PAINT: // ���ǿͻ����ػ���Ϣ
        Direct3D_Render(hwnd); //����Direct3D��Ⱦ����
        ValidateRect(hwnd, NULL); // ���¿ͻ�������ʾ
        break; //������switch���

    case WM_KEYDOWN: // ���Ǽ��̰�����Ϣ
        if (wParam == VK_ESCAPE) // ��������µļ���ESC
            DestroyWindow(hwnd); // ���ٴ���, ������һ��WM_DESTROY��Ϣ
        break; //������switch���

    case WM_DESTROY: //���Ǵ���������Ϣ
        Direct3D_CleanUp(); //�����Զ������Դ������Game_CleanUp���������˳�ǰ����Դ����
        PostQuitMessage(0); //��ϵͳ�����и��߳�����ֹ����������ӦWM_DESTROY��Ϣ
        break; //������switch���

    default: //������case�����������ϣ���ִ�и�default���
        return DefWindowProc(hwnd, message, wParam, lParam); //����ȱʡ�Ĵ��ڹ���
    }

    return 0; //�����˳�
}

//Direct3D_Init( )����
HRESULT Direct3D_Init(HWND hwnd)
{
    D3D9Device d3d9(hwnd);
    g_pd3dDevice = d3d9.device9();

    if (S_OK != Objects_Init(hwnd))
        return E_FAIL; //����һ��Objects_Init��������Ⱦ��Դ�ĳ�ʼ��

    // ������Ⱦ״̬
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE); //�رչ���
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW); //������������

    return S_OK;
}

//Object_Init( )����
HRESULT Objects_Init(HWND hwnd)
{
    //��������
    if (FAILED(D3DXCreateFont(g_pd3dDevice, 36, 0, 0, 1, false, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, _T("΢���ź�"), &g_pFont)))
        return E_FAIL;
    srand(timeGetTime()); //��ϵͳʱ���ʼ���������

    //�������㻺��
    if (FAILED(g_pd3dDevice->CreateVertexBuffer(8 * sizeof(CUSTOMVERTEX),
            0, D3DFVF_CUSTOMVERTEX,
            D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL))) {
        return E_FAIL;
    }
    // ������������
    if (FAILED(g_pd3dDevice->CreateIndexBuffer(36 * sizeof(WORD), 0,
            D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIndexBuffer, NULL))) {
        return E_FAIL;
    }
    //�������ݵ����ã�
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

    //��䶥�㻺��
    VOID* pVertices;
    if (FAILED(g_pVertexBuffer->Lock(0, sizeof(Vertices), (void**)&pVertices, 0)))
        return E_FAIL;
    memcpy(pVertices, Vertices, sizeof(Vertices));
    g_pVertexBuffer->Unlock();

    // �����������
    WORD* pIndices = NULL;
    g_pIndexBuffer->Lock(0, 0, (void**)&pIndices, 0);

    // ����
    pIndices[0] = 0, pIndices[1] = 1, pIndices[2] = 2;
    pIndices[3] = 0, pIndices[4] = 2, pIndices[5] = 3;
    // ����
    pIndices[6] = 0, pIndices[7] = 3, pIndices[8] = 7;
    pIndices[9] = 0, pIndices[10] = 7, pIndices[11] = 4;
    // �����
    pIndices[12] = 0, pIndices[13] = 4, pIndices[14] = 5;
    pIndices[15] = 0, pIndices[16] = 5, pIndices[17] = 1;
    // �Ҳ���
    pIndices[18] = 2, pIndices[19] = 6, pIndices[20] = 7;
    pIndices[21] = 2, pIndices[22] = 7, pIndices[23] = 3;
    // ����
    pIndices[24] = 2, pIndices[25] = 5, pIndices[26] = 6;
    pIndices[27] = 2, pIndices[28] = 1, pIndices[29] = 5;
    // ����
    pIndices[30] = 4, pIndices[31] = 6, pIndices[32] = 5;
    pIndices[33] = 4, pIndices[34] = 7, pIndices[35] = 6;
    g_pIndexBuffer->Unlock();

    return S_OK;
}

//Matrix_Set( )����

VOID Matrix_Set()
{
    //����任���������
    D3DXMATRIX matWorld, Rx, Ry, Rz;
    D3DXMatrixIdentity(&matWorld); // ��λ���������
    D3DXMatrixRotationX(&Rx, D3DX_PI * (::timeGetTime() / 1000.0f / 3)); // ��X����ת
    D3DXMatrixRotationY(&Ry, D3DX_PI * (::timeGetTime() / 1000.0f / 3)); // ��Y����ת
    D3DXMatrixRotationZ(&Rz, D3DX_PI * (::timeGetTime() / 1000.0f / 3)); // ��Z����ת
    matWorld = Rx * Ry * Rz * matWorld; // �õ����յ���Ͼ���
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld); //��������任����

    //ȡ���任���������
    D3DXMATRIX matView; //����һ������
    D3DXVECTOR3 vEye(0.0f, 0.0f, -200.0f); //�������λ��
    D3DXVECTOR3 vAt(0.0f, 0.0f, 0.0f); //�۲���λ��
    D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f); //���ϵ�����
    D3DXMatrixLookAtLH(&matView, &vEye, &vAt, &vUp); //�����ȡ���任����
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView); //Ӧ��ȡ���任����

    //ͶӰ�任���������
    D3DXMATRIX matProj; //����һ������
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4.0f, 1.0f, 1.0f, 1000.0f); //����ͶӰ�任����
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj); //����ͶӰ�任����

    //�ӿڱ任������
    D3DVIEWPORT9 vp; //ʵ����һ��D3DVIEWPORT9�ṹ�壬Ȼ��������������������ֵ�Ϳ�����
    vp.X = 0; //��ʾ�ӿ�����ڴ��ڵ�X����
    vp.Y = 0; //�ӿ���ԶԴ��ڵ�Y����
    vp.Width = WINDOW_WIDTH; //�ӿڵĿ��
    vp.Height = WINDOW_HEIGHT; //�ӿڵĸ߶�
    vp.MinZ = 0.0f; //�ӿ�����Ȼ����е���С���ֵ
    vp.MaxZ = 1.0f; //�ӿ�����Ȼ����е�������ֵ
    g_pd3dDevice->SetViewport(&vp); //�ӿڵ�����
}

//Direct3D_Render( )����
void Direct3D_Render(HWND hwnd)
{
    // ��������
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    //����һ�����Σ����ڻ�ȡ�����ھ���
    RECT formatRect;
    GetClientRect(hwnd, &formatRect);
    //��ʼ����
    g_pd3dDevice->BeginScene(); // ��ʼ����

    Matrix_Set(); //���÷�װ���Ĵ�任�ĺ�������Direct3D����任��ȡ���任��ͶӰ�任���ӿڱ任��������
    // ��ȡ������Ϣ������������Ӧ�����ģʽ
    if (::GetAsyncKeyState(0x31) & 0x8000f) // �����ּ�1�����£������߿����
        g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    if (::GetAsyncKeyState(0x32) & 0x8000f) // �����ּ�2�����£�����ʵ�����
        g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    g_pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof(CUSTOMVERTEX)); //�Ѱ����ļ�������Ϣ�Ķ��㻺�����Ⱦ��ˮ�������
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); //ָ������ʹ�õ������ʽ�ĺ�����
    g_pd3dDevice->SetIndices(g_pIndexBuffer); //������������
    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12); //��������������϶��㻺�����ͼ��

    //�ڴ������ϽǴ�����ʾÿ��֡��
    int charCount = swprintf_s(g_strFPS, 20, _T("FPS:%0.3f"), Get_FPS());
    g_pFont->DrawText(NULL, g_strFPS, charCount, &formatRect, DT_TOP | DT_RIGHT, D3DCOLOR_XRGB(255, 39, 136));

    // ��������
    g_pd3dDevice->EndScene(); // ��������
    // ��ʾ��ת
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL); // ��ת����ʾ
}

//���ڼ���ÿ��֡���ʵ�һ������
float Get_FPS()
{
    //�����ĸ���̬����
    static float fps = 0; //������Ҫ�����FPSֵ
    static int frameCount = 0; //֡��
    static float currentTime = 0.0f; //��ǰʱ��
    static float lastTime = 0.0f; //����ʱ��

    frameCount++; //ÿ����һ��Get_FPS()������֡������1
    currentTime = timeGetTime() * 0.001f; //��ȡϵͳʱ�䣬����timeGetTime�������ص����Ժ���Ϊ��λ��ϵͳʱ�䣬������Ҫ����0.001���õ���λΪ���ʱ��

    //�����ǰʱ���ȥ����ʱ�������1���ӣ��ͽ���һ��FPS�ļ���ͳ���ʱ��ĸ��£�����֡��ֵ����
    if (currentTime - lastTime > 1.0f) //��ʱ�������1����
    {
        fps = (float)frameCount / (currentTime - lastTime); //������1���ӵ�FPSֵ
        lastTime = currentTime; //����ǰʱ��currentTime��������ʱ��lastTime����Ϊ��һ��Ļ�׼ʱ��
        frameCount = 0; //������֡��frameCountֵ����
    }

    return fps;
}

//��Դ���������ڴ˺����н��г����˳�ǰ��Դ��������
void Direct3D_CleanUp()
{
    //�ͷ�COM�ӿڶ���
    SAFE_RELEASE(g_pIndexBuffer)
    SAFE_RELEASE(g_pVertexBuffer)
    SAFE_RELEASE(g_pFont)
    SAFE_RELEASE(g_pd3dDevice)
}
