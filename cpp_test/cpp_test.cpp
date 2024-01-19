// cpp_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

class A {
public:
  A() {
    std::cout << "A ctor" << std::endl;
  }
  ~A() {
    std::cout << "A de-ctor" << std::endl;
  }
};

class B {
public:
  B() {
    std::cout << "B ctor" << std::endl;
  }
  ~B() {
    std::cout << "B de-ctor" << std::endl;
  }
};

class C {
public:
  C() {
    std::cout << "C ctor" << std::endl;
  }
  ~C() {
    std::cout << "C de-ctor" << std::endl;
  }

  B b;
  A a;
};

//int main()
//{
//  C c;
//  std::cout << "Hello World!\n";
//}

#if 0
#pragma comment(lib, "advapi32.lib")

#include <aclapi.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

int main()
{
    DWORD dwRes, dwDisposition;
    PSID pEveryoneSID = NULL, pAdminSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea[2];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;
    LONG lRes;
    HKEY hkSub = NULL;

    // Create a well-known SID for the Everyone group.
    if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pEveryoneSID)) {
        _tprintf(_T("AllocateAndInitializeSid Error %u\n"), GetLastError());
        goto Cleanup;
    }

    // Initialize an EXPLICIT_ACCESS structure for an ACE.
    // The ACE will allow Everyone read access to the key.
    ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
    ea[0].grfAccessPermissions = KEY_READ;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

    // Create a SID for the BUILTIN\Administrators group.
    if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pAdminSID)) {
        _tprintf(_T("AllocateAndInitializeSid Error %u\n"), GetLastError());
        goto Cleanup;
    }

    // Initialize an EXPLICIT_ACCESS structure for an ACE.
    // The ACE will allow the Administrators group full access to
    // the key.
    ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName = (LPTSTR)pAdminSID;

    // Create a new ACL that contains the new ACEs.
    dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
    if (ERROR_SUCCESS != dwRes) {
        _tprintf(_T("SetEntriesInAcl Error %u\n"), GetLastError());
        goto Cleanup;
    }

    // Initialize a security descriptor.
    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
        SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (NULL == pSD) {
        _tprintf(_T("LocalAlloc Error %u\n"), GetLastError());
        goto Cleanup;
    }

    if (!InitializeSecurityDescriptor(pSD,
            SECURITY_DESCRIPTOR_REVISION)) {
        _tprintf(_T("InitializeSecurityDescriptor Error %u\n"),
            GetLastError());
        goto Cleanup;
    }

    // Add the ACL to the security descriptor.
    if (!SetSecurityDescriptorDacl(pSD,
            TRUE, // bDaclPresent flag
            pACL,
            FALSE)) // not a default DACL
    {
        _tprintf(_T("SetSecurityDescriptorDacl Error %u\n"),
            GetLastError());
        goto Cleanup;
    }

    // Initialize a security attributes structure.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;

    // Use the security attributes to set the security descriptor
    // when you create a key.
    //{
    //  wchar_t class_str[] = _T("");
    //  lRes = RegCreateKeyEx(HKEY_CURRENT_USER, _T("mykey"), 0, class_str, 0,
    //    KEY_READ | KEY_WRITE, &sa, &hkSub, &dwDisposition);
    //  _tprintf(_T("RegCreateKeyEx result %u\n"), lRes);
    //}

Cleanup:

    if (pEveryoneSID)
        FreeSid(pEveryoneSID);
    if (pAdminSID)
        FreeSid(pAdminSID);
    if (pACL)
        LocalFree(pACL);
    if (pSD)
        LocalFree(pSD);
    if (hkSub)
        RegCloseKey(hkSub);

    return 0;
}
#endif

#include <Windows.h>

