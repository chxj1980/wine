/*
 * Copyright 2010 Piotr Caban for CodeWeavers
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

#include <stdarg.h>

#include "msvcp.h"

#include "windef.h"
#include "winbase.h"
#include "wine/debug.h"
WINE_DEFAULT_DEBUG_CHANNEL(msvcp);

#define CLASS_IS_SIMPLE_TYPE          1
#define CLASS_HAS_VIRTUAL_BASE_CLASS  4

void WINAPI _CxxThrowException(exception*,const cxx_exception_type*);

/* vtables */
extern const vtable_ptr MSVCP_type_info_vtable;
extern const vtable_ptr MSVCP_exception_vtable;
/* ??_7bad_alloc@std@@6B@ */
extern const vtable_ptr MSVCP_bad_alloc_vtable;
/* ??_7logic_error@std@@6B@ */
extern const vtable_ptr MSVCP_logic_error_vtable;
/* ??_7length_error@std@@6B@ */
extern const vtable_ptr MSVCP_length_error_vtable;
/* ??_7out_of_range@std@@6B@ */
extern const vtable_ptr MSVCP_out_of_range_vtable;
extern const vtable_ptr MSVCP_invalid_argument_vtable;
/* ??_7runtime_error@std@@6B@ */
extern const vtable_ptr MSVCP_runtime_error_vtable;
extern const vtable_ptr MSVCP_failure_vtable;

static void MSVCP_type_info_dtor(type_info * _this)
{
    free(_this->name);
}

DEFINE_THISCALL_WRAPPER(MSVCP_type_info_vector_dtor,8)
void * __thiscall MSVCP_type_info_vector_dtor(type_info * _this, unsigned int flags)
{
    TRACE("(%p %x)\n", _this, flags);
    if (flags & 2)
    {
        /* we have an array, with the number of elements stored before the first object */
        INT_PTR i, *ptr = (INT_PTR *)_this - 1;

        for (i = *ptr - 1; i >= 0; i--) MSVCP_type_info_dtor(_this + i);
        MSVCRT_operator_delete(ptr);
    }
    else
    {
        MSVCP_type_info_dtor(_this);
        if (flags & 1) MSVCRT_operator_delete(_this);
    }
    return _this;
}

DEFINE_RTTI_DATA0( type_info, 0, ".?AVtype_info@@" );

static exception* MSVCP_exception_ctor(exception *this, const char *name)
{
    TRACE("(%p %s)\n", this, name);

    this->vtable = &MSVCP_exception_vtable;
    if(name) {
        unsigned int name_len = strlen(name) + 1;
        this->name = malloc(name_len);
        memcpy(this->name, name, name_len);
        this->do_free = TRUE;
    } else {
        this->name = NULL;
        this->do_free = FALSE;
    }
    return this;
}

DEFINE_THISCALL_WRAPPER(MSVCP_exception_copy_ctor,8)
exception* __thiscall MSVCP_exception_copy_ctor(exception *this, const exception *rhs)
{
    TRACE("(%p,%p)\n", this, rhs);

    if(!rhs->do_free) {
        this->vtable = &MSVCP_exception_vtable;
        this->name = rhs->name;
        this->do_free = FALSE;
    } else
        MSVCP_exception_ctor(this, rhs->name);
    TRACE("name = %s\n", this->name);
    return this;
}

DEFINE_THISCALL_WRAPPER(MSVCP_exception_dtor,4)
void __thiscall MSVCP_exception_dtor(exception *this)
{
    TRACE("(%p)\n", this);
    this->vtable = &MSVCP_exception_vtable;
    if(this->do_free)
        free(this->name);
}

