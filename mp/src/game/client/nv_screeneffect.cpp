
#include "cbase.h"
#include "ScreenSpaceEffects.h"
#include "rendertexture.h"
#include "model_types.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "cdll_client_int.h"
#include "materialsystem/itexture.h"
#include "KeyValues.h"
#include "clienteffectprecachesystem.h"
#include "tier0/vprof.h"
#include "viewrender.h"
#include "view_scene.h"
#include "nv_screeneffect.h"


ADD_SCREENSPACE_EFFECT(C_BugVision, bugvision);

C_BugVision::C_BugVision() {
        m_bEnable = false;
}
 
C_BugVision::~C_BugVision(){
}

void C_BugVision::Init() {
        // This is just example code, init your effect material here
        //m_Material.Init( "engine/exampleeffect", TEXTURE_GROUP_OTHER );
        m_Material.Init(materials->FindMaterial("nightvision", TEXTURE_GROUP_OTHER, true)); //TEXTURE_GROUP_PIXEL_SHADERS
        m_bEnable = false;
}
 
void C_BugVision::Shutdown() {
        m_Material.Shutdown();
}

void C_BugVision::Enable(bool bEnable) {
        // This is just example code, don't enable it
        m_bEnable = bEnable;
}
 
bool C_BugVision::IsEnabled() {
        return m_bEnable;
}

void C_BugVision::SetParameters(KeyValues *params) {
        /*if( params->GetDataType( "example_param" ) == KeyValues::TYPE_STRING )
        {
                // ...
        }*/
}

void C_BugVision::Render(int x, int y, int w, int h) {
        if (!IsEnabled())
                return;
 
        // Render Effect
        /*Rect_t actualRect;
        UpdateScreenEffectTexture( 0, x, y, w, h, false, &actualRect );
        ITexture *pTexture = GetFullFrameFrameBufferTexture( 0 );
 
        CMatRenderContextPtr pRenderContext(materials);
 
        pRenderContext->DrawScreenSpaceRectangle(m_Material, x, y, w, h,
                actualRect.x, actualRect.y, actualRect.x+actualRect.width-1, actualRect.y+actualRect.height-1,
                pTexture->GetActualWidth(), pTexture->GetActualHeight() );*/
 
        //UpdateScreenEffectTexture();
        Rect_t actualRect;
        UpdateScreenEffectTexture(0, x, y, w, h, false, &actualRect);
        CMatRenderContextPtr pRenderContext(materials);
        ITexture *pTexture = GetFullFrameFrameBufferTexture(0);
 
        pRenderContext->MatrixMode(MATERIAL_PROJECTION);
        pRenderContext->PushMatrix();
        pRenderContext->LoadIdentity();
 
        pRenderContext->MatrixMode(MATERIAL_VIEW);
        pRenderContext->PushMatrix();
        pRenderContext->LoadIdentity();
 
        //IMaterial *pMaterial = materials->FindMaterial("herpshaders/bugvision", TEXTURE_GROUP_CLIENT_EFFECTS, true);
        //pRenderContext->DrawScreenSpaceQuad(pMaterial);
        pRenderContext->DrawScreenSpaceRectangle(m_Material, x, y, w, h, actualRect.x, actualRect.y, actualRect.x+actualRect.width-1, actualRect.y + actualRect.height - 1, pTexture->GetActualWidth(), pTexture->GetActualHeight());
 
        pRenderContext->MatrixMode(MATERIAL_PROJECTION);
        pRenderContext->PopMatrix();
        pRenderContext->MatrixMode(MATERIAL_VIEW);
        pRenderContext->PopMatrix();
}
 
static void OnNightVision(void) {
        IScreenSpaceEffect *pEffect = g_pScreenSpaceEffects->GetScreenSpaceEffect("bugvision");
       
        if(pEffect) {
                pEffect->Enable(!pEffect->IsEnabled());
        }
}
 
//static ConCommand sbr_nightvision("sbr_nightvision", NightVision_f);
//static ConCommand sbr_nightvision("sbr_nightvision", OnNightVision, "Toggles night vision.", FCVAR_CLIENTCMD_CAN_EXECUTE|FCVAR_SERVER_CAN_EXECUTE|FCVAR_DEMO);
