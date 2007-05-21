/* $Id$
 *
 * COPYRIGHT:            See COPYING in the top level directory
 * PROJECT:              ReactOS DirectX
 * FILE:                 ddraw/ddraw/ddraw_displaymode.c
 * PURPOSE:              IDirectDraw7 Implementation
 * PROGRAMMER:           Maarten Bosma
 *
 */


#include "rosdraw.h"

HRESULT WINAPI 
Main_DirectDraw_EnumDisplayModes(LPDIRECTDRAW7 iface, DWORD dwFlags,
                                  LPDDSURFACEDESC2 pDDSD, LPVOID pContext, LPDDENUMMODESCALLBACK2 pCallback)
{
    LPDDRAWI_DIRECTDRAW_INT This = (LPDDRAWI_DIRECTDRAW_INT)iface;
    DX_WINDBG_trace();

    if(!pCallback)
        return DDERR_INVALIDPARAMS;

    // FIXME: Process DDEDM_STANDARDVGAMODES flag

    int i;
    for (i=0; i<This->lpLcl->lpGbl->dwNumModes; i++)
    {
        DDSURFACEDESC2 SurfaceDesc; 

        // FIXME: do we need to set/test more sturcture members ?
        SurfaceDesc.dwSize = sizeof (DDSURFACEDESC2);
        SurfaceDesc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_REFRESHRATE | DDSD_WIDTH | DDSD_PIXELFORMAT;
        SurfaceDesc.dwHeight = This->lpLcl->lpGbl->lpModeInfo[i].dwWidth;
        SurfaceDesc.dwWidth = This->lpLcl->lpGbl->lpModeInfo[i].dwHeight;
        SurfaceDesc.lPitch = This->lpLcl->lpGbl->lpModeInfo[i].lPitch;
        SurfaceDesc.dwRefreshRate = This->lpLcl->lpGbl->lpModeInfo[i].wRefreshRate;

        SurfaceDesc.ddpfPixelFormat.dwSize = sizeof (DDPIXELFORMAT);
        SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
        SurfaceDesc.ddpfPixelFormat.dwRBitMask = This->lpLcl->lpGbl->lpModeInfo[i].dwRBitMask;
        SurfaceDesc.ddpfPixelFormat.dwGBitMask = This->lpLcl->lpGbl->lpModeInfo[i].dwGBitMask;
        SurfaceDesc.ddpfPixelFormat.dwBBitMask = This->lpLcl->lpGbl->lpModeInfo[i].dwBBitMask;
        SurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask = This->lpLcl->lpGbl->lpModeInfo[i].dwAlphaBitMask;
        SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = This->lpLcl->lpGbl->lpModeInfo[i].dwBPP;

        if(dwFlags & DDEDM_REFRESHRATES && SurfaceDesc.dwRefreshRate != This->lpLcl->lpGbl->dwMonitorFrequency)
        {
            continue;
        }

        if(pDDSD)
        {
            if(pDDSD->dwFlags & DDSD_HEIGHT && pDDSD->dwHeight != SurfaceDesc.dwHeight)
                continue;

            else if(pDDSD->dwFlags & DDSD_WIDTH && pDDSD->dwWidth != SurfaceDesc.dwWidth)
                continue;

            else if(pDDSD->dwFlags & DDSD_PITCH && pDDSD->lPitch != SurfaceDesc.lPitch)
                continue;

            else if(pDDSD->dwFlags & DDSD_REFRESHRATE && pDDSD->dwRefreshRate != SurfaceDesc.dwRefreshRate)
                continue;

            else if(pDDSD->dwFlags & DDSD_PIXELFORMAT && pDDSD->ddpfPixelFormat.dwRGBBitCount != SurfaceDesc.ddpfPixelFormat.dwRGBBitCount)
                continue;  // FIXME: test for the other members of ddpfPixelFormat as well
        }

        (*pCallback)(&SurfaceDesc, pContext);
    }

    return DD_OK;
}