DEFINE_THISCALL_WRAPPER(MSVCP_exception_vector_dtor, 8)
void * __thiscall MSVCP_exception_vector_dtor(exception *this, unsigned int flags)
{
    TRACE("%p %x\n", this, flags);
    if(flags & 2) {
        /* we have an array, with the number of elements stored before the first object */
        INT_PTR i, *ptr = (INT_PTR *)this-1;

        for(i=*ptr-1; i>=0; i--)
            MSVCP_exception_dtor(this+i);
        MSVCRT_operator_delete(ptr);
    } else {
        MSVCP_exception_dtor(this);
        if(flags & 1)
            MSVCRT_operator_delete(this);
    }

    return this;
}

DEFINE_RTTI_DATA0(exception, 0, ".?AVexception@std@@");
DEFINE_CXX_DATA0(exception, MSVCP_exception_dtor);

/* ?_Doraise@bad_alloc@std@@MBEXXZ */
/* ?_Doraise@bad_alloc@std@@MEBAXXZ */
/* ?_Doraise@logic_error@std@@MBEXXZ */
/* ?_Doraise@logic_error@std@@MEBAXXZ */
/* ?_Doraise@length_error@std@@MBEXXZ */
/* ?_Doraise@length_error@std@@MEBAXXZ */
/* ?_Doraise@out_of_range@std@@MBEXXZ */
/* ?_Doraise@out_of_range@std@@MEBAXXZ */
/* ?_Doraise@runtime_error@std@@MBEXXZ */
/* ?_Doraise@runtime_error@std@@MEBAXXZ */
DEFINE_THISCALL_WRAPPER(MSVCP_exception__Doraise, 4)
void __thiscall MSVCP_exception__Doraise(exception *this)
{
    FIXME("(%p) stub\n", this);
}

DEFINE_THISCALL_WRAPPER(MSVCP_exception_what,4)
const char* __thiscall MSVCP_exception_what(exception * this)
{
    TRACE("(%p) returning %s\n", this, this->name);
    return this->name ? this->name : "Unknown exception";
}

/* bad_alloc class data */
typedef exception bad_alloc;

/* ??0bad_alloc@std@@QAE@PBD@Z */
/* ??0bad_alloc@std@@QEAA@PEBD@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_bad_alloc_ctor, 8)
bad_alloc* __thiscall MSVCP_bad_alloc_ctor(bad_alloc *this, const char *name)
{
    TRACE("%p %s\n", this, name);
    MSVCP_exception_ctor(this, name);
    this->vtable = &MSVCP_bad_alloc_vtable;
    return this;
}

/* ??0bad_alloc@std@@QAE@XZ */
/* ??0bad_alloc@std@@QEAA@XZ */
DEFINE_THISCALL_WRAPPER(MSVCP_bad_alloc_default_ctor, 4)
bad_alloc* __thiscall MSVCP_bad_alloc_default_ctor(bad_alloc *this)
{
    return MSVCP_bad_alloc_ctor(this, "bad allocation");
}

/* ??0bad_alloc@std@@QAE@ABV01@@Z */
/* ??0bad_alloc@std@@QEAA@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_bad_alloc_copy_ctor, 8)
bad_alloc* __thiscall MSVCP_bad_alloc_copy_ctor(bad_alloc *this, const bad_alloc *rhs)
{
    TRACE("%p %p\n", this, rhs);
    MSVCP_exception_copy_ctor(this, rhs);
    this->vtable = &MSVCP_bad_alloc_vtable;
    return this;
}

/* ??1bad_alloc@std@@UAE@XZ */
/* ??1bad_alloc@std@@UEAA@XZ */
DEFINE_THISCALL_WRAPPER(MSVCP_bad_alloc_dtor, 4)
void __thiscall MSVCP_bad_alloc_dtor(bad_alloc *this)
{
    TRACE("%p\n", this);
    MSVCP_exception_dtor(this);
}

