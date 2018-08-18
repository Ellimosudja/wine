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

#include <assert.h>

#define COBJMACROS
#include "oaidl.h"
#define USE_STUBLESS_PROXY
#include "rpcproxy.h"
#include "ndrtypes.h"
#include "wine/debug.h"
#include "wine/heap.h"

#include "cpsf.h"

WINE_DEFAULT_DEBUG_CHANNEL(ole);

static size_t write_type_tfs(ITypeInfo *typeinfo, unsigned char *str,
    size_t *len, TYPEDESC *desc, BOOL toplevel, BOOL onstack);

#define WRITE_CHAR(str, len, val) \
    do { if ((str)) (str)[(len)] = (val); (len)++; } while (0)
#define WRITE_SHORT(str, len, val) \
    do { if ((str)) *((short *)((str) + (len))) = (val); (len) += 2; } while (0)
#define WRITE_INT(str, len, val) \
    do { if ((str)) *((int *)((str) + (len))) = (val); (len) += 4; } while (0)

static unsigned char get_base_type(VARTYPE vt)
{
    switch (vt)
    {
    case VT_I1:     return FC_SMALL;
    case VT_BOOL:
    case VT_I2:     return FC_SHORT;
    case VT_INT:
    case VT_ERROR:
    case VT_HRESULT:
    case VT_I4:     return FC_LONG;
    case VT_I8:
    case VT_UI8:    return FC_HYPER;
    case VT_UI1:    return FC_USMALL;
    case VT_UI2:    return FC_USHORT;
    case VT_UINT:
    case VT_UI4:    return FC_ULONG;
    case VT_R4:     return FC_FLOAT;
    case VT_DATE:
    case VT_R8:     return FC_DOUBLE;
    default:        return 0;
    }
}

static unsigned int type_memsize(ITypeInfo *typeinfo, TYPEDESC *desc)
{
    switch (desc->vt)
    {
    case VT_I1:
    case VT_UI1:
        return 1;
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
        return 2;
    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
    case VT_HRESULT:
        return 4;
    case VT_I8:
    case VT_UI8:
    case VT_R8:
    case VT_DATE:
        return 8;
    case VT_BSTR:
    case VT_SAFEARRAY:
    case VT_PTR:
    case VT_UNKNOWN:
    case VT_DISPATCH:
        return sizeof(void *);
    case VT_VARIANT:
        return sizeof(VARIANT);
    case VT_CARRAY:
    {
        unsigned int size = type_memsize(typeinfo, &desc->lpadesc->tdescElem);
        unsigned int i;
        for (i = 0; i < desc->lpadesc->cDims; i++)
            size *= desc->lpadesc->rgbounds[i].cElements;
        return size;
    }
    case VT_USERDEFINED:
    {
        unsigned int size = 0;
        ITypeInfo *refinfo;
        TYPEATTR *attr;

        ITypeInfo_GetRefTypeInfo(typeinfo, desc->hreftype, &refinfo);
        ITypeInfo_GetTypeAttr(refinfo, &attr);
        size = attr->cbSizeInstance;
        ITypeInfo_ReleaseTypeAttr(refinfo, attr);
        ITypeInfo_Release(refinfo);
        return size;
    }
    default:
        FIXME("unhandled type %u\n", desc->vt);
        return 0;
    }
}

static unsigned char get_array_fc(ITypeInfo *typeinfo, TYPEDESC *desc);

static unsigned char get_struct_fc(ITypeInfo *typeinfo, TYPEATTR *attr)
{
    unsigned char fc = FC_STRUCT;
    VARDESC *desc;
    VARTYPE vt;
    WORD i;

    for (i = 0; i < attr->cVars; i++)
    {
        ITypeInfo_GetVarDesc(typeinfo, i, &desc);
        vt = desc->elemdescVar.tdesc.vt;

        switch (vt)
        {
        case VT_CARRAY:
            if (get_array_fc(typeinfo, &desc->elemdescVar.tdesc.lpadesc->tdescElem) == FC_BOGUS_ARRAY)
                fc = FC_BOGUS_STRUCT;
            break;
        default:
            if (!get_base_type(vt))
            {
                FIXME("unhandled type %u\n", vt);
                fc = FC_BOGUS_STRUCT;
            }
            break;
        }

        ITypeInfo_ReleaseVarDesc(typeinfo, desc);
    }

    return fc;
}

