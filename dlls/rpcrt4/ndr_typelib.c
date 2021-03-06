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

static const USER_MARSHAL_ROUTINE_QUADRUPLE oleaut_usermarshal[] =
{
    {
        (USER_MARSHAL_SIZING_ROUTINE)BSTR_UserSize,
        (USER_MARSHAL_MARSHALLING_ROUTINE)BSTR_UserMarshal,
        (USER_MARSHAL_UNMARSHALLING_ROUTINE)BSTR_UserUnmarshal,
        (USER_MARSHAL_FREEING_ROUTINE)BSTR_UserFree
    },
    {
        (USER_MARSHAL_SIZING_ROUTINE)VARIANT_UserSize,
        (USER_MARSHAL_MARSHALLING_ROUTINE)VARIANT_UserMarshal,
        (USER_MARSHAL_UNMARSHALLING_ROUTINE)VARIANT_UserUnmarshal,
        (USER_MARSHAL_FREEING_ROUTINE)VARIANT_UserFree
    },
    {
        (USER_MARSHAL_SIZING_ROUTINE)LPSAFEARRAY_UserSize,
        (USER_MARSHAL_MARSHALLING_ROUTINE)LPSAFEARRAY_UserMarshal,
        (USER_MARSHAL_UNMARSHALLING_ROUTINE)LPSAFEARRAY_UserUnmarshal,
        (USER_MARSHAL_FREEING_ROUTINE)LPSAFEARRAY_UserFree
    }
};

#ifndef _WIN64

static const unsigned char oleaut_tfs[] =
{
        NdrFcShort(0x0),
/* 2 (unsigned short[]) */
        0x1b,	/* FC_CARRAY */
        0x1,	/* 1 */
        NdrFcShort(0x2),	/* 2 */
        0x9,	/* Corr desc: field clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0xfffc),	/* offset = -4 */
        0x06,	/* FC_SHORT */
        0x5b,	/* FC_END */
/* 12 (FLAGGED_WORD_BLOB) */
        0x17,	/* FC_CSTRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0xfff2),	/* Offset= -14 (2) */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 22 (wireBSTR) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfff4),	/* Offset= -12 (12) */
/* 26 (BSTR) */
        0xb4,	/* FC_USER_MARSHAL */
        0x83,	/* Alignment= 3, Flags= 80 */
        NdrFcShort(0x0),	/* Function offset= 0 */
        NdrFcShort(0x4),	/* 4 */
        NdrFcShort(0x0),	/* 0 */
        NdrFcShort(0xfff4),	/* Offset= -12 (22) */
/* 36 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 54 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 72 (wireBSTR) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffc2),	/* Offset= -62 (12) */
/* 76 (CY) */
        0x15,	/* FC_STRUCT */
        0x7,	/* 7 */
        NdrFcShort(0x8),	/* 8 */
        0x0b,	/* FC_HYPER */
        0x5b,	/* FC_END */
/* 82 (DECIMAL) */
        0x15,	/* FC_STRUCT */
        0x7,	/* 7 */
        NdrFcShort(0x10),	/* 16 */
        0x06,	/* FC_SHORT */
        0x02,	/* FC_CHAR */
        0x02,	/* FC_CHAR */
        0x08,	/* FC_LONG */
        0x0b,	/* FC_HYPER */
        0x5b,	/* FC_END */
/* 92 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 110 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 128 (wireBSTR *) */
        0x1b,	/* FC_CARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x4),	/* 4 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x4b,	/* FC_PP */
        0x5c,	/* FC_PAD */
        0x48, /* FC_VARIABLE_REPEAT */
        0x49, /* FC_FIXED_OFFSET */
        NdrFcShort(0x4),	/* Increment = 4 */
        NdrFcShort(0x0),	/* Offset to array = 0 */
        NdrFcShort(0x1),	/* Number of pointers = 1 */
        NdrFcShort(0x0),	/* Memory offset = 0 */
        NdrFcShort(0x0),	/* Buffer offset = 0 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xff74),	/* Offset= -140 (12) */
        0x5b,	/* FC_END */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xff6f),	/* Offset= -145 (12) */
        0x5b,	/* FC_END */
/* 160 (SAFEARR_BSTR) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (172) */
        0x08,	/* FC_LONG */
        0x36,	/* FC_POINTER */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 172 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffd2),	/* Offset= -46 (128) */
/* 176 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 194 (IUnknown **) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe0),	/* Offset= -32 (176) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 212 (SAFEARR_UNKNOWN) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (224) */
        0x08,	/* FC_LONG */
        0x36,	/* FC_POINTER */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 224 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffe0),	/* Offset= -32 (194) */
/* 228 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 246 (IDispatch **) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe0),	/* Offset= -32 (228) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 264 (SAFEARR_DISPATCH) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (276) */
        0x08,	/* FC_LONG */
        0x36,	/* FC_POINTER */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 276 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffe0),	/* Offset= -32 (246) */
/* 280 (wireVARIANT *) */
        0x1b,	/* FC_CARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x4),	/* 4 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x4b,	/* FC_PP */
        0x5c,	/* FC_PAD */
        0x48, /* FC_VARIABLE_REPEAT */
        0x49, /* FC_FIXED_OFFSET */
        NdrFcShort(0x4),	/* Increment = 4 */
        NdrFcShort(0x0),	/* Offset to array = 0 */
        NdrFcShort(0x1),	/* Number of pointers = 1 */
        NdrFcShort(0x0),	/* Memory offset = 0 */
        NdrFcShort(0x0),	/* Buffer offset = 0 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0x33e),	/* Offset= 830 (1134) */
        0x5b,	/* FC_END */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0x339),	/* Offset= 825 (1134) */
        0x5b,	/* FC_END */
/* 312 (SAFEARR_VARIANT) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (324) */
        0x08,	/* FC_LONG */
        0x36,	/* FC_POINTER */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 324 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffd2),	/* Offset= -46 (280) */
/* 328 (IRecordInfo *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x0000002f),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 346 (byte *) */
        0x1b,	/* FC_CARRAY */
        0x0,	/* 0 */
        NdrFcShort(0x1),	/* 1 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x4),	/* offset = 4 */
        0x01,	/* FC_BYTE */
        0x5b,	/* FC_END */
/* 356 (struct _wireBRECORD) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0xa),	/* Offset= 10 (372) */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffd8),	/* Offset= -40 (328) */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 372 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe4),	/* Offset= -28 (346) */
/* 376 (wireBRECORD *) */
        0x1b,	/* FC_CARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x4),	/* 4 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x4b,	/* FC_PP */
        0x5c,	/* FC_PAD */
        0x48, /* FC_VARIABLE_REPEAT */
        0x49, /* FC_FIXED_OFFSET */
        NdrFcShort(0x4),	/* Increment = 4 */
        NdrFcShort(0x0),	/* Offset to array = 0 */
        NdrFcShort(0x1),	/* Number of pointers = 1 */
        NdrFcShort(0x0),	/* Memory offset = 0 */
        NdrFcShort(0x0),	/* Buffer offset = 0 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffd4),	/* Offset= -44 (356) */
        0x5b,	/* FC_END */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffcf),	/* Offset= -49 (356) */
        0x5b,	/* FC_END */