DEFINE_THISCALL_WRAPPER(MSVCP_bad_alloc_vector_dtor, 8)
void * __thiscall MSVCP_bad_alloc_vector_dtor(bad_alloc *this, unsigned int flags)
{
    TRACE("%p %x\n", this, flags);
    if(flags & 2) {
        /* we have an array, with the number of elements stored before the first object */
        INT_PTR i, *ptr = (INT_PTR *)this-1;

        for(i=*ptr-1; i>=0; i--)
            MSVCP_bad_alloc_dtor(this+i);
        MSVCRT_operator_delete(ptr);
    } else {
        MSVCP_bad_alloc_dtor(this);
        if(flags & 1)
            MSVCRT_operator_delete(this);
    }

    return this;
}

/* ??4bad_alloc@std@@QAEAAV01@ABV01@@Z */
/* ??4bad_alloc@std@@QEAAAEAV01@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_bad_alloc_assign, 8)
bad_alloc* __thiscall MSVCP_bad_alloc_assign(bad_alloc *this, const bad_alloc *assign)
{
    MSVCP_bad_alloc_dtor(this);
    return MSVCP_bad_alloc_copy_ctor(this, assign);
}

DEFINE_RTTI_DATA1(bad_alloc, 0, &exception_rtti_base_descriptor, ".?AVbad_alloc@std@@");
DEFINE_CXX_DATA1(bad_alloc, &exception_cxx_type_info, MSVCP_bad_alloc_dtor);

/* logic_error class data */
typedef struct {
    exception e;
    basic_string_char str;
} logic_error;

static logic_error* MSVCP_logic_error_ctor(
        logic_error *this, const char *name)
{
    TRACE("%p %s\n", this, name);
    this->e.vtable = &MSVCP_logic_error_vtable;
    this->e.name = NULL;
    this->e.do_free = FALSE;
    basic_string_char_ctor_cstr(&this->str, name);
    return this;
}

/* ??0logic_error@std@@QAE@ABV01@@Z */
/* ??0logic_error@std@@QEAA@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_logic_error_copy_ctor, 8)
logic_error* __thiscall MSVCP_logic_error_copy_ctor(
        logic_error *this, const logic_error *rhs)
{
    TRACE("%p %p\n", this, rhs);
    MSVCP_exception_copy_ctor(&this->e, &rhs->e);
    basic_string_char_copy_ctor(&this->str, &rhs->str);
    this->e.vtable = &MSVCP_logic_error_vtable;
    return this;
}

/* ??0logic_error@std@@QAE@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
/* ??0logic_error@std@@QEAA@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_logic_error_ctor_bstr, 8)
logic_error* __thiscall MSVCP_logic_error_ctor_bstr(logic_error *this, const basic_string_char *str)
{
    TRACE("(%p %p)\n", this, str);
    return MSVCP_logic_error_ctor(this, basic_string_char_c_str(str));
}

/* ??1logic_error@std@@UAE@XZ */
/* ??1logic_error@std@@UEAA@XZ */
/* ??1length_error@std@@UAE@XZ */
/* ??1length_error@std@@UEAA@XZ */
/* ??1out_of_range@std@@UAE@XZ */
/* ??1out_of_range@std@@UEAA@XZ */
/* ??1runtime_error@std@@UAE@XZ */
/* ??1runtime_error@std@@UEAA@XZ */
DEFINE_THISCALL_WRAPPER(MSVCP_logic_error_dtor, 4)
void __thiscall MSVCP_logic_error_dtor(logic_error *this)
{
    TRACE("%p\n", this);
    MSVCP_exception_dtor(&this->e);
    basic_string_char_dtor(&this->str);
}

DEFINE_THISCALL_WRAPPER(MSVCP_logic_error_vector_dtor, 8)
void* __thiscall MSVCP_logic_error_vector_dtor(
        logic_error *this, unsigned int flags)
{
    TRACE("%p %x\n", this, flags);
    if(flags & 2) {
        /* we have an array, with the number of elements stored before the first object */
        INT_PTR i, *ptr = (INT_PTR *)this-1;

        for(i=*ptr-1; i>=0; i--)
            MSVCP_logic_error_dtor(this+i);
        MSVCRT_operator_delete(ptr);
    } else {
        MSVCP_logic_error_dtor(this);
        if(flags & 1)
            MSVCRT_operator_delete(this);
    }

    return this;
}