static unsigned char get_array_fc(ITypeInfo *typeinfo, TYPEDESC *desc)
{
    if (get_base_type(desc->vt))
        return FC_LGFARRAY;
    else if (desc->vt == VT_USERDEFINED)
    {
        ITypeInfo *refinfo;
        TYPEATTR *attr;
        unsigned char fc;

        ITypeInfo_GetRefTypeInfo(typeinfo, desc->hreftype, &refinfo);
        ITypeInfo_GetTypeAttr(refinfo, &attr);

        if (attr->typekind == TKIND_ENUM)
            fc = FC_LGFARRAY;
        else if (attr->typekind == TKIND_RECORD && get_struct_fc(refinfo, attr) == FC_STRUCT)
            fc = FC_LGFARRAY;
        else
            fc = FC_BOGUS_ARRAY;

        ITypeInfo_ReleaseTypeAttr(refinfo, attr);
        ITypeInfo_Release(refinfo);

        return fc;
    }
    else
        return FC_BOGUS_ARRAY;
}

static size_t write_struct_tfs(ITypeInfo *typeinfo, unsigned char *str,
    size_t *len, TYPEATTR *attr)
{
    /* The only reason to bother writing the embedded types is for endianness
     * conversion. Since Wine doesn't support that, don't. */

    unsigned char fc = get_struct_fc(typeinfo, attr);
    size_t off = *len;

    if (fc != FC_STRUCT)
        FIXME("fc %02x not implemented\n", fc);

    WRITE_CHAR (str, *len, fc);
    WRITE_CHAR (str, *len, attr->cbAlignment - 1);
    WRITE_SHORT(str, *len, attr->cbSizeInstance);
    WRITE_CHAR (str, *len, FC_PAD);
    WRITE_CHAR (str, *len, FC_END);

    return off;
}

static size_t write_array_tfs(ITypeInfo *typeinfo, unsigned char *str,
    size_t *len, ARRAYDESC *desc)
{
    unsigned char fc = get_array_fc(typeinfo, &desc->tdescElem);
    ULONG size = type_memsize(typeinfo, &desc->tdescElem);
    unsigned char basetype;
    size_t ref = 0, off;
    USHORT i;

    if (fc != FC_LGFARRAY)
        FIXME("complex arrays not implemented\n");

    if (!(basetype = get_base_type(desc->tdescElem.vt)))
        ref = write_type_tfs(typeinfo, str, len, &desc->tdescElem, FALSE, FALSE);

    /* In theory arrays should be nested, but there's no reason not to marshal
     * [x][y] as [x*y]. */
    for (i = 0; i < desc->cDims; i++) size *= desc->rgbounds[i].cElements;

    off = *len;

    WRITE_CHAR(str, *len, FC_LGFARRAY);
    WRITE_CHAR(str, *len, 0);
    WRITE_INT (str, *len, size);
    if (basetype)
        WRITE_CHAR(str, *len, basetype);
    else
    {
        WRITE_CHAR (str, *len, FC_EMBEDDED_COMPLEX);
        WRITE_CHAR (str, *len, 0);
        WRITE_SHORT(str, *len, ref - *len);
        WRITE_CHAR (str, *len, FC_PAD);
    }
    WRITE_CHAR(str, *len, FC_END);

    return off;
}

static size_t write_ip_tfs(unsigned char *str, size_t *len, const GUID *iid)
{
    size_t off = *len;

    if (str)
    {
        str[*len] = FC_IP;
        str[*len+1] = FC_CONSTANT_IID;
        memcpy(str + *len + 2, iid, sizeof(*iid));
    }
    *len += 2 + sizeof(*iid);

    return off;
}

static size_t write_pointer_tfs(unsigned char *str, size_t *len, VARTYPE vt,
    size_t ref, BOOL toplevel, BOOL onstack)
{
    unsigned char basetype, flags = 0;
    size_t off = *len;

    if ((basetype = get_base_type(vt)))
    {
        assert(!toplevel); /* toplevel base-type pointers should use IsSimpleRef */
        WRITE_CHAR(str, *len, FC_UP);
        WRITE_CHAR(str, *len, FC_SIMPLE_POINTER);
        WRITE_CHAR(str, *len, basetype);
        WRITE_CHAR(str, *len, FC_PAD);
    }
    else
    {
        if (onstack) flags |= FC_ALLOCED_ON_STACK;
        if (vt == VT_PTR || vt == VT_UNKNOWN || vt == VT_DISPATCH)
            flags |= FC_POINTER_DEREF;

        WRITE_CHAR (str, *len, toplevel ? FC_RP : FC_UP);
        WRITE_CHAR (str, *len, flags);
        WRITE_SHORT(str, *len, ref - *len);
    }

    return off;
}