/* 408 (SAFEARR_BRECORD) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (420) */
        0x08,	/* FC_LONG */
        0x36,	/* FC_POINTER */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 420 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffd2),	/* Offset= -46 (376) */
/* 424 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 442 (IUnknown **) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe0),	/* Offset= -32 (424) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 460 (byte[8]) */
        0x1d,	/* FC_SMFARRAY */
        0x0,	/* 0 */
        NdrFcShort(0x8),	/* 8 */
        0x01,	/* FC_BYTE */
        0x5b,	/* FC_END */
/* 466 (IID) */
        0x15,	/* FC_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        0x08,	/* FC_LONG */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xfff1),	/* Offset= -15 (460) */
        0x5b,	/* FC_END */
/* 478 (SAFEARR_HAVEIID) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x18),	/* 24 */
        NdrFcShort(0x0),
        NdrFcShort(0xa),	/* Offset= 10 (494) */
        0x08,	/* FC_LONG */
        0x36,	/* FC_POINTER */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe8),	/* Offset= -24 (466) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 494 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffca),	/* Offset= -54 (442) */
/* 498 (byte *) */
        0x1b,	/* FC_CARRAY */
        0x0,	/* 0 */
        NdrFcShort(0x1),	/* 1 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x01,	/* FC_BYTE */
        0x5b,	/* FC_END */
/* 508 (BYTE_SIZEDARR) */
        0x16,	/* FC_PSTRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x4b,	/* FC_PP */
        0x5c,	/* FC_PAD */
        0x46, /* FC_NO_REPEAT */
        0x5c, /* FC_PAD */
        NdrFcShort(0x4),	/* Memory offset = 4 */
        NdrFcShort(0x4),	/* Buffer offset = 4 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (498) */
        0x5b,	/* FC_END */
        0x08,	/* FC_LONG */
        0x8,	/* FC_LONG */
        0x5b,	/* FC_END */
/* 528 (unsigned short *) */
        0x1b,	/* FC_CARRAY */
        0x1,	/* 1 */
        NdrFcShort(0x2),	/* 2 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x06,	/* FC_SHORT */
        0x5b,	/* FC_END */
/* 538 (WORD_SIZEDARR) */
        0x16,	/* FC_PSTRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x4b,	/* FC_PP */
        0x5c,	/* FC_PAD */
        0x46, /* FC_NO_REPEAT */
        0x5c, /* FC_PAD */
        NdrFcShort(0x4),	/* Memory offset = 4 */
        NdrFcShort(0x4),	/* Buffer offset = 4 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (528) */
        0x5b,	/* FC_END */
        0x08,	/* FC_LONG */
        0x8,	/* FC_LONG */
        0x5b,	/* FC_END */
/* 558 (ULONG *) */
        0x1b,	/* FC_CARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x4),	/* 4 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x08,	/* FC_LONG */
        0x5b,	/* FC_END */
/* 568 (DWORD_SIZEDARR) */
        0x16,	/* FC_PSTRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x4b,	/* FC_PP */
        0x5c,	/* FC_PAD */
        0x46, /* FC_NO_REPEAT */
        0x5c, /* FC_PAD */
        NdrFcShort(0x4),	/* Memory offset = 4 */
        NdrFcShort(0x4),	/* Buffer offset = 4 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (558) */
        0x5b,	/* FC_END */
        0x08,	/* FC_LONG */
        0x8,	/* FC_LONG */
        0x5b,	/* FC_END */
/* 588 (hyper *) */
        0x1b,	/* FC_CARRAY */
        0x7,	/* 7 */
        NdrFcShort(0x8),	/* 8 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x0b,	/* FC_HYPER */
        0x5b,	/* FC_END */
/* 598 (HYPER_SIZEDARR) */
        0x16,	/* FC_PSTRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x4b,	/* FC_PP */
        0x5c,	/* FC_PAD */
        0x46, /* FC_NO_REPEAT */
        0x5c, /* FC_PAD */
        NdrFcShort(0x4),	/* Memory offset = 4 */
        NdrFcShort(0x4),	/* Buffer offset = 4 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (588) */
        0x5b,	/* FC_END */
        0x08,	/* FC_LONG */
        0x8,	/* FC_LONG */
        0x5b,	/* FC_END */
/* 618 (SAFEARRAYUNION) */
        0x2a,	/* FC_ENCAPSULATED_UNION */
        0x49,	/* Switch type= FC_ULONG */
        NdrFcShort(0x18),	/* 24 */
        NdrFcShort(0xa),	/* 10 */
        NdrFcLong(0x8),	/* 8 */
        NdrFcShort(0xfe2c),	/* Offset= -468 (160) */
        NdrFcLong(0xd),	/* 13 */
        NdrFcShort(0xfe5a),	/* Offset= -422 (212) */
        NdrFcLong(0x9),	/* 9 */
        NdrFcShort(0xfe88),	/* Offset= -376 (264) */
        NdrFcLong(0xc),	/* 12 */
        NdrFcShort(0xfeb2),	/* Offset= -334 (312) */
        NdrFcLong(0x24),	/* 36 */
        NdrFcShort(0xff0c),	/* Offset= -244 (408) */
        NdrFcLong(0x800d),	/* 32781 */
        NdrFcShort(0xff4c),	/* Offset= -180 (478) */
        NdrFcLong(0x10),	/* 16 */
        NdrFcShort(0xff64),	/* Offset= -156 (508) */
        NdrFcLong(0x2),	/* 2 */
        NdrFcShort(0xff7c),	/* Offset= -132 (538) */
        NdrFcLong(0x3),	/* 3 */
        NdrFcShort(0xff94),	/* Offset= -108 (568) */
        NdrFcLong(0x14),	/* 20 */
        NdrFcShort(0xffac),	/* Offset= -84 (598) */
        NdrFcShort(0xffff),
/* 686 (SAFEARRAYBOUND) */
        0x15,	/* FC_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 694 (SAFEARRAYBOUND[]) */
        0x1b,	/* FC_CARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x7,	/* Corr desc: field cDims, FC_USHORT */
        0x0,	/* no operators */
        NdrFcShort(0xffd8),	/* offset = -40 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffee),	/* Offset= -18 (686) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 708 (struct _wireSAFEARRAY) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x28),	/* 40 */
        NdrFcShort(0xffee),	/* Offset= -18 (694) */
        NdrFcShort(0x0),	/* Offset= 0 (714) */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xff98),	/* Offset= -104 (618) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 726 (wireSAFEARRAY) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffec),	/* Offset= -20 (708) */
/* 730 (signed char *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x02,	/* FC_CHAR */
        0x5c,	/* FC_PAD */
/* 734 (USHORT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x07,	/* FC_USHORT */
        0x5c,	/* FC_PAD */
/* 738 (ULONG *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x09,	/* FC_ULONG */
        0x5c,	/* FC_PAD */
/* 742 (INT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
/* 746 (UINT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x09,	/* FC_ULONG */
        0x5c,	/* FC_PAD */
/* 750 (BYTE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x02,	/* FC_CHAR */
        0x5c,	/* FC_PAD */
/* 754 (SHORT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x06,	/* FC_SHORT */
        0x5c,	/* FC_PAD */