/* ??4logic_error@std@@QAEAAV01@ABV01@@Z */
/* ??4logic_error@std@@QEAAAEAV01@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_logic_error_assign, 8)
logic_error* __thiscall MSVCP_logic_error_assign(logic_error *this, const logic_error *assign)
{
    MSVCP_logic_error_dtor(this);
    return MSVCP_logic_error_copy_ctor(this, assign);
}

/* ?what@logic_error@std@@UBEPBDXZ */
/* ?what@logic_error@std@@UEBAPEBDXZ */
DEFINE_THISCALL_WRAPPER(MSVCP_logic_error_what, 4)
const char* __thiscall MSVCP_logic_error_what(logic_error *this)
{
    TRACE("%p\n", this);
    return basic_string_char_c_str(&this->str);
}

DEFINE_RTTI_DATA1(logic_error, 0, &exception_rtti_base_descriptor, ".?AVlogic_error@std@@");
DEFINE_CXX_DATA1(logic_error, &exception_cxx_type_info, MSVCP_logic_error_dtor);

/* length_error class data */
typedef logic_error length_error;

static length_error* MSVCP_length_error_ctor(
        length_error *this, const char *name)
{
    TRACE("%p %s\n", this, name);
    MSVCP_logic_error_ctor(this, name);
    this->e.vtable = &MSVCP_length_error_vtable;
    return this;
}

/* ??0length_error@std@@QAE@ABV01@@Z */
/* ??0length_error@std@@QEAA@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_length_error_copy_ctor, 8)
length_error* __thiscall MSVCP_length_error_copy_ctor(
        length_error *this, const length_error *rhs)
{
    TRACE("%p %p\n", this, rhs);
    MSVCP_logic_error_copy_ctor(this, rhs);
    this->e.vtable = &MSVCP_length_error_vtable;
    return this;
}

/* ??0length_error@std@@QAE@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
/* ??0length_error@std@@QEAA@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_length_error_ctor_bstr, 8)
length_error* __thiscall MSVCP_length_error_ctor_bstr(length_error *this, const basic_string_char *str)
{
        TRACE("(%p %p)\n", this, str);
        return MSVCP_length_error_ctor(this, basic_string_char_c_str(str));
}

/* ??4length_error@std@@QAEAAV01@ABV01@@Z */
/* ??4length_error@std@@QEAAAEAV01@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_length_error_assign, 8)
length_error* __thiscall MSVCP_length_error_assign(length_error *this, const length_error *assign)
{
    MSVCP_logic_error_dtor(this);
    return MSVCP_length_error_copy_ctor(this, assign);
}

DEFINE_RTTI_DATA2(length_error, 0, &logic_error_rtti_base_descriptor, &exception_rtti_base_descriptor, ".?AVlength_error@std@@");
DEFINE_CXX_DATA2(length_error, &logic_error_cxx_type_info, &exception_cxx_type_info, MSVCP_logic_error_dtor);

/* out_of_range class data */
typedef logic_error out_of_range;

static out_of_range* MSVCP_out_of_range_ctor(
        out_of_range *this, const char *name)
{
    TRACE("%p %s\n", this, name);
    MSVCP_logic_error_ctor(this, name);
    this->e.vtable = &MSVCP_out_of_range_vtable;
    return this;
}