static size_t write_type_tfs(ITypeInfo *typeinfo, unsigned char *str,
    size_t *len, TYPEDESC *desc, BOOL toplevel, BOOL onstack)
{
    ITypeInfo *refinfo;
    TYPEATTR *attr;
    size_t ref, off;

    TRACE("vt %d%s\n", desc->vt, toplevel ? " (toplevel)" : "");

    switch (desc->vt)
    {
    case VT_PTR:
        desc = desc->lptdesc;

        if (desc->vt == VT_USERDEFINED)
        {
            ITypeInfo_GetRefTypeInfo(typeinfo, desc->hreftype, &refinfo);
            ITypeInfo_GetTypeAttr(refinfo, &attr);

            switch (attr->typekind)
            {
            case TKIND_ENUM:
                assert(!toplevel);  /* toplevel base-type pointers should use IsSimpleRef */
                off = *len;
                WRITE_CHAR(str, *len, FC_UP);
                WRITE_CHAR(str, *len, FC_SIMPLE_POINTER);
                WRITE_CHAR(str, *len, FC_ENUM32);
                WRITE_CHAR(str, *len, FC_PAD);
                break;
            case TKIND_INTERFACE:
            case TKIND_DISPATCH:
                off = *len;
                write_ip_tfs(str, len, &attr->guid);
                break;
            default:
                FIXME("unhandled kind %#x\n", attr->typekind);
                off = *len;
                WRITE_SHORT(str, *len, 0);
                break;
            }

            ITypeInfo_ReleaseTypeAttr(refinfo, attr);
            ITypeInfo_Release(refinfo);
            return off;
        }

        ref = write_type_tfs(typeinfo, str, len, desc, FALSE, FALSE);
        return write_pointer_tfs(str, len, desc->vt, ref, toplevel, onstack);
    case VT_CARRAY:
        return write_array_tfs(typeinfo, str, len, desc->lpadesc);
    case VT_USERDEFINED:
        ITypeInfo_GetRefTypeInfo(typeinfo, desc->hreftype, &refinfo);
        ITypeInfo_GetTypeAttr(refinfo, &attr);

        switch (attr->typekind)
        {
        case TKIND_RECORD:
            off = write_struct_tfs(refinfo, str, len, attr);
            break;
        default:
            FIXME("unhandled kind %u\n", attr->typekind);
            off = *len;
            WRITE_SHORT(str, *len, 0);
            break;
        }

        ITypeInfo_ReleaseTypeAttr(refinfo, attr);
        ITypeInfo_Release(refinfo);
        break;
    default:
        /* base types are always embedded directly */
        assert(!get_base_type(desc->vt));
        FIXME("unhandled type %u\n", desc->vt);
        off = *len;
        WRITE_SHORT(str, *len, 0);
        break;
    }

    return off;
}

static unsigned short get_stack_size(ITypeInfo *typeinfo, TYPEDESC *desc)
{
#ifdef __i386__
    if (desc->vt == VT_CARRAY)
        return sizeof(void *);
    return (type_memsize(typeinfo, desc) + 3) & ~3;
#elif defined(__x86_64__)
    return sizeof(void *);
#else
#warn get_stack_size() not implemented for this architecture
#endif
}

static const unsigned short MustSize    = 0x0001;
static const unsigned short MustFree    = 0x0002;
static const unsigned short IsIn        = 0x0008;
static const unsigned short IsOut       = 0x0010;
static const unsigned short IsReturn    = 0x0020;
static const unsigned short IsBasetype  = 0x0040;
static const unsigned short IsByValue   = 0x0080;
static const unsigned short IsSimpleRef = 0x0100;