/* 758 (LONG *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
/* 762 (FLOAT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x0a,	/* FC_FLOAT */
        0x5c,	/* FC_PAD */
/* 766 (DOUBLE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x0c,	/* FC_DOUBLE */
        0x5c,	/* FC_PAD */
/* 770 (VARIANT_BOOL *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x06,	/* FC_SHORT */
        0x5c,	/* FC_PAD */
/* 774 (SCODE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
/* 778 (DATE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x0c,	/* FC_DOUBLE */
        0x5c,	/* FC_PAD */
/* 782 (wireBSTR) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfcfc),	/* Offset= -772 (12) */
/* 786 (wireBSTR *) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (782) */
/* 790 (wireVARIANT) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0x156),	/* Offset= 342 (1134) */
/* 794 (wireVARIANT *) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (790) */
/* 798 (CY *) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfd2c),	/* Offset= -724 (76) */
/* 802 (DECIMAL *) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfd2e),	/* Offset= -722 (82) */
/* 806 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 824 (IUnknown **) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xffec),	/* Offset= -20 (806) */
/* 828 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 846 (IDispatch **) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xffec),	/* Offset= -20 (828) */
/* 850 (wireSAFEARRAY) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xff70),	/* Offset= -144 (708) */
/* 854 (wireSAFEARRAY *) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (850) */
/* 858 (wireBRECORD) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfe08),	/* Offset= -504 (356) */
/* 862 (union ) */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x2b),	/* 43 */
        NdrFcLong(0x0),	/* 0 */
        NdrFcShort(0x0),	/* No type */
        NdrFcLong(0x1),	/* 1 */
        NdrFcShort(0x0),	/* No type */
        NdrFcLong(0x10),	/* 16 */
        NdrFcShort(0x8002),	/* Simple arm type: FC_CHAR */
        NdrFcLong(0x12),	/* 18 */
        NdrFcShort(0x8007),	/* Simple arm type: FC_USHORT */
        NdrFcLong(0x13),	/* 19 */
        NdrFcShort(0x8009),	/* Simple arm type: FC_ULONG */
        NdrFcLong(0x16),	/* 22 */
        NdrFcShort(0x8008),	/* Simple arm type: FC_LONG */
        NdrFcLong(0x17),	/* 23 */
        NdrFcShort(0x8009),	/* Simple arm type: FC_ULONG */
        NdrFcLong(0x11),	/* 17 */
        NdrFcShort(0x8002),	/* Simple arm type: FC_CHAR */
        NdrFcLong(0x2),	/* 2 */
        NdrFcShort(0x8006),	/* Simple arm type: FC_SHORT */
        NdrFcLong(0x3),	/* 3 */
        NdrFcShort(0x8008),	/* Simple arm type: FC_LONG */
        NdrFcLong(0x4),	/* 4 */
        NdrFcShort(0x800a),	/* Simple arm type: FC_FLOAT */
        NdrFcLong(0x5),	/* 5 */
        NdrFcShort(0x800c),	/* Simple arm type: FC_DOUBLE */
        NdrFcLong(0xb),	/* 11 */
        NdrFcShort(0x8006),	/* Simple arm type: FC_SHORT */
        NdrFcLong(0xa),	/* 10 */
        NdrFcShort(0x8008),	/* Simple arm type: FC_LONG */
        NdrFcLong(0x7),	/* 7 */
        NdrFcShort(0x800c),	/* Simple arm type: FC_DOUBLE */
        NdrFcLong(0x8),	/* 8 */
        NdrFcShort(0xff4e),	/* Offset= -178 (782) */
        NdrFcLong(0x6),	/* 6 */
        NdrFcShort(0xfc86),	/* Offset= -890 (76) */
        NdrFcLong(0xe),	/* 14 */
        NdrFcShort(0xfc86),	/* Offset= -890 (82) */
        NdrFcLong(0xd),	/* 13 */
        NdrFcShort(0xfc8a),	/* Offset= -886 (92) */
        NdrFcLong(0x9),	/* 9 */
        NdrFcShort(0xfc96),	/* Offset= -874 (110) */
        NdrFcLong(0x2000),	/* 8192 */
        NdrFcShort(0xff74),	/* Offset= -140 (850) */
        NdrFcLong(0x4010),	/* 16400 */
        NdrFcShort(0xfef6),	/* Offset= -266 (730) */
        NdrFcLong(0x4012),	/* 16402 */
        NdrFcShort(0xfef4),	/* Offset= -268 (734) */
        NdrFcLong(0x4013),	/* 16403 */
        NdrFcShort(0xfef2),	/* Offset= -270 (738) */
        NdrFcLong(0x4016),	/* 16406 */
        NdrFcShort(0xfef0),	/* Offset= -272 (742) */
        NdrFcLong(0x4017),	/* 16407 */
        NdrFcShort(0xfeee),	/* Offset= -274 (746) */
        NdrFcLong(0x4011),	/* 16401 */
        NdrFcShort(0xfeec),	/* Offset= -276 (750) */
        NdrFcLong(0x4002),	/* 16386 */
        NdrFcShort(0xfeea),	/* Offset= -278 (754) */
        NdrFcLong(0x4003),	/* 16387 */
        NdrFcShort(0xfee8),	/* Offset= -280 (758) */
        NdrFcLong(0x4004),	/* 16388 */
        NdrFcShort(0xfee6),	/* Offset= -282 (762) */
        NdrFcLong(0x4005),	/* 16389 */
        NdrFcShort(0xfee4),	/* Offset= -284 (766) */
        NdrFcLong(0x400b),	/* 16395 */
        NdrFcShort(0xfee2),	/* Offset= -286 (770) */
        NdrFcLong(0x400a),	/* 16394 */
        NdrFcShort(0xfee0),	/* Offset= -288 (774) */
        NdrFcLong(0x4007),	/* 16391 */
        NdrFcShort(0xfede),	/* Offset= -290 (778) */
        NdrFcLong(0x4008),	/* 16392 */
        NdrFcShort(0xfee0),	/* Offset= -288 (786) */
        NdrFcLong(0x400c),	/* 16396 */
        NdrFcShort(0xfee2),	/* Offset= -286 (794) */
        NdrFcLong(0x4006),	/* 16390 */
        NdrFcShort(0xfee0),	/* Offset= -288 (798) */
        NdrFcLong(0x400e),	/* 16398 */
        NdrFcShort(0xfede),	/* Offset= -290 (802) */
        NdrFcLong(0x400d),	/* 16397 */
        NdrFcShort(0xfeee),	/* Offset= -274 (824) */
        NdrFcLong(0x4009),	/* 16393 */
        NdrFcShort(0xfefe),	/* Offset= -258 (846) */
        NdrFcLong(0x6000),	/* 24576 */
        NdrFcShort(0xff00),	/* Offset= -256 (854) */
        NdrFcLong(0x24),	/* 36 */
        NdrFcShort(0xfefe),	/* Offset= -258 (858) */
        NdrFcLong(0x4024),	/* 16420 */
        NdrFcShort(0xfef8),	/* Offset= -264 (858) */
        NdrFcShort(0xffff),
