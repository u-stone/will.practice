#pragma once
#include "framework.h"

class CMyDShow
{
public:
  CMyDShow();
  ~CMyDShow();

  HRESULT InitCaptureGraphBuilder(
    IGraphBuilder **ppGraph,  // Receives the pointer.
    ICaptureGraphBuilder2 **ppBuild  // Receives the pointer.
  );
private:

};