static HRESULT write_param_fs(ITypeInfo *typeinfo, unsigned char *type,
    size_t *typelen, unsigned char *proc, size_t *proclen, ELEMDESC *desc,
    BOOL is_return, unsigned short *stack_offset)
{
    USHORT param_flags = desc->paramdesc.wParamFlags;
    int is_in  = param_flags & PARAMFLAG_FIN;
    int is_out = param_flags & PARAMFLAG_FOUT;
    TYPEDESC *tdesc = &desc->tdesc;
    unsigned short server_size = 0;
    unsigned short stack_size = get_stack_size(typeinfo, tdesc);
    unsigned short flags = MustSize;
    unsigned char basetype = 0;
    ITypeInfo *refinfo;
    TYPEATTR *attr;
    size_t off = 0;

    if (is_in)      flags |= IsIn;
    if (is_out)     flags |= IsOut;
    if (is_return)  flags |= IsOut | IsReturn;

    switch (tdesc->vt)
    {
    case VT_VARIANT:
#ifndef __i386__
        flags |= IsSimpleRef | MustFree;
        break;
#endif
        /* otherwise fall through */
    case VT_BSTR:
    case VT_SAFEARRAY:
    case VT_CY:
        flags |= IsByValue | MustFree;
        break;
    case VT_UNKNOWN:
    case VT_DISPATCH:
    case VT_CARRAY:
        flags |= MustFree;
        break;
    case VT_PTR:
        switch (tdesc->lptdesc->vt)
        {
        case VT_UNKNOWN:
        case VT_DISPATCH:
            flags |= MustFree;
            if (is_in && is_out)
                server_size = sizeof(void *);
            break;
        case VT_PTR:
            flags |= MustFree;

            if (tdesc->lptdesc->lptdesc->vt == VT_USERDEFINED)
            {
                ITypeInfo_GetRefTypeInfo(typeinfo, tdesc->lptdesc->lptdesc->hreftype, &refinfo);
                ITypeInfo_GetTypeAttr(refinfo, &attr);

                switch (attr->typekind)
                {
                case TKIND_INTERFACE:
                case TKIND_DISPATCH:
                case TKIND_COCLASS:
                    if (is_in && is_out)
                        server_size = sizeof(void *);
                    break;
                default:
                    server_size = sizeof(void *);
                }

                ITypeInfo_ReleaseTypeAttr(refinfo, attr);
                ITypeInfo_Release(refinfo);
            }
            else
                server_size = sizeof(void *);
            break;
        case VT_CARRAY:
            flags |= MustFree;
            server_size = sizeof(void *);
            break;
        case VT_USERDEFINED:
            ITypeInfo_GetRefTypeInfo(typeinfo, tdesc->lptdesc->hreftype, &refinfo);
            ITypeInfo_GetTypeAttr(refinfo, &attr);

            switch (attr->typekind)
            {
            case TKIND_ENUM:
                flags |= IsSimpleRef | IsBasetype;
                if (!is_in && is_out)
                    server_size = sizeof(void *);
                basetype = FC_ENUM32;
                break;
            case TKIND_RECORD:
                flags |= IsSimpleRef | MustFree;
                if (!is_in && is_out)
                    server_size = attr->cbSizeInstance;
                tdesc = tdesc->lptdesc;
                break;
            case TKIND_INTERFACE:
            case TKIND_DISPATCH:
            case TKIND_COCLASS:
                flags |= MustFree;
                break;
            default:
                FIXME("unhandled kind %#x\n", attr->typekind);
                return E_NOTIMPL;
            }

            ITypeInfo_ReleaseTypeAttr(refinfo, attr);
            ITypeInfo_Release(refinfo);
            break;
        default:
            flags |= IsSimpleRef;
            tdesc = tdesc->lptdesc;
            if (!is_in && is_out)
                server_size = type_memsize(typeinfo, tdesc);
            if ((basetype = get_base_type(tdesc->vt)))
                flags |= IsBasetype;
            break;
        }
        break;
    case VT_USERDEFINED:
        ITypeInfo_GetRefTypeInfo(typeinfo, tdesc->hreftype, &refinfo);
        ITypeInfo_GetTypeAttr(refinfo, &attr);

        switch (attr->typekind)
        {
        case TKIND_ENUM:
            flags |= IsBasetype;
            basetype = FC_ENUM32;
            break;
        case TKIND_RECORD:
#ifdef __i386__
            flags |= IsByValue | MustFree;
#elif defined(__x86_64__)
            if (attr->cbSizeInstance <= 8)
                flags |= IsByValue | MustFree;
            else
                flags |= IsSimpleRef | MustFree;
#endif
            break;
        default:
            FIXME("unhandled kind %#x\n", attr->typekind);
            return E_NOTIMPL;
        }

        ITypeInfo_ReleaseTypeAttr(refinfo, attr);
        ITypeInfo_Release(refinfo);
        break;
    default:
        if ((basetype = get_base_type(tdesc->vt)))
            flags |= IsBasetype;
        else
        {
            FIXME("unhandled type %u\n", tdesc->vt);
            return E_NOTIMPL;
        }
        break;
    }

    server_size = (server_size + 7) / 8;
    if (server_size < 8) flags |= server_size << 13;

    if (!basetype)
        off = write_type_tfs(typeinfo, type, typelen, tdesc, TRUE, !is_in && is_out);

    WRITE_SHORT(proc, *proclen, flags);
    WRITE_SHORT(proc, *proclen, *stack_offset);
    WRITE_SHORT(proc, *proclen, basetype ? basetype : off);

    *stack_offset += stack_size;

    return S_OK;
}