/* 1126 */
        0x2b,	/* FC_NON_ENCAPSULATED_UNION */
        0x8,	/* FIXME: always FC_LONG */
        0x7,	/* Corr desc: field vt, FC_USHORT */
        0x0,	/* no operators */
        NdrFcShort(0xfff8),	/* offset = -8 */
        NdrFcShort(0xfef2),	/* Offset= -270 (862) */
/* 1134 (struct _wireVARIANT) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x7,	/* 7 */
        NdrFcShort(0x20),	/* 32 */
        NdrFcShort(0x0),
        NdrFcShort(0x0),	/* Offset= 0 (1140) */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe8),	/* Offset= -24 (1126) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 1154 (wireVARIANT) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffea),	/* Offset= -22 (1134) */
/* 1158 (VARIANT) */
        0xb4,	/* FC_USER_MARSHAL */
        0x83,	/* Alignment= 3, Flags= 80 */
        NdrFcShort(0x1),	/* Function offset= 1 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),	/* 0 */
        NdrFcShort(0xfff4),	/* Offset= -12 (1154) */
/* 1168 (wireSAFEARRAY) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfe32),	/* Offset= -462 (708) */
/* 1172 (wirePSAFEARRAY) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (1168) */
/* 1176 (LPSAFEARRAY) */
        0xb4,	/* FC_USER_MARSHAL */
        0x83,	/* Alignment= 3, Flags= 80 */
        NdrFcShort(0x2),	/* Function offset= 2 */
        NdrFcShort(0x4),	/* 4 */
        NdrFcShort(0x0),	/* 0 */
        NdrFcShort(0xfff4),	/* Offset= -12 (1172) */
        0x0
};

static const unsigned short oleaut_offsets[] =
{
    26,     /* BSTR */
    36,     /* IUnknown* */
    54,     /* IDispatch* */
    1158,   /* VARIANT */
    1176,   /* LPSAFEARRAY */
};
#else
static const unsigned char oleaut_tfs[] =
{
        NdrFcShort(0x0),
/* 2 (unsigned short[]) */
        0x1b,	/* FC_CARRAY */
        0x1,	/* 1 */
        NdrFcShort(0x2),	/* 2 */
        0x9,	/* Corr desc: field clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0xfffc),	/* offset = -4 */
        0x06,	/* FC_SHORT */
        0x5b,	/* FC_END */
/* 12 (FLAGGED_WORD_BLOB) */
        0x17,	/* FC_CSTRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0xfff2),	/* Offset= -14 (2) */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 22 (wireBSTR) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfff4),	/* Offset= -12 (12) */
/* 26 (BSTR) */
        0xb4,	/* FC_USER_MARSHAL */
        0x83,	/* Alignment= 3, Flags= 80 */
        NdrFcShort(0x0),	/* Function offset= 0 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0x0),	/* 0 */
        NdrFcShort(0xfff4),	/* Offset= -12 (22) */
/* 36 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 54 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 72 (wireBSTR) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffc2),	/* Offset= -62 (12) */
/* 76 (CY) */
        0x15,	/* FC_STRUCT */
        0x7,	/* 7 */
        NdrFcShort(0x8),	/* 8 */
        0x0b,	/* FC_HYPER */
        0x5b,	/* FC_END */
/* 82 (DECIMAL) */
        0x15,	/* FC_STRUCT */
        0x7,	/* 7 */
        NdrFcShort(0x10),	/* 16 */
        0x06,	/* FC_SHORT */
        0x02,	/* FC_CHAR */
        0x02,	/* FC_CHAR */
        0x08,	/* FC_LONG */
        0x0b,	/* FC_HYPER */
        0x5b,	/* FC_END */
/* 92 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 110 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 128 (wireBSTR *) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xff7e),	/* Offset= -130 (12) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 146 (SAFEARR_BSTR) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (158) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 158 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffe0),	/* Offset= -32 (128) */
/* 162 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 180 (IUnknown **) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe0),	/* Offset= -32 (162) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 198 (SAFEARR_UNKNOWN) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (210) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 210 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffe0),	/* Offset= -32 (180) */
/* 214 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 232 (IDispatch **) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe0),	/* Offset= -32 (214) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 250 (SAFEARR_DISPATCH) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (262) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 262 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffe0),	/* Offset= -32 (232) */
/* 266 (wireVARIANT *) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0x31c),	/* Offset= 796 (1076) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 284 (SAFEARR_VARIANT) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (296) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 296 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffe0),	/* Offset= -32 (266) */
/* 300 (IRecordInfo *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x0000002f),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 318 (byte *) */
        0x1b,	/* FC_CARRAY */
        0x0,	/* 0 */
        NdrFcShort(0x1),	/* 1 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x4),	/* offset = 4 */
        0x01,	/* FC_BYTE */
        0x5b,	/* FC_END */
/* 328 (struct _wireBRECORD) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x18),	/* 24 */
        NdrFcShort(0x0),
        NdrFcShort(0xa),	/* Offset= 10 (344) */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffd8),	/* Offset= -40 (300) */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 344 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe4),	/* Offset= -28 (318) */
/* 348 (wireBRECORD *) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffde),	/* Offset= -34 (328) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 366 (SAFEARR_BRECORD) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (378) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 378 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffe0),	/* Offset= -32 (348) */
/* 382 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 400 (IUnknown **) */
        0x21,	/* FC_BOGUS_ARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x0),	/* 0 */
        0x19,	/* Corr desc: field pointer Size, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        NdrFcLong(0xffffffff),	/* -1 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe0),	/* Offset= -32 (382) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 418 (byte[8]) */
        0x1d,	/* FC_SMFARRAY */
        0x0,	/* 0 */
        NdrFcShort(0x8),	/* 8 */
        0x01,	/* FC_BYTE */
        0x5b,	/* FC_END */
/* 424 (IID) */
        0x15,	/* FC_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        0x08,	/* FC_LONG */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xfff1),	/* Offset= -15 (418) */
        0x5b,	/* FC_END */
/* 436 (SAFEARR_HAVEIID) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x20),	/* 32 */
        NdrFcShort(0x0),
        NdrFcShort(0xa),	/* Offset= 10 (452) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe7),	/* Offset= -25 (424) */
        0x5b,	/* FC_END */
/* 452 */
        0x11, 0x0,		/* FC_RP */
        NdrFcShort(0xffca),	/* Offset= -54 (400) */
/* 456 (byte *) */
        0x1b,	/* FC_CARRAY */
        0x0,	/* 0 */
        NdrFcShort(0x1),	/* 1 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x01,	/* FC_BYTE */
        0x5b,	/* FC_END */
/* 466 (BYTE_SIZEDARR) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (478) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 478 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (456) */
/* 482 (unsigned short *) */
        0x1b,	/* FC_CARRAY */
        0x1,	/* 1 */
        NdrFcShort(0x2),	/* 2 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x06,	/* FC_SHORT */
        0x5b,	/* FC_END */
/* 492 (WORD_SIZEDARR) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (504) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 504 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (482) */
/* 508 (ULONG *) */
        0x1b,	/* FC_CARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x4),	/* 4 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x08,	/* FC_LONG */
        0x5b,	/* FC_END */
