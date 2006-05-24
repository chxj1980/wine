/*
 *	free threaded marshaller
 *
 *  Copyright 2002  Juergen Schmied
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "objbase.h"

#include "wine/debug.h"

#include "compobj_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(ole);

typedef struct _FTMarshalImpl {
	const IUnknownVtbl *lpVtbl;
	LONG ref;
	const IMarshalVtbl *lpvtblFTM;

	IUnknown *pUnkOuter;
} FTMarshalImpl;

#define _IFTMUnknown_(This)(IUnknown*)&(This->lpVtbl)
#define _IFTMarshal_(This) (IMarshal*)&(This->lpvtblFTM)

static inline FTMarshalImpl *impl_from_IMarshal( IMarshal *iface )
{
    return (FTMarshalImpl *)((char*)iface - FIELD_OFFSET(FTMarshalImpl, lpvtblFTM));
}

/* inner IUnknown to handle aggregation */
static HRESULT WINAPI
IiFTMUnknown_fnQueryInterface (IUnknown * iface, REFIID riid, LPVOID * ppv)
{

    FTMarshalImpl *This = (FTMarshalImpl *)iface;

    TRACE ("\n");
    *ppv = NULL;

    if (IsEqualIID (&IID_IUnknown, riid))
	*ppv = _IFTMUnknown_ (This);
    else if (IsEqualIID (&IID_IMarshal, riid))
	*ppv = _IFTMarshal_ (This);
    else {
	FIXME ("No interface for %s.\n", debugstr_guid (riid));
	return E_NOINTERFACE;
    }
    IUnknown_AddRef ((IUnknown *) * ppv);
    return S_OK;
}

static ULONG WINAPI IiFTMUnknown_fnAddRef (IUnknown * iface)
{

    FTMarshalImpl *This = (FTMarshalImpl *)iface;

    TRACE ("\n");
    return InterlockedIncrement (&This->ref);
}

static ULONG WINAPI IiFTMUnknown_fnRelease (IUnknown * iface)
{

    FTMarshalImpl *This = (FTMarshalImpl *)iface;

    TRACE ("\n");
    if (InterlockedDecrement (&This->ref))
	return This->ref;
    HeapFree (GetProcessHeap (), 0, This);
    return 0;
}

static const IUnknownVtbl iunkvt =
{
	IiFTMUnknown_fnQueryInterface,
	IiFTMUnknown_fnAddRef,
	IiFTMUnknown_fnRelease
};

static HRESULT WINAPI
FTMarshalImpl_QueryInterface (LPMARSHAL iface, REFIID riid, LPVOID * ppv)
{

    FTMarshalImpl *This = impl_from_IMarshal(iface);

    TRACE ("(%p)->(\n\tIID:\t%s,%p)\n", This, debugstr_guid (riid), ppv);
    return IUnknown_QueryInterface (This->pUnkOuter, riid, ppv);
}

static ULONG WINAPI
FTMarshalImpl_AddRef (LPMARSHAL iface)
{

    FTMarshalImpl *This = impl_from_IMarshal(iface);

    TRACE ("\n");
    return IUnknown_AddRef (This->pUnkOuter);
}

static ULONG WINAPI
FTMarshalImpl_Release (LPMARSHAL iface)
{

    FTMarshalImpl *This = impl_from_IMarshal(iface);

    TRACE ("\n");
    return IUnknown_Release (This->pUnkOuter);
}

static HRESULT WINAPI
FTMarshalImpl_GetUnmarshalClass (LPMARSHAL iface, REFIID riid, void *pv, DWORD dwDestContext,
						void *pvDestContext, DWORD mshlflags, CLSID * pCid)
{
    FIXME ("(), stub!\n");
    return S_OK;
}

static HRESULT WINAPI
FTMarshalImpl_GetMarshalSizeMax (LPMARSHAL iface, REFIID riid, void *pv, DWORD dwDestContext,
						void *pvDestContext, DWORD mshlflags, DWORD * pSize)
{

    IMarshal *pMarshal = NULL;
    HRESULT hres;

    FTMarshalImpl *This = impl_from_IMarshal(iface);

    FIXME ("(), stub!\n");

    /* if the marshalling happens inside the same process the interface pointer is
       copied between the apartments */
    if (dwDestContext == MSHCTX_INPROC || dwDestContext == MSHCTX_CROSSCTX) {
	*pSize = sizeof (This);
	return S_OK;
    }

    /* use the standard marshaller to handle all other cases */
    CoGetStandardMarshal (riid, pv, dwDestContext, pvDestContext, mshlflags, &pMarshal);
    hres = IMarshal_GetMarshalSizeMax (pMarshal, riid, pv, dwDestContext, pvDestContext, mshlflags, pSize);
    IMarshal_Release (pMarshal);
    return hres;
}