static void write_proc_func_header(ITypeInfo *typeinfo, FUNCDESC *desc,
    WORD proc_idx, unsigned char *proc, size_t *proclen)
{
    unsigned short stack_size = 2 * sizeof(void *); /* This + return */
    WORD param_idx;

    WRITE_CHAR (proc, *proclen, FC_AUTO_HANDLE);
    WRITE_CHAR (proc, *proclen, Oi_OBJECT_PROC | Oi_OBJ_USE_V2_INTERPRETER);
    WRITE_SHORT(proc, *proclen, proc_idx);
    for (param_idx = 0; param_idx < desc->cParams; param_idx++)
        stack_size += get_stack_size(typeinfo, &desc->lprgelemdescParam[param_idx].tdesc);
    WRITE_SHORT(proc, *proclen, stack_size);

    WRITE_SHORT(proc, *proclen, 0); /* constant_client_buffer_size */
    WRITE_SHORT(proc, *proclen, 0); /* constant_server_buffer_size */
    WRITE_CHAR (proc, *proclen, 0x07);  /* HasReturn | ClientMustSize | ServerMustSize */
    WRITE_CHAR (proc, *proclen, desc->cParams + 1); /* incl. return value */
}

static HRESULT write_iface_fs(ITypeInfo *typeinfo, WORD funcs,
    unsigned char *type, size_t *typelen, unsigned char *proc, size_t *proclen,
    unsigned short *offset)
{
    unsigned short stack_offset;
    WORD proc_idx, param_idx;
    FUNCDESC *desc;
    HRESULT hr;

    for (proc_idx = 0; proc_idx < funcs; proc_idx++)
    {
        TRACE("proc %d\n", proc_idx);

        hr = ITypeInfo_GetFuncDesc(typeinfo, proc_idx, &desc);
        if (FAILED(hr)) return hr;

        if (offset)
            offset[proc_idx] = *proclen;

        write_proc_func_header(typeinfo, desc, proc_idx + 3, proc, proclen);

        stack_offset = sizeof(void *);  /* This */
        for (param_idx = 0; param_idx < desc->cParams; param_idx++)
        {
            TRACE("param %d\n", param_idx);
            hr = write_param_fs(typeinfo, type, typelen, proc, proclen,
                &desc->lprgelemdescParam[param_idx], FALSE, &stack_offset);
            if (FAILED(hr)) return hr;
        }

        hr = write_param_fs(typeinfo, type, typelen, proc, proclen,
            &desc->elemdescFunc, TRUE, &stack_offset);
        if (FAILED(hr)) return hr;

        ITypeInfo_ReleaseFuncDesc(typeinfo, desc);
    }

    return S_OK;
}

static HRESULT build_format_strings(ITypeInfo *typeinfo, WORD funcs,
    const unsigned char **type_ret,
    const unsigned char **proc_ret, unsigned short **offset_ret)
{
    size_t typelen = 0, proclen = 0;
    unsigned char *type, *proc;
    unsigned short *offset;
    HRESULT hr;

    hr = write_iface_fs(typeinfo, funcs, NULL, &typelen, NULL, &proclen, NULL);
    if (FAILED(hr)) return hr;

    type = heap_alloc(typelen);
    proc = heap_alloc(proclen);
    offset = heap_alloc(funcs * sizeof(*offset));
    if (!type || !proc || !offset)
    {
        hr = E_OUTOFMEMORY;
        goto err;
    }

    typelen = 0;
    proclen = 0;

    hr = write_iface_fs(typeinfo, funcs, type, &typelen, proc, &proclen, offset);
    if (SUCCEEDED(hr))
    {
        *type_ret = type;
        *proc_ret = proc;
        *offset_ret = offset;
        return S_OK;
    }

err:
    heap_free(type);
    heap_free(proc);
    heap_free(offset);
    return hr;
}

