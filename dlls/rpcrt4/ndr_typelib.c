/*
 * Type library proxy/stub implementation
 *
 * Copyright 2018 Zebediah Figura
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

#define COBJMACROS
#include "oaidl.h"
#include "rpcproxy.h"
#include "wine/debug.h"
#include "wine/heap.h"

#include "cpsf.h"

WINE_DEFAULT_DEBUG_CHANNEL(ole);

struct typelib_proxy
{
    StdProxyImpl proxy;
};

static ULONG WINAPI typelib_proxy_Release(IRpcProxyBuffer *iface)
{
    struct typelib_proxy *This = CONTAINING_RECORD(iface, struct typelib_proxy, proxy.IRpcProxyBuffer_iface);
    ULONG refcount = InterlockedDecrement(&This->proxy.RefCount);

    TRACE("(%p) decreasing refs to %d\n", This, refcount);

    if (!refcount)
    {
        heap_free(This);
    }
    return refcount;
}

static const IRpcProxyBufferVtbl typelib_proxy_vtbl =
{
    StdProxy_QueryInterface,
    StdProxy_AddRef,
    typelib_proxy_Release,
    StdProxy_Connect,
    StdProxy_Disconnect,
};

static HRESULT typelib_proxy_init(struct typelib_proxy *This, IUnknown *outer,
    IRpcProxyBuffer **proxy, void **obj)
{
    if (!outer) outer = (IUnknown *)&This->proxy;

    This->proxy.IRpcProxyBuffer_iface.lpVtbl = &typelib_proxy_vtbl;
    This->proxy.RefCount = 1;
    This->proxy.pUnkOuter = outer;

    *proxy = &This->proxy.IRpcProxyBuffer_iface;

    return E_NOTIMPL;
}

HRESULT WINAPI CreateProxyFromTypeInfo(ITypeInfo *typeinfo, IUnknown *outer,
    REFIID iid, IRpcProxyBuffer **proxy, void **obj)
{
    struct typelib_proxy *This;
    HRESULT hr;

    TRACE("typeinfo %p, outer %p, iid %s, proxy %p, obj %p.\n",
        typeinfo, outer, debugstr_guid(iid), proxy, obj);

    if (!(This = heap_alloc_zero(sizeof(*This))))
        return E_OUTOFMEMORY;

    hr = typelib_proxy_init(This, outer, proxy, obj);
    if (FAILED(hr))
        heap_free(This);

    return hr;
}

struct typelib_stub
{
    CStdStubBuffer stub;
};

static HRESULT typelib_stub_init(struct typelib_stub *This,
    IUnknown *server, IRpcStubBuffer **stub)
{
    This->stub.RefCount = 1;
    *stub = (IRpcStubBuffer *)&This->stub;

    return E_NOTIMPL;
}

HRESULT WINAPI CreateStubFromTypeInfo(ITypeInfo *typeinfo, REFIID iid,
    IUnknown *server, IRpcStubBuffer **stub)
{
    struct typelib_stub *This;
    HRESULT hr;

    TRACE("typeinfo %p, iid %s, server %p, stub %p.\n",
        typeinfo, debugstr_guid(iid), server, stub);

    if (!(This = heap_alloc_zero(sizeof(*This))))
        return E_OUTOFMEMORY;

    hr = typelib_stub_init(This, server, stub);
    if (FAILED(hr))
        heap_free(This);

    return hr;
}