/* ??0out_of_range@std@@QAE@ABV01@@Z */
/* ??0out_of_range@std@@QEAA@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_out_of_range_copy_ctor, 8)
out_of_range* __thiscall MSVCP_out_of_range_copy_ctor(
        out_of_range *this, const out_of_range *rhs)
{
    TRACE("%p %p\n", this, rhs);
    MSVCP_logic_error_copy_ctor(this, rhs);
    this->e.vtable = &MSVCP_out_of_range_vtable;
    return this;
}

/* ??0out_of_range@std@@QAE@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
/* ??0out_of_range@std@@QEAA@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_out_of_range_ctor_bstr, 8)
out_of_range* __thiscall MSVCP_out_of_range_ctor_bstr(out_of_range *this, const basic_string_char *str)
{
    TRACE("(%p %p)\n", this, str);
    return MSVCP_out_of_range_ctor(this, basic_string_char_c_str(str));
}

/* ??4out_of_range@std@@QAEAAV01@ABV01@@Z */
/* ??4out_of_range@std@@QEAAAEAV01@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_out_of_range_assign, 8)
out_of_range* __thiscall MSVCP_out_of_range_assign(out_of_range *this, const out_of_range *assign)
{
    MSVCP_logic_error_dtor(this);
    return MSVCP_out_of_range_copy_ctor(this, assign);
}

DEFINE_RTTI_DATA2(out_of_range, 0, &logic_error_rtti_base_descriptor, &exception_rtti_base_descriptor, ".?AVout_of_range@std@@");
DEFINE_CXX_DATA2(out_of_range, &logic_error_cxx_type_info, &exception_cxx_type_info, MSVCP_logic_error_dtor);

/* invalid_argument class data */
typedef logic_error invalid_argument;

static invalid_argument* MSVCP_invalid_argument_ctor(
        invalid_argument *this, const char *name)
{
    TRACE("%p %s\n", this, name);
    MSVCP_logic_error_ctor(this, name);
    this->e.vtable = &MSVCP_invalid_argument_vtable;
    return this;
}

DEFINE_THISCALL_WRAPPER(MSVCP_invalid_argument_copy_ctor, 8)
invalid_argument* __thiscall MSVCP_invalid_argument_copy_ctor(
        invalid_argument *this, invalid_argument *rhs)
{
    TRACE("%p %p\n", this, rhs);
    MSVCP_logic_error_copy_ctor(this, rhs);
    this->e.vtable = &MSVCP_invalid_argument_vtable;
    return this;
}

DEFINE_RTTI_DATA2(invalid_argument, 0, &logic_error_rtti_base_descriptor, &exception_rtti_base_descriptor, ".?AVinvalid_argument@std@@");
DEFINE_CXX_DATA2(invalid_argument, &logic_error_cxx_type_info,  &exception_cxx_type_info, MSVCP_logic_error_dtor);

/* runtime_error class data */
typedef logic_error runtime_error;

static runtime_error* MSVCP_runtime_error_ctor(
        runtime_error *this, const char *name)
{
    TRACE("%p %s\n", this, name);
    MSVCP_logic_error_ctor(this, name);
    this->e.vtable = &MSVCP_runtime_error_vtable;
    return this;
}

/* ??0runtime_error@std@@QAE@ABV01@@Z */
/* ??0runtime_error@std@@QEAA@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_runtime_error_copy_ctor, 8)
runtime_error* __thiscall MSVCP_runtime_error_copy_ctor(
        runtime_error *this, const runtime_error *rhs)
{
    TRACE("%p %p\n", this, rhs);
    MSVCP_logic_error_copy_ctor(this, rhs);
    this->e.vtable = &MSVCP_runtime_error_vtable;
    return this;
}

/* ??0runtime_error@std@@QAE@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
/* ??0runtime_error@std@@QEAA@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_runtime_error_ctor_bstr, 8)
runtime_error* __thiscall MSVCP_runtime_error_ctor_bstr(runtime_error *this, const basic_string_char *str)
{
    TRACE("(%p %p)\n", this, str);
    return MSVCP_runtime_error_ctor(this, basic_string_char_c_str(str));
}

DEFINE_THISCALL_WRAPPER(MSVCP_runtime_error_vector_dtor, 8)
void* __thiscall MSVCP_runtime_error_vector_dtor(
        runtime_error *this, unsigned int flags)
{
    TRACE("%p %x\n", this, flags);
    return MSVCP_logic_error_vector_dtor(this, flags);
}

/* ??4runtime_error@std@@QAEAAV01@ABV01@@Z */
/* ??4runtime_error@std@@QEAAAEAV01@AEBV01@@Z */
DEFINE_THISCALL_WRAPPER(MSVCP_runtime_error_assign, 8)
runtime_error* __thiscall MSVCP_runtime_error_assign(runtime_error *this, const runtime_error *assign)
{
    MSVCP_logic_error_dtor(this);
    return MSVCP_runtime_error_copy_ctor(this, assign);
}