/* 518 (DWORD_SIZEDARR) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (530) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 530 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (508) */
/* 534 (hyper *) */
        0x1b,	/* FC_CARRAY */
        0x7,	/* 7 */
        NdrFcShort(0x8),	/* 8 */
        0x19,	/* Corr desc: field pointer clSize, FC_ULONG */
        0x0,	/* no operators */
        NdrFcShort(0x0),	/* offset = 0 */
        0x0b,	/* FC_HYPER */
        0x5b,	/* FC_END */
/* 544 (HYPER_SIZEDARR) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x0),
        NdrFcShort(0x6),	/* Offset= 6 (556) */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x36,	/* FC_POINTER */
        0x5b,	/* FC_END */
/* 556 */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffe8),	/* Offset= -24 (534) */
/* 560 (SAFEARRAYUNION) */
        0x2a,	/* FC_ENCAPSULATED_UNION */
        0x89,	/* Switch type= FC_ULONG */
        NdrFcShort(0x20),	/* 32 */
        NdrFcShort(0xa),	/* 10 */
        NdrFcLong(0x8),	/* 8 */
        NdrFcShort(0xfe58),	/* Offset= -424 (146) */
        NdrFcLong(0xd),	/* 13 */
        NdrFcShort(0xfe86),	/* Offset= -378 (198) */
        NdrFcLong(0x9),	/* 9 */
        NdrFcShort(0xfeb4),	/* Offset= -332 (250) */
        NdrFcLong(0xc),	/* 12 */
        NdrFcShort(0xfed0),	/* Offset= -304 (284) */
        NdrFcLong(0x24),	/* 36 */
        NdrFcShort(0xff1c),	/* Offset= -228 (366) */
        NdrFcLong(0x800d),	/* 32781 */
        NdrFcShort(0xff5c),	/* Offset= -164 (436) */
        NdrFcLong(0x10),	/* 16 */
        NdrFcShort(0xff74),	/* Offset= -140 (466) */
        NdrFcLong(0x2),	/* 2 */
        NdrFcShort(0xff88),	/* Offset= -120 (492) */
        NdrFcLong(0x3),	/* 3 */
        NdrFcShort(0xff9c),	/* Offset= -100 (518) */
        NdrFcLong(0x14),	/* 20 */
        NdrFcShort(0xffb0),	/* Offset= -80 (544) */
        NdrFcShort(0xffff),
/* 628 (SAFEARRAYBOUND) */
        0x15,	/* FC_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 636 (SAFEARRAYBOUND[]) */
        0x1b,	/* FC_CARRAY */
        0x3,	/* 3 */
        NdrFcShort(0x8),	/* 8 */
        0x7,	/* Corr desc: field cDims, FC_USHORT */
        0x0,	/* no operators */
        NdrFcShort(0xffc8),	/* offset = -56 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffee),	/* Offset= -18 (628) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 650 (struct _wireSAFEARRAY) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x3,	/* 3 */
        NdrFcShort(0x38),	/* 56 */
        NdrFcShort(0xffee),	/* Offset= -18 (636) */
        NdrFcShort(0x0),	/* Offset= 0 (656) */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x39,	/* FC_ALIGNM8 */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xff97),	/* Offset= -105 (560) */
        0x5b,	/* FC_END */
/* 668 (wireSAFEARRAY) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffec),	/* Offset= -20 (650) */
/* 672 (signed char *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x02,	/* FC_CHAR */
        0x5c,	/* FC_PAD */
/* 676 (USHORT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x07,	/* FC_USHORT */
        0x5c,	/* FC_PAD */
/* 680 (ULONG *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x09,	/* FC_ULONG */
        0x5c,	/* FC_PAD */
/* 684 (INT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
/* 688 (UINT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x09,	/* FC_ULONG */
        0x5c,	/* FC_PAD */
/* 692 (BYTE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x02,	/* FC_CHAR */
        0x5c,	/* FC_PAD */
/* 696 (SHORT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x06,	/* FC_SHORT */
        0x5c,	/* FC_PAD */
/* 700 (LONG *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
/* 704 (FLOAT *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x0a,	/* FC_FLOAT */
        0x5c,	/* FC_PAD */
/* 708 (DOUBLE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x0c,	/* FC_DOUBLE */
        0x5c,	/* FC_PAD */
/* 712 (VARIANT_BOOL *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x06,	/* FC_SHORT */
        0x5c,	/* FC_PAD */
/* 716 (SCODE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x08,	/* FC_LONG */
        0x5c,	/* FC_PAD */
/* 720 (DATE *) */
        0x12, 0x8,	/* FC_UP [simple_pointer] */
        0x0c,	/* FC_DOUBLE */
        0x5c,	/* FC_PAD */
/* 724 (wireBSTR) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfd36),	/* Offset= -714 (12) */
/* 728 (wireBSTR *) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (724) */
/* 732 (wireVARIANT) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0x156),	/* Offset= 342 (1076) */
/* 736 (wireVARIANT *) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (732) */
/* 740 (CY *) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfd66),	/* Offset= -666 (76) */
/* 744 (DECIMAL *) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfd68),	/* Offset= -664 (82) */
/* 748 (IUnknown *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00000000),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 766 (IUnknown **) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xffec),	/* Offset= -20 (748) */
/* 770 (IDispatch *) */
        0x2f,	/* FC_IP */
        0x5a,	/* FC_CONSTANT_IID */
        NdrFcLong(0x00020400),
        NdrFcShort(0x0000),
        NdrFcShort(0x0000),
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46,

