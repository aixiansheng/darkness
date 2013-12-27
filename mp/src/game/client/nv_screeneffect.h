#ifndef GE_SCREENSPACEEFFECTS_H
#define GE_SCREENSPACEEFFECTS_H
#ifdef _WIN32
#pragma once
#endif
 
#include "ScreenSpaceEffects.h"
 
class C_BugVision : public IScreenSpaceEffect
{
public:
        C_BugVision();
        ~C_BugVision();
 
        void Init();
        void Shutdown();
 
        void SetParameters( KeyValues *params );
 
        void Render(int x, int y, int w, int h);
 
        void Enable(bool bEnable);
        bool IsEnabled();
 
private:
        bool m_bEnable;
        CMaterialReference m_Material;
};

#endif