DEFINE_RTTI_DATA1(runtime_error, 0, &exception_rtti_base_descriptor, ".?AVruntime_error@std@@");
DEFINE_CXX_DATA1(runtime_error, &exception_cxx_type_info, MSVCP_logic_error_dtor);

/* ?what@runtime_error@std@@UBEPBDXZ */
/* ?what@runtime_error@std@@UEBAPEBDXZ */
DEFINE_THISCALL_WRAPPER(MSVCP_runtime_error_what, 4)
const char* __thiscall MSVCP_runtime_error_what(runtime_error *this)
{
    TRACE("%p\n", this);
    return basic_string_char_c_str(&this->str);
}

/* failure class data */
typedef runtime_error failure;

static failure* MSVCP_failure_ctor(
        failure *this, const char *name)
{
    TRACE("%p %s\n", this, name);
    MSVCP_runtime_error_ctor(this, name);
    this->e.vtable = &MSVCP_failure_vtable;
    return this;
}

DEFINE_THISCALL_WRAPPER(MSVCP_failure_copy_ctor, 8)
failure* __thiscall MSVCP_failure_copy_ctor(
        failure *this, failure *rhs)
{
    TRACE("%p %p\n", this, rhs);
    MSVCP_runtime_error_copy_ctor(this, rhs);
    this->e.vtable = &MSVCP_failure_vtable;
    return this;
}

DEFINE_THISCALL_WRAPPER(MSVCP_failure_dtor, 4)
void __thiscall MSVCP_failure_dtor(failure *this)
{
    TRACE("%p\n", this);
    MSVCP_logic_error_dtor(this);
}

DEFINE_THISCALL_WRAPPER(MSVCP_failure_vector_dtor, 8)
void* __thiscall MSVCP_failure_vector_dtor(
        failure *this, unsigned int flags)
{
    TRACE("%p %x\n", this, flags);
    return MSVCP_runtime_error_vector_dtor(this, flags);
}

DEFINE_THISCALL_WRAPPER(MSVCP_failure_what, 4)
const char* __thiscall MSVCP_failure_what(failure *this)
{
    TRACE("%p\n", this);
    return MSVCP_runtime_error_what(this);
}

DEFINE_RTTI_DATA2(failure, 0, &runtime_error_rtti_base_descriptor, &exception_rtti_base_descriptor, ".?AVfailure@std@@");
DEFINE_CXX_DATA2(failure, &runtime_error_cxx_type_info, &exception_cxx_type_info, MSVCP_logic_error_dtor);

/* ?_Nomemory@std@@YAXXZ */
void __cdecl _Nomemory(void)
{
    TRACE("()\n");
    throw_exception(EXCEPTION_BAD_ALLOC, NULL);
}