HRESULT WINAPI
Main_DirectDraw_SetDisplayMode (LPDIRECTDRAW7 iface, DWORD dwWidth, DWORD dwHeight,
                                                                DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
    LPDDRAWI_DIRECTDRAW_INT This = (LPDDRAWI_DIRECTDRAW_INT)iface;
    DX_WINDBG_trace();

    // FIXME: Check primary if surface is locked / busy etc.

    // Check Parameters
    if(dwFlags != 0)
    {
        return DDERR_INVALIDPARAMS;
    }

    if ((!dwHeight || This->lpLcl->lpGbl->vmiData.dwDisplayHeight == dwHeight) && 
        (!dwWidth || This->lpLcl->lpGbl->vmiData.dwDisplayWidth == dwWidth)  && 
        (!dwBPP || This->lpLcl->lpGbl->vmiData.ddpfDisplay.dwRGBBitCount == dwBPP) &&
        (!dwRefreshRate || This->lpLcl->lpGbl->dwMonitorFrequency == dwRefreshRate))  
    {
        return DD_OK; // nothing to do here for us
    }

    // Here we go
    DEVMODE DevMode;
    DevMode.dmFields = 0;
    if(dwHeight) 
        DevMode.dmFields |= DM_PELSHEIGHT;
    if(dwWidth) 
        DevMode.dmFields |= DM_PELSWIDTH;
    if(dwBPP) 
        DevMode.dmFields |= DM_BITSPERPEL;
    if(dwRefreshRate) 
        DevMode.dmFields |= DM_DISPLAYFREQUENCY;

    DevMode.dmPelsHeight = dwHeight;
    DevMode.dmPelsWidth = dwWidth;
    DevMode.dmBitsPerPel = dwBPP;
    DevMode.dmDisplayFrequency = dwRefreshRate;

    LONG retval = ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN); /* FIXME: Are we supposed to set CDS_SET_PRIMARY as well ? */

    if(retval == DISP_CHANGE_BADMODE)
    {
        return DDERR_UNSUPPORTED;
    }
    else if(retval != DISP_CHANGE_SUCCESSFUL)
    {
        return DDERR_GENERIC;
    }

    // Update Interals
    BOOL ModeChanged;
    DdReenableDirectDrawObject(This->lpLcl->lpGbl, &ModeChanged);
    StartDirectDraw((LPDIRECTDRAW)iface, 0, TRUE);

    return DD_OK;
}

HRESULT WINAPI
Main_DirectDraw_RestoreDisplayMode (LPDIRECTDRAW7 iface)
{
    LPDDRAWI_DIRECTDRAW_INT This = (LPDDRAWI_DIRECTDRAW_INT)iface;
    DX_WINDBG_trace();

    ChangeDisplaySettings(NULL, 0);

    // Update Interals
    BOOL ModeChanged;
    DdReenableDirectDrawObject(This->lpLcl->lpGbl, &ModeChanged);
    StartDirectDraw((LPDIRECTDRAW)iface, 0, TRUE);

    return DD_OK;
}

HRESULT WINAPI
Main_DirectDraw_GetMonitorFrequency (LPDIRECTDRAW7 iface, LPDWORD lpFreq)
{
    LPDDRAWI_DIRECTDRAW_INT This = (LPDDRAWI_DIRECTDRAW_INT)iface;
    DX_WINDBG_trace();

    if (lpFreq == NULL)
        return DDERR_INVALIDPARAMS;

    *lpFreq = This->lpLcl->lpGbl->dwMonitorFrequency;

    return DD_OK;
}

HRESULT WINAPI
Main_DirectDraw_GetDisplayMode (LPDIRECTDRAW7 iface, LPDDSURFACEDESC2 pDDSD)
{
    LPDDRAWI_DIRECTDRAW_INT This = (LPDDRAWI_DIRECTDRAW_INT)iface;
    DX_WINDBG_trace();

    if (pDDSD == NULL)
        return DDERR_INVALIDPARAMS;

    if (pDDSD->dwSize != sizeof(LPDDSURFACEDESC2))
        return DDERR_INVALIDPARAMS;

    // FIXME: More stucture members might need to be filled

    pDDSD->dwFlags |= DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_PITCH | DDSD_REFRESHRATE;
    pDDSD->dwHeight = This->lpLcl->lpGbl->vmiData.dwDisplayWidth;
    pDDSD->dwWidth = This->lpLcl->lpGbl->vmiData.dwDisplayHeight;
    pDDSD->ddpfPixelFormat = This->lpLcl->lpGbl->vmiData.ddpfDisplay;
    pDDSD->dwRefreshRate = This->lpLcl->lpGbl->dwMonitorFrequency;
    pDDSD->lPitch = This->lpLcl->lpGbl->vmiData.lDisplayPitch;

    return DD_OK;
}
