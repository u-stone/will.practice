#pragma once


class HookImpl
{
    HookImpl();
    ~HookImpl();
public:
    static HookImpl* instance();

    void SetTargetProcId(DWORD pid);

    void Install();
    void Uninstall();

private:
    void DoSth();

private:
    DWORD pid_ = NULL;
};