DWORD all_event[] = {
    EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_VALUECHANGE, EVENT_UIA_EVENTID_START, EVENT_OBJECT_SHOW, EVENT_OBJECT_FOCUS, EVENT_SYSTEM_MINIMIZEEND, EVENT_UIA_EVENTID_END, EVENT_OBJECT_DRAGLEAVE, EVENT_OBJECT_PARENTCHANGE, EVENT_OBJECT_CLOAKED, EVENT_OBJECT_HIDE, EVENT_SYSTEM_MENUPOPUPEND, EVENT_UIA_PROPID_END, EVENT_AIA_START, EVENT_SYSTEM_SWITCHEND, EVENT_OBJECT_HOSTEDOBJECTSINVALIDATED, EVENT_OEM_DEFINED_END, EVENT_SYSTEM_SCROLLINGEND, EVENT_SYSTEM_DESKTOPSWITCH, EVENT_OBJECT_DRAGENTER, EVENT_SYSTEM_ARRANGMENTPREVIEW, EVENT_OBJECT_DESCRIPTIONCHANGE, EVENT_OEM_DEFINED_START, EVENT_SYSTEM_CAPTUREEND, EVENT_OBJECT_LOCATIONCHANGE, EVENT_SYSTEM_CAPTURESTART, EVENT_SYSTEM_MENUEND, EVENT_UIA_PROPID_START, EVENT_OBJECT_SELECTION, EVENT_OBJECT_STATECHANGE, EVENT_SYSTEM_DRAGDROPEND, EVENT_SYSTEM_SOUND, EVENT_OBJECT_UNCLOAKED, EVENT_OBJECT_TEXTSELECTIONCHANGED, EVENT_MAX, EVENT_OBJECT_CREATE, EVENT_OBJECT_INVOKED, EVENT_SYSTEM_DIALOGEND, EVENT_OBJECT_IME_SHOW, EVENT_SYSTEM_CONTEXTHELPEND, EVENT_OBJECT_SELECTIONWITHIN, EVENT_OBJECT_IME_HIDE, EVENT_OBJECT_DRAGSTART, EVENT_SYSTEM_FOREGROUND, EVENT_OBJECT_SELECTIONREMOVE, EVENT_SYSTEM_MOVESIZEEND, EVENT_SYSTEM_MENUSTART, EVENT_OBJECT_REORDER, EVENT_SYSTEM_MENUPOPUPSTART, EVENT_SYSTEM_SCROLLINGSTART, EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_SWITCHSTART, EVENT_SYSTEM_DRAGDROPSTART, EVENT_OBJECT_DESTROY, EVENT_OBJECT_HELPCHANGE, EVENT_SYSTEM_DIALOGSTART, EVENT_OBJECT_DRAGCANCEL, EVENT_OBJECT_DRAGDROPPED, EVENT_OBJECT_ACCELERATORCHANGE, EVENT_AIA_END, EVENT_OBJECT_DRAGCOMPLETE, EVENT_OBJECT_CONTENTSCROLLED, EVENT_SYSTEM_ALERT, EVENT_OBJECT_DEFACTIONCHANGE, EVENT_OBJECT_IME_CHANGE, EVENT_OBJECT_SELECTIONADD, EVENT_OBJECT_END, EVENT_SYSTEM_CONTEXTHELPSTART, EVENT_SYSTEM_MOVESIZESTART, EVENT_MIN, EVENT_OBJECT_LIVEREGIONCHANGED, EVENT_SYSTEM_END, EVENT_OBJECT_TEXTEDIT_CONVERSIONTARGETCHANGED
};