#ifndef __GNUC__
void __asm_dummy_vtables(void) {
#endif
    __ASM_VTABLE(type_info,
            VTABLE_ADD_FUNC(MSVCP_type_info_vector_dtor));
    __ASM_VTABLE(exception,
            VTABLE_ADD_FUNC(MSVCP_exception_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_exception_what)
            VTABLE_ADD_FUNC(MSVCP_exception__Doraise));
    __ASM_VTABLE(bad_alloc,
            VTABLE_ADD_FUNC(MSVCP_bad_alloc_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_exception_what)
            VTABLE_ADD_FUNC(MSVCP_exception__Doraise));
    __ASM_VTABLE(logic_error,
            VTABLE_ADD_FUNC(MSVCP_logic_error_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_logic_error_what)
            VTABLE_ADD_FUNC(MSVCP_exception__Doraise));
    __ASM_VTABLE(length_error,
            VTABLE_ADD_FUNC(MSVCP_logic_error_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_logic_error_what)
            VTABLE_ADD_FUNC(MSVCP_exception__Doraise));
    __ASM_VTABLE(out_of_range,
            VTABLE_ADD_FUNC(MSVCP_logic_error_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_logic_error_what)
            VTABLE_ADD_FUNC(MSVCP_exception__Doraise));
    __ASM_VTABLE(invalid_argument,
            VTABLE_ADD_FUNC(MSVCP_logic_error_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_logic_error_what)
            VTABLE_ADD_FUNC(MSVCP_exception__Doraise));
    __ASM_VTABLE(runtime_error,
            VTABLE_ADD_FUNC(MSVCP_runtime_error_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_runtime_error_what)
            VTABLE_ADD_FUNC(MSVCP_exception__Doraise));
    __ASM_VTABLE(failure,
            VTABLE_ADD_FUNC(MSVCP_failure_vector_dtor)
            VTABLE_ADD_FUNC(MSVCP_failure_what));
#ifndef __GNUC__
}
#endif

/* Internal: throws selected exception */
void throw_exception(exception_type et, const char *str)
{
    switch(et) {
    case EXCEPTION_RERAISE:
        _CxxThrowException(NULL, NULL);
    case EXCEPTION: {
        exception e;
        MSVCP_exception_ctor(&e, str);
        _CxxThrowException(&e, &exception_cxx_type);
    }
    case EXCEPTION_BAD_ALLOC: {
        bad_alloc e;
        MSVCP_bad_alloc_ctor(&e, str);
        _CxxThrowException(&e, &bad_alloc_cxx_type);
    }
    case EXCEPTION_LOGIC_ERROR: {
        logic_error e;
        MSVCP_logic_error_ctor(&e, str);
        _CxxThrowException((exception*)&e, &logic_error_cxx_type);
    }
    case EXCEPTION_LENGTH_ERROR: {
        length_error e;
        MSVCP_length_error_ctor(&e, str);
        _CxxThrowException((exception*)&e, &length_error_cxx_type);
    }
    case EXCEPTION_OUT_OF_RANGE: {
        out_of_range e;
        MSVCP_out_of_range_ctor(&e, str);
        _CxxThrowException((exception*)&e, &out_of_range_cxx_type);
    }
    case EXCEPTION_INVALID_ARGUMENT: {
        invalid_argument e;
        MSVCP_invalid_argument_ctor(&e, str);
        _CxxThrowException((exception*)&e, &invalid_argument_cxx_type);
    }
    case EXCEPTION_RUNTIME_ERROR: {
        runtime_error e;
        MSVCP_runtime_error_ctor(&e, str);
        _CxxThrowException((exception*)&e, &runtime_error_cxx_type);
    }
    case EXCEPTION_FAILURE: {
        failure e;
        MSVCP_failure_ctor(&e, str);
        _CxxThrowException((exception*)&e, &failure_cxx_type);
    }
    default:
        ERR("exception type not handled: %d\n", et);
    }
}

void init_exception(void *base)
{
#ifdef __x86_64__
    init_type_info_rtti(base);
    init_exception_rtti(base);
    init_bad_alloc_rtti(base);
    init_logic_error_rtti(base);
    init_length_error_rtti(base);
    init_out_of_range_rtti(base);
    init_invalid_argument_rtti(base);
    init_runtime_error_rtti(base);
    init_failure_rtti(base);

    init_exception_cxx(base);
    init_bad_alloc_cxx(base);
    init_logic_error_cxx(base);
    init_length_error_cxx(base);
    init_out_of_range_cxx(base);
    init_invalid_argument_cxx(base);
    init_runtime_error_cxx(base);
    init_failure_cxx(base);
#endif
}