/* 788 (IDispatch **) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xffec),	/* Offset= -20 (770) */
/* 792 (wireSAFEARRAY) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xff70),	/* Offset= -144 (650) */
/* 796 (wireSAFEARRAY *) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (792) */
/* 800 (wireBRECORD) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfe26),	/* Offset= -474 (328) */
/* 804 (union ) */
        NdrFcShort(0x10),	/* 16 */
        NdrFcShort(0x2b),	/* 43 */
        NdrFcLong(0x0),	/* 0 */
        NdrFcShort(0x0),	/* No type */
        NdrFcLong(0x1),	/* 1 */
        NdrFcShort(0x0),	/* No type */
        NdrFcLong(0x10),	/* 16 */
        NdrFcShort(0x8002),	/* Simple arm type: FC_CHAR */
        NdrFcLong(0x12),	/* 18 */
        NdrFcShort(0x8007),	/* Simple arm type: FC_USHORT */
        NdrFcLong(0x13),	/* 19 */
        NdrFcShort(0x8009),	/* Simple arm type: FC_ULONG */
        NdrFcLong(0x16),	/* 22 */
        NdrFcShort(0x8008),	/* Simple arm type: FC_LONG */
        NdrFcLong(0x17),	/* 23 */
        NdrFcShort(0x8009),	/* Simple arm type: FC_ULONG */
        NdrFcLong(0x11),	/* 17 */
        NdrFcShort(0x8002),	/* Simple arm type: FC_CHAR */
        NdrFcLong(0x2),	/* 2 */
        NdrFcShort(0x8006),	/* Simple arm type: FC_SHORT */
        NdrFcLong(0x3),	/* 3 */
        NdrFcShort(0x8008),	/* Simple arm type: FC_LONG */
        NdrFcLong(0x4),	/* 4 */
        NdrFcShort(0x800a),	/* Simple arm type: FC_FLOAT */
        NdrFcLong(0x5),	/* 5 */
        NdrFcShort(0x800c),	/* Simple arm type: FC_DOUBLE */
        NdrFcLong(0xb),	/* 11 */
        NdrFcShort(0x8006),	/* Simple arm type: FC_SHORT */
        NdrFcLong(0xa),	/* 10 */
        NdrFcShort(0x8008),	/* Simple arm type: FC_LONG */
        NdrFcLong(0x7),	/* 7 */
        NdrFcShort(0x800c),	/* Simple arm type: FC_DOUBLE */
        NdrFcLong(0x8),	/* 8 */
        NdrFcShort(0xff4e),	/* Offset= -178 (724) */
        NdrFcLong(0x6),	/* 6 */
        NdrFcShort(0xfcc0),	/* Offset= -832 (76) */
        NdrFcLong(0xe),	/* 14 */
        NdrFcShort(0xfcc0),	/* Offset= -832 (82) */
        NdrFcLong(0xd),	/* 13 */
        NdrFcShort(0xfcc4),	/* Offset= -828 (92) */
        NdrFcLong(0x9),	/* 9 */
        NdrFcShort(0xfcd0),	/* Offset= -816 (110) */
        NdrFcLong(0x2000),	/* 8192 */
        NdrFcShort(0xff74),	/* Offset= -140 (792) */
        NdrFcLong(0x4010),	/* 16400 */
        NdrFcShort(0xfef6),	/* Offset= -266 (672) */
        NdrFcLong(0x4012),	/* 16402 */
        NdrFcShort(0xfef4),	/* Offset= -268 (676) */
        NdrFcLong(0x4013),	/* 16403 */
        NdrFcShort(0xfef2),	/* Offset= -270 (680) */
        NdrFcLong(0x4016),	/* 16406 */
        NdrFcShort(0xfef0),	/* Offset= -272 (684) */
        NdrFcLong(0x4017),	/* 16407 */
        NdrFcShort(0xfeee),	/* Offset= -274 (688) */
        NdrFcLong(0x4011),	/* 16401 */
        NdrFcShort(0xfeec),	/* Offset= -276 (692) */
        NdrFcLong(0x4002),	/* 16386 */
        NdrFcShort(0xfeea),	/* Offset= -278 (696) */
        NdrFcLong(0x4003),	/* 16387 */
        NdrFcShort(0xfee8),	/* Offset= -280 (700) */
        NdrFcLong(0x4004),	/* 16388 */
        NdrFcShort(0xfee6),	/* Offset= -282 (704) */
        NdrFcLong(0x4005),	/* 16389 */
        NdrFcShort(0xfee4),	/* Offset= -284 (708) */
        NdrFcLong(0x400b),	/* 16395 */
        NdrFcShort(0xfee2),	/* Offset= -286 (712) */
        NdrFcLong(0x400a),	/* 16394 */
        NdrFcShort(0xfee0),	/* Offset= -288 (716) */
        NdrFcLong(0x4007),	/* 16391 */
        NdrFcShort(0xfede),	/* Offset= -290 (720) */
        NdrFcLong(0x4008),	/* 16392 */
        NdrFcShort(0xfee0),	/* Offset= -288 (728) */
        NdrFcLong(0x400c),	/* 16396 */
        NdrFcShort(0xfee2),	/* Offset= -286 (736) */
        NdrFcLong(0x4006),	/* 16390 */
        NdrFcShort(0xfee0),	/* Offset= -288 (740) */
        NdrFcLong(0x400e),	/* 16398 */
        NdrFcShort(0xfede),	/* Offset= -290 (744) */
        NdrFcLong(0x400d),	/* 16397 */
        NdrFcShort(0xfeee),	/* Offset= -274 (766) */
        NdrFcLong(0x4009),	/* 16393 */
        NdrFcShort(0xfefe),	/* Offset= -258 (788) */
        NdrFcLong(0x6000),	/* 24576 */
        NdrFcShort(0xff00),	/* Offset= -256 (796) */
        NdrFcLong(0x24),	/* 36 */
        NdrFcShort(0xfefe),	/* Offset= -258 (800) */
        NdrFcLong(0x4024),	/* 16420 */
        NdrFcShort(0xfef8),	/* Offset= -264 (800) */
        NdrFcShort(0xffff),
/* 1068 */
        0x2b,	/* FC_NON_ENCAPSULATED_UNION */
        0x8,	/* FIXME: always FC_LONG */
        0x7,	/* Corr desc: field vt, FC_USHORT */
        0x0,	/* no operators */
        NdrFcShort(0xfff8),	/* offset = -8 */
        NdrFcShort(0xfef2),	/* Offset= -270 (804) */
/* 1076 (struct _wireVARIANT) */
        0x1a,	/* FC_BOGUS_STRUCT */
        0x7,	/* 7 */
        NdrFcShort(0x20),	/* 32 */
        NdrFcShort(0x0),
        NdrFcShort(0x0),	/* Offset= 0 (1082) */
        0x08,	/* FC_LONG */
        0x08,	/* FC_LONG */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x06,	/* FC_SHORT */
        0x4c,	/* FC_EMBEDDED_COMPLEX */
        0x0,
        NdrFcShort(0xffe8),	/* Offset= -24 (1068) */
        0x5c,	/* FC_PAD */
        0x5b,	/* FC_END */
/* 1096 (wireVARIANT) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xffea),	/* Offset= -22 (1076) */
/* 1100 (VARIANT) */
        0xb4,	/* FC_USER_MARSHAL */
        0x83,	/* Alignment= 3, Flags= 80 */
        NdrFcShort(0x1),	/* Function offset= 1 */
        NdrFcShort(0x18),	/* 24 */
        NdrFcShort(0x0),	/* 0 */
        NdrFcShort(0xfff4),	/* Offset= -12 (1096) */
/* 1110 (wireSAFEARRAY) */
        0x12, 0x0,		/* FC_UP */
        NdrFcShort(0xfe32),	/* Offset= -462 (650) */
/* 1114 (wirePSAFEARRAY) */
        0x12, 0x10,		/* FC_UP [pointer_deref] */
        NdrFcShort(0xfffa),	/* Offset= -6 (1110) */
/* 1118 (LPSAFEARRAY) */
        0xb4,	/* FC_USER_MARSHAL */
        0x83,	/* Alignment= 3, Flags= 80 */
        NdrFcShort(0x2),	/* Function offset= 2 */
        NdrFcShort(0x8),	/* 8 */
        NdrFcShort(0x0),	/* 0 */
        NdrFcShort(0xfff4),	/* Offset= -12 (1114) */
        0x0
};

static const unsigned short oleaut_offsets[] =
{
    26,     /* BSTR */
    36,     /* IUnknown* */
    54,     /* IDispatch* */
    1100,   /* VARIANT */
    1118,   /* LPSAFEARRAY */
};
#endif