// convert event id to string
const char* event_id_to_string(DWORD event_id)
{
    switch (event_id) {
    case EVENT_OBJECT_NAMECHANGE:
        return "EVENT_OBJECT_NAMECHANGE";
    case EVENT_OBJECT_VALUECHANGE:
        return "EVENT_OBJECT_VALUECHANGE";
    case EVENT_UIA_EVENTID_START:
        return "EVENT_UIA_EVENTID_START";
    case EVENT_OBJECT_SHOW:
        return "EVENT_OBJECT_SHOW";
    case EVENT_OBJECT_FOCUS:
        return "EVENT_OBJECT_FOCUS";
    case EVENT_SYSTEM_MINIMIZEEND:
        return "EVENT_SYSTEM_MINIMIZEEND";
    case EVENT_UIA_EVENTID_END:
        return "EVENT_UIA_EVENTID_END";
    case EVENT_OBJECT_DRAGLEAVE:
        return "EVENT_OBJECT_DRAGLEAVE";
    case EVENT_OBJECT_PARENTCHANGE:
        return "EVENT_OBJECT_PARENTCHANGE";
    case EVENT_OBJECT_CLOAKED:
        return "EVENT_OBJECT_CLOAKED";
    case EVENT_OBJECT_HIDE:
        return "EVENT_OBJECT_HIDE";
    case EVENT_SYSTEM_MENUPOPUPEND:
        return "EVENT_SYSTEM_MENUPOPUPEND";
    case EVENT_UIA_PROPID_END:
        return "EVENT_UIA_PROPID_END";
    case EVENT_AIA_START:
        return "EVENT_AIA_START";
    case EVENT_SYSTEM_SWITCHEND:
        return "EVENT_SYSTEM_SWITCHEND";
    case EVENT_OBJECT_HOSTEDOBJECTSINVALIDATED:
        return "EVENT_OBJECT_HOSTEDOBJECTSINVALIDATED";
    case EVENT_OEM_DEFINED_END:
        return "EVENT_OEM_DEFINED_END";
    case EVENT_SYSTEM_SCROLLINGEND:
        return "EVENT_SYSTEM_SCROLLINGEND";
    case EVENT_SYSTEM_DESKTOPSWITCH:
        return "EVENT_SYSTEM_DESKTOPSWITCH";
    case EVENT_OBJECT_DRAGENTER:
        return "EVENT_OBJECT_DRAGENTER";
    case EVENT_SYSTEM_ARRANGMENTPREVIEW:
        return "EVENT_SYSTEM_ARRANGMENTPREVIEW";
    case EVENT_OBJECT_DESCRIPTIONCHANGE:
        return "EVENT_OBJECT_DESCRIPTIONCHANGE";
    case EVENT_OEM_DEFINED_START:
        return "EVENT_OEM_DEFINED_START";
    case EVENT_SYSTEM_CAPTUREEND:
        return "EVENT_SYSTEM_CAPTUREEND";
    case EVENT_OBJECT_LOCATIONCHANGE:
        return "EVENT_OBJECT_LOCATIONCHANGE";
    case EVENT_SYSTEM_CAPTURESTART:
        return "EVENT_SYSTEM_CAPTURESTART";
    case EVENT_SYSTEM_MENUEND:
        return "EVENT_SYSTEM_MENUEND";
    case EVENT_UIA_PROPID_START:
        return "EVENT_UIA_PROPID_START";
    case EVENT_OBJECT_SELECTION:
        return "EVENT_OBJECT_SELECTION";
    case EVENT_OBJECT_STATECHANGE:
        return "EVENT_OBJECT_STATECHANGE";
    case EVENT_SYSTEM_DRAGDROPEND:
        return "EVENT_SYSTEM_DRAGDROPEND";
        // case EVENT_SYSTEM_SOUND:
        //     return "EVENT_SYSTEM_SOUND";
    case EVENT_OBJECT_UNCLOAKED:
        return "EVENT_OBJECT_UNCLOAKED";
        // case EVENT_SYSTEM_SCROLLEND:
        //     return "EVENT_SYSTEM_SCROLLEND";
    case EVENT_OBJECT_TEXTSELECTIONCHANGED:
        return "EVENT_OBJECT_TEXTSELECTIONCHANGED";
    case EVENT_MAX:
        return "EVENT_MAX";
    case EVENT_OBJECT_CREATE:
        return "EVENT_OBJECT_CREATE";
    case EVENT_OBJECT_INVOKED:
        return "EVENT_OBJECT_INVOKED";
    case EVENT_SYSTEM_DIALOGEND:
        return "EVENT_SYSTEM_DIALOGEND";
    case EVENT_OBJECT_IME_SHOW:
        return "EVENT_OBJECT_IME_SHOW";
    case EVENT_SYSTEM_CONTEXTHELPEND:
        return "EVENT_SYSTEM_CONTEXTHELPEND";
    case EVENT_OBJECT_SELECTIONWITHIN:
        return "EVENT_OBJECT_SELECTIONWITHIN";
    case EVENT_OBJECT_IME_HIDE:
        return "EVENT_OBJECT_IME_HIDE";
    case EVENT_OBJECT_DRAGSTART:
        return "EVENT_OBJECT_DRAGSTART";
    case EVENT_SYSTEM_FOREGROUND:
        return "EVENT_SYSTEM_FOREGROUND";
    case EVENT_OBJECT_SELECTIONREMOVE:
        return "EVENT_OBJECT_SELECTIONREMOVE";
    case EVENT_SYSTEM_MOVESIZEEND:
        return "EVENT_SYSTEM_MOVESIZEEND";
    case EVENT_SYSTEM_MENUSTART:
        return "EVENT_SYSTEM_MENUSTART";
    case EVENT_OBJECT_REORDER:
        return "EVENT_OBJECT_REORDER";
    case EVENT_SYSTEM_MENUPOPUPSTART:
        return "EVENT_SYSTEM_MENUPOPUPSTART";
    case EVENT_SYSTEM_SCROLLINGSTART:
        return "EVENT_SYSTEM_SCROLLINGSTART";
    case EVENT_SYSTEM_MINIMIZESTART:
        return "EVENT_SYSTEM_MINIMIZESTART";
    case EVENT_SYSTEM_SWITCHSTART:
        return "EVENT_SYSTEM_SWITCHSTART";
    case EVENT_SYSTEM_DRAGDROPSTART:
        return "EVENT_SYSTEM_DRAGDROPSTART";
    case EVENT_OBJECT_DESTROY:
        return "EVENT_OBJECT_DESTROY";
    case EVENT_OBJECT_HELPCHANGE:
        return "EVENT_OBJECT_HELPCHANGE";
    case EVENT_SYSTEM_DIALOGSTART:
        return "EVENT_SYSTEM_DIALOGSTART";
    case EVENT_OBJECT_DRAGCANCEL:
        return "EVENT_OBJECT_DRAGCANCEL";
    case EVENT_OBJECT_DRAGDROPPED:
        return "EVENT_OBJECT_DRAGDROPPED";
    case EVENT_OBJECT_ACCELERATORCHANGE:
        return "EVENT_OBJECT_ACCELERATORCHANGE";
    case EVENT_AIA_END:
        return "EVENT_AIA_END";
    case EVENT_OBJECT_DRAGCOMPLETE:
        return "EVENT_OBJECT_DRAGCOMPLETE";
    case EVENT_OBJECT_CONTENTSCROLLED:
        return "EVENT_OBJECT_CONTENTSCROLLED";
    case EVENT_SYSTEM_ALERT:
        return "EVENT_SYSTEM_ALERT";
    case EVENT_OBJECT_DEFACTIONCHANGE:
        return "EVENT_OBJECT_DEFACTIONCHANGE";
    case EVENT_OBJECT_IME_CHANGE:
        return "EVENT_OBJECT_IME_CHANGE";
    case EVENT_OBJECT_SELECTIONADD:
        return "EVENT_OBJECT_SELECTIONADD";
    case EVENT_OBJECT_END:
        return "EVENT_OBJECT_END";
    case EVENT_SYSTEM_CONTEXTHELPSTART:
        return "EVENT_SYSTEM_CONTEXTHELPSTART";
    case EVENT_SYSTEM_MOVESIZESTART:
        return "EVENT_SYSTEM_MOVESIZESTART";
    case EVENT_MIN:
        return "EVENT_MIN";
    case EVENT_OBJECT_LIVEREGIONCHANGED:
        return "EVENT_OBJECT_LIVEREGIONCHANGED";
    case EVENT_SYSTEM_END:
        return "EVENT_SYSTEM_END";
    case EVENT_OBJECT_TEXTEDIT_CONVERSIONTARGETCHANGED:
      return "EVENT_OBJECT_TEXTEDIT_CONVERSIONTARGETCHANGED";
    default:
        return "UNKNOWN";
    }
}
#include <thread>
int main()
{
  std::shared_ptr<int> ptr = std::make_shared<int>(2011);

  for (auto i = 0; i < 10; i++) {
    std::thread([&ptr] {                         
      ptr = std::make_shared<int>(2014);         
    }).detach();
  }
    // get all event id
    //DWORD event_id = 0;
    //for (auto event : all_event) {
    //  std::cout << event_id_to_string(event) << std::endl;
    //}
    //while (event_id < EVENT_MAX) {
    //    std::cout << event_id_to_string(event_id) << std::endl;
    //    event_id++;
    //}
    return 0;
}