static HRESULT WINAPI
FTMarshalImpl_MarshalInterface (LPMARSHAL iface, IStream * pStm, REFIID riid, void *pv,
			       DWORD dwDestContext, void *pvDestContext, DWORD mshlflags)
{

    IMarshal *pMarshal = NULL;
    HRESULT hres;
    DWORD magic = 0x57dfd54d /* MEOW */;

    TRACE("(%p, %s, %p, 0x%lx, %p, 0x%lx)\n", pStm, debugstr_guid(riid), pv,
        dwDestContext, pvDestContext, mshlflags);

    /* if the marshalling happens inside the same process the interface pointer is
       copied between the apartments */
    if (dwDestContext == MSHCTX_INPROC || dwDestContext == MSHCTX_CROSSCTX) {
        void *object;
        DWORD constant = 0;
        GUID unknown_guid = { 0 };

        hres = IUnknown_QueryInterface((IUnknown *)pv, riid, &object);
        if (FAILED(hres))
            return hres;

        hres = IStream_Write (pStm, &mshlflags, sizeof (mshlflags), NULL);
        if (hres != S_OK) return STG_E_MEDIUMFULL;

        hres = IStream_Write (pStm, &object, sizeof (object), NULL);
        if (hres != S_OK) return STG_E_MEDIUMFULL;

        hres = IStream_Write (pStm, &constant, sizeof (constant), NULL);
        if (hres != S_OK) return STG_E_MEDIUMFULL;

        hres = IStream_Write (pStm, &unknown_guid, sizeof (unknown_guid), NULL);
        if (hres != S_OK) return STG_E_MEDIUMFULL;

        return S_OK;
    }

    /* FIXME: this isn't exactly corret. it looks like the standard marshaler
     * for native writes all of the OBJREF data into the stream, so we should
     * really rely on it to write this constant for us. however, we need a
     * constant to differentiate the outofproc data from the inproc data */
    hres = IStream_Write (pStm, &magic, sizeof (magic), NULL);
    if (hres != S_OK) return STG_E_MEDIUMFULL;

    /* use the standard marshaler to handle all other cases */
    CoGetStandardMarshal (riid, pv, dwDestContext, pvDestContext, mshlflags, &pMarshal);
    hres = IMarshal_MarshalInterface (pMarshal, pStm, riid, pv, dwDestContext, pvDestContext, mshlflags);
    IMarshal_Release (pMarshal);
    return hres;
}

static HRESULT WINAPI
FTMarshalImpl_UnmarshalInterface (LPMARSHAL iface, IStream * pStm, REFIID riid, void **ppv)
{
    DWORD mshlflags;
    HRESULT hres;

    TRACE ("(%p, %s, %p)\n", pStm, debugstr_guid(riid), ppv);

    hres = IStream_Read (pStm, &mshlflags, sizeof (mshlflags), NULL);
    if (hres != S_OK) return STG_E_READFAULT;

    if (mshlflags == 0x57dfd54d /* MEOW */) {
        IMarshal *pMarshal;

        hres = CoCreateInstance (&CLSID_DfMarshal, NULL, CLSCTX_INPROC, &IID_IMarshal, (void **)&pMarshal);
        if (FAILED(hres)) return hres;

        hres = IMarshal_UnmarshalInterface (pMarshal, pStm, riid, ppv);
        IMarshal_Release (pMarshal);
        return hres;
    }
    else {
        IUnknown *object;
        DWORD constant;
        GUID unknown_guid;

        hres = IStream_Read (pStm, &object, sizeof (object), NULL);
        if (hres != S_OK) return STG_E_READFAULT;

        hres = IStream_Read (pStm, &constant, sizeof (constant), NULL);
        if (hres != S_OK) return STG_E_READFAULT;
        if (constant != 0)
            FIXME("constant is 0x%lx instead of 0\n", constant);

        hres = IStream_Read (pStm, &unknown_guid, sizeof (unknown_guid), NULL);
        if (hres != S_OK) return STG_E_READFAULT;

        hres = IUnknown_QueryInterface(object, riid, ppv);
        IUnknown_Release(object);
        return hres;
    }
}

static HRESULT WINAPI FTMarshalImpl_ReleaseMarshalData (LPMARSHAL iface, IStream * pStm)
{
    FIXME ("(), stub!\n");
    return S_OK;
}

static HRESULT WINAPI FTMarshalImpl_DisconnectObject (LPMARSHAL iface, DWORD dwReserved)
{
    FIXME ("(), stub!\n");
    return S_OK;
}

static const IMarshalVtbl ftmvtbl =
{
	FTMarshalImpl_QueryInterface,
	FTMarshalImpl_AddRef,
	FTMarshalImpl_Release,
	FTMarshalImpl_GetUnmarshalClass,
	FTMarshalImpl_GetMarshalSizeMax,
	FTMarshalImpl_MarshalInterface,
	FTMarshalImpl_UnmarshalInterface,
	FTMarshalImpl_ReleaseMarshalData,
	FTMarshalImpl_DisconnectObject
};

/***********************************************************************
 *          CoCreateFreeThreadedMarshaler [OLE32.@]
 *
 */
HRESULT WINAPI CoCreateFreeThreadedMarshaler (LPUNKNOWN punkOuter, LPUNKNOWN * ppunkMarshal)
{

    FTMarshalImpl *ftm;

    TRACE ("(%p %p)\n", punkOuter, ppunkMarshal);

    ftm = HeapAlloc (GetProcessHeap (), 0, sizeof (FTMarshalImpl));
    if (!ftm)
	return E_OUTOFMEMORY;

    ftm->lpVtbl = &iunkvt;
    ftm->lpvtblFTM = &ftmvtbl;
    ftm->ref = 1;
    ftm->pUnkOuter = punkOuter ? punkOuter : _IFTMUnknown_(ftm);

    *ppunkMarshal = _IFTMUnknown_ (ftm);
    return S_OK;
}