static unsigned short write_oleaut_tfs(VARTYPE vt)
{
    switch (vt)
    {
    case VT_BSTR:       return oleaut_offsets[0];
    case VT_UNKNOWN:    return oleaut_offsets[1];
    case VT_DISPATCH:   return oleaut_offsets[2];
    case VT_VARIANT:    return oleaut_offsets[3];
    case VT_SAFEARRAY:  return oleaut_offsets[4];
    }

    return 0;
}

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

    if ((off = write_oleaut_tfs(desc->vt)))
        return off;

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

static HRESULT write_iface_fs(ITypeInfo *typeinfo, WORD funcs, WORD parentfuncs,
    unsigned char *type, size_t *typelen, unsigned char *proc, size_t *proclen,
    unsigned short *offset)
{
    unsigned short stack_offset;
    WORD proc_idx, param_idx;
    FUNCDESC *desc;
    HRESULT hr;

    for (proc_idx = 3; proc_idx < parentfuncs; proc_idx++)
    {
        if (offset)
            offset[proc_idx - 3] = -1;
    }

    for (proc_idx = 0; proc_idx < funcs; proc_idx++)
    {
        TRACE("proc %d\n", proc_idx);

        hr = ITypeInfo_GetFuncDesc(typeinfo, proc_idx, &desc);
        if (FAILED(hr)) return hr;

        if (offset)
            offset[proc_idx + parentfuncs - 3] = *proclen;

        write_proc_func_header(typeinfo, desc, proc_idx + parentfuncs, proc, proclen);

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
    WORD parentfuncs, const unsigned char **type_ret,
    const unsigned char **proc_ret, unsigned short **offset_ret)
{
    size_t typelen = sizeof(oleaut_tfs), proclen = 0;
    unsigned char *type, *proc;
    unsigned short *offset;
    HRESULT hr;

    hr = write_iface_fs(typeinfo, funcs, parentfuncs, NULL, &typelen, NULL, &proclen, NULL);
    if (FAILED(hr)) return hr;

    type = heap_alloc(typelen);
    proc = heap_alloc(proclen);
    offset = heap_alloc((parentfuncs + funcs - 3) * sizeof(*offset));
    if (!type || !proc || !offset)
    {
        hr = E_OUTOFMEMORY;
        goto err;
    }

    memcpy(type, oleaut_tfs, sizeof(oleaut_tfs));
    typelen = sizeof(oleaut_tfs);
    proclen = 0;

    hr = write_iface_fs(typeinfo, funcs, parentfuncs, type, &typelen, proc, &proclen, offset);
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

/* Dual interfaces report their size to be sizeof(IDispatchVtbl) and their
 * implemented type to be IDispatch. We need to retrieve the underlying
 * interface to get that information. */
static HRESULT get_real_typeinfo(ITypeInfo *typeinfo, ITypeInfo **ret)
{
    HREFTYPE reftype;
    TYPEATTR *attr;
    TYPEKIND kind;
    HRESULT hr;

    hr = ITypeInfo_GetTypeAttr(typeinfo, &attr);
    if (FAILED(hr)) return hr;
    kind = attr->typekind;
    ITypeInfo_ReleaseTypeAttr(typeinfo, attr);

    if (kind == TKIND_DISPATCH)
    {
        hr = ITypeInfo_GetRefTypeOfImplType(typeinfo, -1, &reftype);
        if (FAILED(hr)) return hr;

        return ITypeInfo_GetRefTypeInfo(typeinfo, reftype, ret);
    }

    ITypeInfo_AddRef(typeinfo);
    *ret = typeinfo;
    return S_OK;
}

HRESULT get_parent_iid(ITypeInfo *typeinfo, GUID *iid)
{
    ITypeInfo *parentinfo;
    HREFTYPE reftype;
    TYPEATTR *attr;
    HRESULT hr;

    hr = ITypeInfo_GetRefTypeOfImplType(typeinfo, 0, &reftype);
    if (FAILED(hr)) return hr;

    hr = ITypeInfo_GetRefTypeInfo(typeinfo, reftype, &parentinfo);
    if (FAILED(hr)) return hr;

    hr = ITypeInfo_GetTypeAttr(parentinfo, &attr);
    if (SUCCEEDED(hr))
    {
        *iid = attr->guid;
        ITypeInfo_ReleaseTypeAttr(parentinfo, attr);
    }

    ITypeInfo_Release(parentinfo);
    return hr;
}

static void init_stub_desc(MIDL_STUB_DESC *desc)
{
    desc->pfnAllocate = NdrOleAllocate;
    desc->pfnFree = NdrOleFree;
    desc->Version = 0x50002;
    desc->aUserMarshalQuadruple = oleaut_usermarshal;
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
        if (This->proxy.base_object)
            IUnknown_Release(This->proxy.base_object);
        if (This->proxy.base_proxy)
            IRpcProxyBuffer_Release(This->proxy.base_proxy);
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
    ULONG count, const GUID *parentiid, IRpcProxyBuffer **proxy, void **obj)
{
    if (!fill_stubless_table((IUnknownVtbl *)This->proxy_vtbl->Vtbl, count))
        return E_OUTOFMEMORY;

    if (!outer) outer = (IUnknown *)&This->proxy;

    This->proxy.IRpcProxyBuffer_iface.lpVtbl = &typelib_proxy_vtbl;
    This->proxy.PVtbl = This->proxy_vtbl->Vtbl;
    This->proxy.RefCount = 1;
    This->proxy.piid = This->proxy_vtbl->header.piid;
    This->proxy.pUnkOuter = outer;

    if (!IsEqualGUID(parentiid, &IID_IUnknown))
    {
        HRESULT hr = create_proxy(parentiid, NULL, &This->proxy.base_proxy,
            (void **)&This->proxy.base_object);
        if (FAILED(hr)) return hr;
    }

    *proxy = &This->proxy.IRpcProxyBuffer_iface;
    *obj = &This->proxy.PVtbl;
    IUnknown_AddRef((IUnknown *)*obj);

    return S_OK;
}

HRESULT WINAPI CreateProxyFromTypeInfo(ITypeInfo *typeinfo, IUnknown *outer,
    REFIID iid, IRpcProxyBuffer **proxy, void **obj)
{
    WORD funcs, parentfuncs, vtbl_size, i;
    struct typelib_proxy *This;
    TYPEATTR *typeattr;
    GUID parentiid;
    HRESULT hr;

    TRACE("typeinfo %p, outer %p, iid %s, proxy %p, obj %p.\n",
        typeinfo, outer, debugstr_guid(iid), proxy, obj);

    hr = get_real_typeinfo(typeinfo, &typeinfo);
    if (FAILED(hr))
        return hr;

    hr = ITypeInfo_GetTypeAttr(typeinfo, &typeattr);
    if (FAILED(hr))
        goto done;
    funcs = typeattr->cFuncs;
    vtbl_size = typeattr->cbSizeVft;
    ITypeInfo_ReleaseTypeAttr(typeinfo, typeattr);

    hr = get_parent_iid(typeinfo, &parentiid);
    if (FAILED(hr))
        goto done;

    parentfuncs = (vtbl_size / sizeof(void *)) - funcs;

    if (!(This = heap_alloc_zero(sizeof(*This))))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    init_stub_desc(&This->stub_desc);
    This->proxy_info.pStubDesc = &This->stub_desc;

    This->proxy_vtbl = heap_alloc_zero(sizeof(This->proxy_vtbl->header) + vtbl_size);
    if (!This->proxy_vtbl)
    {
        heap_free(This);
        hr = E_OUTOFMEMORY;
        goto done;
    }
    This->proxy_vtbl->header.pStublessProxyInfo = &This->proxy_info;
    This->iid = *iid;
    This->proxy_vtbl->header.piid = &This->iid;
    fill_delegated_proxy_table((IUnknownVtbl *)This->proxy_vtbl->Vtbl, parentfuncs);
    for (i = 0; i < funcs; i++)
        This->proxy_vtbl->Vtbl[parentfuncs + i] = (void *)-1;

    hr = build_format_strings(typeinfo, funcs, parentfuncs,
        &This->stub_desc.pFormatTypes, &This->proxy_info.ProcFormatString,
        &This->offset_table);
    if (FAILED(hr))
    {
        heap_free(This->proxy_vtbl);
        heap_free(This);
        goto done;
    }
    This->proxy_info.FormatStringOffset = &This->offset_table[-3];

    hr = typelib_proxy_init(This, outer, vtbl_size / sizeof(void *), &parentiid, proxy, obj);
    if (FAILED(hr))
    {
        heap_free((void *)This->stub_desc.pFormatTypes);
        heap_free((void *)This->proxy_info.ProcFormatString);
        heap_free((void *)This->offset_table);
        heap_free(This->proxy_vtbl);
        heap_free(This);
    }

done:
    ITypeInfo_Release(typeinfo);
    return hr;
}

struct typelib_stub
{
    cstdstubbuffer_delegating_t stub;
    IID iid;
    MIDL_STUB_DESC stub_desc;
    MIDL_SERVER_INFO server_info;
    CInterfaceStubVtbl stub_vtbl;
    unsigned short *offset_table;
    PRPC_STUB_FUNCTION *dispatch_table;
};

static ULONG WINAPI typelib_stub_Release(IRpcStubBuffer *iface)
{
    struct typelib_stub *This = CONTAINING_RECORD(iface, struct typelib_stub, stub.stub_buffer);
    ULONG refcount = InterlockedDecrement(&This->stub.stub_buffer.RefCount);

    TRACE("(%p) decreasing refs to %d\n", This, refcount);

    if (!refcount)
    {
        /* test_Release shows that native doesn't call Disconnect here.
           We'll leave it in for the time being. */
        IRpcStubBuffer_Disconnect(iface);

        if (This->stub.base_stub)
        {
            IRpcStubBuffer_Release(This->stub.base_stub);
            release_delegating_vtbl(This->stub.base_obj);
            heap_free(This->dispatch_table);
        }

        heap_free((void *)This->stub_desc.pFormatTypes);
        heap_free((void *)This->server_info.ProcString);
        heap_free(This->offset_table);
        heap_free(This);
    }

    return refcount;
}

static HRESULT typelib_stub_init(struct typelib_stub *This,
    IUnknown *server, const GUID *parentiid, IRpcStubBuffer **stub)
{
    HRESULT hr;

    hr = IUnknown_QueryInterface(server, This->stub_vtbl.header.piid,
        (void **)&This->stub.stub_buffer.pvServerObject);
    if (FAILED(hr))
    {
        WARN("Failed to get interface %s, hr %#x.\n",
            debugstr_guid(This->stub_vtbl.header.piid), hr);
        This->stub.stub_buffer.pvServerObject = server;
        IUnknown_AddRef(server);
    }

    if (!IsEqualGUID(parentiid, &IID_IUnknown))
    {
        This->stub.base_obj = get_delegating_vtbl(This->stub_vtbl.header.DispatchTableCount);
        hr = create_stub(parentiid, (IUnknown *)&This->stub.base_obj, &This->stub.base_stub);
        if (FAILED(hr))
        {
            release_delegating_vtbl(This->stub.base_obj);
            IUnknown_Release(This->stub.stub_buffer.pvServerObject);
            return hr;
        }
    }

    This->stub.stub_buffer.lpVtbl = &This->stub_vtbl.Vtbl;
    This->stub.stub_buffer.RefCount = 1;

    *stub = (IRpcStubBuffer *)&This->stub.stub_buffer;
    return S_OK;
}

HRESULT WINAPI CreateStubFromTypeInfo(ITypeInfo *typeinfo, REFIID iid,
    IUnknown *server, IRpcStubBuffer **stub)
{
    WORD funcs, parentfuncs, vtbl_size, i;
    struct typelib_stub *This;
    TYPEATTR *typeattr;
    GUID parentiid;
    HRESULT hr;

    TRACE("typeinfo %p, iid %s, server %p, stub %p.\n",
        typeinfo, debugstr_guid(iid), server, stub);

    hr = get_real_typeinfo(typeinfo, &typeinfo);
    if (FAILED(hr))
        goto done;

    hr = ITypeInfo_GetTypeAttr(typeinfo, &typeattr);
    if (FAILED(hr))
        goto done;
    funcs = typeattr->cFuncs;
    vtbl_size = typeattr->cbSizeVft;
    ITypeInfo_ReleaseTypeAttr(typeinfo, typeattr);

    hr = get_parent_iid(typeinfo, &parentiid);
    if (FAILED(hr))
        return hr;

    parentfuncs = (vtbl_size / sizeof(void *)) - funcs;

    if (!(This = heap_alloc_zero(sizeof(*This))))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    init_stub_desc(&This->stub_desc);
    This->server_info.pStubDesc = &This->stub_desc;

    hr = build_format_strings(typeinfo, funcs, parentfuncs,
        &This->stub_desc.pFormatTypes, &This->server_info.ProcString,
        &This->offset_table);
    if (FAILED(hr))
    {
        heap_free(This);
        goto done;
    }
    This->server_info.FmtStringOffset = &This->offset_table[-3];

    This->iid = *iid;
    This->stub_vtbl.header.piid = &This->iid;
    This->stub_vtbl.header.pServerInfo = &This->server_info;
    This->stub_vtbl.header.DispatchTableCount = vtbl_size / sizeof(void *);

    if (!IsEqualGUID(&parentiid, &IID_IUnknown))
    {
        This->dispatch_table = heap_alloc(vtbl_size);
        for (i = 3; i < parentfuncs; i++)
            This->dispatch_table[i - 3] = NdrStubForwardingFunction;
        for (; i < vtbl_size / sizeof(void *); i++)
            This->dispatch_table[i - 3] = (PRPC_STUB_FUNCTION)NdrStubCall2;
        This->stub_vtbl.header.pDispatchTable = &This->dispatch_table[-3];
        This->stub_vtbl.Vtbl = CStdStubBuffer_Delegating_Vtbl;
    }
    else
        This->stub_vtbl.Vtbl = CStdStubBuffer_Vtbl;
    This->stub_vtbl.Vtbl.Release = typelib_stub_Release;

    hr = typelib_stub_init(This, server, &parentiid, stub);
    if (FAILED(hr))
    {
        heap_free((void *)This->stub_desc.pFormatTypes);
        heap_free((void *)This->server_info.ProcString);
        heap_free(This->offset_table);
        heap_free(This);
    }

done:
    ITypeInfo_Release(typeinfo);
    return hr;
}