static void init_stub_desc(MIDL_STUB_DESC *desc)
{
    desc->pfnAllocate = NdrOleAllocate;
    desc->pfnFree = NdrOleFree;
    desc->Version = 0x50002;
    /* type format string is initialized with proc format string and offset table */
}

struct typelib_proxy
{
    StdProxyImpl proxy;
    IID iid;
    MIDL_STUB_DESC stub_desc;
    MIDL_STUBLESS_PROXY_INFO proxy_info;
    CInterfaceProxyVtbl *proxy_vtbl;
    unsigned short *offset_table;
};

static ULONG WINAPI typelib_proxy_Release(IRpcProxyBuffer *iface)
{
    struct typelib_proxy *This = CONTAINING_RECORD(iface, struct typelib_proxy, proxy.IRpcProxyBuffer_iface);
    ULONG refcount = InterlockedDecrement(&This->proxy.RefCount);

    TRACE("(%p) decreasing refs to %d\n", This, refcount);

    if (!refcount)
    {
        if (This->proxy.pChannel)
            IRpcProxyBuffer_Disconnect(&This->proxy.IRpcProxyBuffer_iface);
        heap_free((void *)This->stub_desc.pFormatTypes);
        heap_free((void *)This->proxy_info.ProcFormatString);
        heap_free(This->offset_table);
        heap_free(This->proxy_vtbl);
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
    ULONG count, IRpcProxyBuffer **proxy, void **obj)
{
    if (!fill_stubless_table((IUnknownVtbl *)This->proxy_vtbl->Vtbl, count))
        return E_OUTOFMEMORY;

    if (!outer) outer = (IUnknown *)&This->proxy;

    This->proxy.IRpcProxyBuffer_iface.lpVtbl = &typelib_proxy_vtbl;
    This->proxy.PVtbl = This->proxy_vtbl->Vtbl;
    This->proxy.RefCount = 1;
    This->proxy.piid = This->proxy_vtbl->header.piid;
    This->proxy.pUnkOuter = outer;

    *proxy = &This->proxy.IRpcProxyBuffer_iface;
    *obj = &This->proxy.PVtbl;
    IUnknown_AddRef((IUnknown *)*obj);

    return S_OK;
}

HRESULT WINAPI CreateProxyFromTypeInfo(ITypeInfo *typeinfo, IUnknown *outer,
    REFIID iid, IRpcProxyBuffer **proxy, void **obj)
{
    struct typelib_proxy *This;
    WORD funcs, vtbl_size, i;
    TYPEATTR *typeattr;
    HRESULT hr;

    TRACE("typeinfo %p, outer %p, iid %s, proxy %p, obj %p.\n",
        typeinfo, outer, debugstr_guid(iid), proxy, obj);

    hr = ITypeInfo_GetTypeAttr(typeinfo, &typeattr);
    if (FAILED(hr))
        return hr;
    funcs = typeattr->cFuncs;
    vtbl_size = typeattr->cbSizeVft;
    ITypeInfo_ReleaseTypeAttr(typeinfo, typeattr);

    if (!(This = heap_alloc_zero(sizeof(*This))))
        return E_OUTOFMEMORY;

    init_stub_desc(&This->stub_desc);
    This->proxy_info.pStubDesc = &This->stub_desc;

    This->proxy_vtbl = heap_alloc_zero(sizeof(This->proxy_vtbl->header) + vtbl_size);
    if (!This->proxy_vtbl)
    {
        heap_free(This);
        return E_OUTOFMEMORY;
    }
    This->proxy_vtbl->header.pStublessProxyInfo = &This->proxy_info;
    This->iid = *iid;
    This->proxy_vtbl->header.piid = &This->iid;
    for (i = 0; i < funcs; i++)
        This->proxy_vtbl->Vtbl[3 + i] = (void *)-1;

    hr = build_format_strings(typeinfo, funcs, &This->stub_desc.pFormatTypes,
        &This->proxy_info.ProcFormatString, &This->offset_table);
    if (FAILED(hr))
    {
        heap_free(This->proxy_vtbl);
        heap_free(This);
        return hr;
    }
    This->proxy_info.FormatStringOffset = &This->offset_table[-3];

    hr = typelib_proxy_init(This, outer, vtbl_size / sizeof(void *), proxy, obj);
    if (FAILED(hr))
    {
        heap_free((void *)This->stub_desc.pFormatTypes);
        heap_free((void *)This->proxy_info.ProcFormatString);
        heap_free((void *)This->offset_table);
        heap_free(This->proxy_vtbl);
        heap_free(This);
    }

    return hr;
}

struct typelib_stub
{
    CStdStubBuffer stub;
    IID iid;
    MIDL_STUB_DESC stub_desc;
    MIDL_SERVER_INFO server_info;
    CInterfaceStubVtbl stub_vtbl;
    unsigned short *offset_table;
};

static ULONG WINAPI typelib_stub_Release(IRpcStubBuffer *iface)
{
    struct typelib_stub *This = CONTAINING_RECORD(iface, struct typelib_stub, stub);
    ULONG refcount = InterlockedDecrement(&This->stub.RefCount);

    TRACE("(%p) decreasing refs to %d\n", This, refcount);

    if (!refcount)
    {
        /* test_Release shows that native doesn't call Disconnect here.
           We'll leave it in for the time being. */
        IRpcStubBuffer_Disconnect(iface);

        heap_free((void *)This->stub_desc.pFormatTypes);
        heap_free((void *)This->server_info.ProcString);
        heap_free(This->offset_table);
        heap_free(This);
    }

    return refcount;
}

static HRESULT typelib_stub_init(struct typelib_stub *This,
    IUnknown *server, IRpcStubBuffer **stub)
{
    HRESULT hr;

    hr = IUnknown_QueryInterface(server, This->stub_vtbl.header.piid,
        (void **)&This->stub.pvServerObject);
    if (FAILED(hr))
    {
        WARN("Failed to get interface %s, hr %#x.\n",
            debugstr_guid(This->stub_vtbl.header.piid), hr);
        This->stub.pvServerObject = server;
        IUnknown_AddRef(server);
    }

    This->stub.lpVtbl = &This->stub_vtbl.Vtbl;
    This->stub.RefCount = 1;

    *stub = (IRpcStubBuffer *)&This->stub;
    return S_OK;
}

HRESULT WINAPI CreateStubFromTypeInfo(ITypeInfo *typeinfo, REFIID iid,
    IUnknown *server, IRpcStubBuffer **stub)
{
    struct typelib_stub *This;
    WORD funcs, vtbl_size;
    TYPEATTR *typeattr;
    HRESULT hr;

    TRACE("typeinfo %p, iid %s, server %p, stub %p.\n",
        typeinfo, debugstr_guid(iid), server, stub);

    hr = ITypeInfo_GetTypeAttr(typeinfo, &typeattr);
    if (FAILED(hr))
        return hr;
    funcs = typeattr->cFuncs;
    vtbl_size = typeattr->cbSizeVft;
    ITypeInfo_ReleaseTypeAttr(typeinfo, typeattr);

    if (!(This = heap_alloc_zero(sizeof(*This))))
        return E_OUTOFMEMORY;

    init_stub_desc(&This->stub_desc);
    This->server_info.pStubDesc = &This->stub_desc;

    hr = build_format_strings(typeinfo, funcs, &This->stub_desc.pFormatTypes,
        &This->server_info.ProcString, &This->offset_table);
    if (FAILED(hr))
    {
        heap_free(This);
        return hr;
    }
    This->server_info.FmtStringOffset = &This->offset_table[-3];

    This->iid = *iid;
    This->stub_vtbl.header.piid = &This->iid;
    This->stub_vtbl.header.pServerInfo = &This->server_info;
    This->stub_vtbl.header.DispatchTableCount = vtbl_size / sizeof(void *);
    This->stub_vtbl.Vtbl = CStdStubBuffer_Vtbl;
    This->stub_vtbl.Vtbl.Release = typelib_stub_Release;

    hr = typelib_stub_init(This, server, stub);
    if (FAILED(hr))
    {
        heap_free((void *)This->stub_desc.pFormatTypes);
        heap_free((void *)This->server_info.ProcString);
        heap_free(This->offset_table);
        heap_free(This);
    }

    return hr;
}
