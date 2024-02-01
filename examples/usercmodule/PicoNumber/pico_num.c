/*
  This example extends on features0 but demonstrates how to define a class.

  The Factorial class constructor takes an integer, and then the calculate
  method can be called to get the factorial.

  >>> import features4
  >>> f = features4.Factorial(4)
  >>> f.calculate()
  24
*/

// Include the header file to get access to the MicroPython API
//#include "py/runtime.h"
#include "m_string.h"
#include "q.h"

mp_obj_full_type_t mp_type_fix16;

// This is the internal state of a Factorial instance.
typedef struct {
    mp_obj_base_t base;
    q_t n;
} mp_obj_fix16_t;

// Essentially Factorial.__new__ (but also kind of __init__).
// Takes a single argument (the number to find the factorial of)
STATIC mp_obj_t fix16_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args_in) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, type);
    o->n = qint(mp_obj_get_int(args_in[0]));

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t add(mp_obj_t a_obj,mp_obj_t b_obj) {
    // Extract the integer from the MicroPython input object
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qadd(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_add_obj, add);

STATIC mp_obj_t sub(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qsub(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_sub_obj, sub);

STATIC mp_obj_t mul(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qmul(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_mul_obj, mul);

STATIC mp_obj_t div(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    check_div0(a->n,b->n);
    o->n = qdiv(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_div_obj, div);

STATIC mp_obj_t fdiv(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    check_div0(a->n,b->n);
    o->n = qfloor(qdiv(a->n,b->n)); 

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_fdiv_obj, fdiv);

STATIC mp_obj_t modulus(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    check_div0(a->n,b->n);
    o->n = qrem(a->n,b->n); 

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_modulus_obj, modulus);

STATIC mp_obj_t fix16_pow(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qpow(a->n,b->n); 

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_pow_obj, fix16_pow);

STATIC mp_obj_t fix16_invert(mp_obj_t a_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qinvert(a->n);
    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(fix16_invert_obj, fix16_invert);

STATIC mp_obj_t fix16_or(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qor(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_or_obj, fix16_or);

STATIC mp_obj_t fix16_and(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qand(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_and_obj, fix16_and);

STATIC mp_obj_t fix16_xor(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qxor(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_xor_obj, fix16_xor);

STATIC mp_obj_t fix16_lsl(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qlls(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_lsl_obj, fix16_lsl);

STATIC mp_obj_t fix16_asr(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qars(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_asr_obj, fix16_asr);

STATIC mp_obj_t fix16_lsr(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qlrs(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_lsr_obj, fix16_lsr);

STATIC mp_obj_t fix16_rotr(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qrotr(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_rotr_obj, fix16_rotr);

STATIC mp_obj_t fix16_rotl(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, mp_obj_get_type(a_obj));
    o->n = qrotl(a->n,b->n);

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_rotl_obj, fix16_rotl);

STATIC mp_obj_t fix16_less(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);
    return mp_obj_new_bool(a->n < b->n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_less_obj, fix16_less);

STATIC mp_obj_t fix16_eqless(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);
    return mp_obj_new_bool(a->n <= b->n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_eqless_obj, fix16_eqless);

STATIC mp_obj_t fix16_more(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);
    return mp_obj_new_bool(a->n > b->n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_more_obj, fix16_more);

STATIC mp_obj_t fix16_eqmore(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);
    return mp_obj_new_bool(a->n >= b->n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_eqmore_obj, fix16_eqmore);

STATIC mp_obj_t fix16_equal(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);
    return mp_obj_new_bool(a->n == b->n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_equal_obj, fix16_equal);

STATIC mp_obj_t fix16_notequal(mp_obj_t a_obj,mp_obj_t b_obj) {
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);
    return mp_obj_new_bool(a->n != b->n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_notequal_obj, fix16_notequal);

STATIC mp_obj_t to_string(mp_obj_t self_in) {
    mp_obj_fix16_t *self = MP_OBJ_TO_PTR(self_in);
    char temp[16]; //max length according to the string function docs
	const int r = qsprint(self->n, temp, sizeof temp);
    if (r==-1)
		mp_raise_ValueError(MP_ERROR_TEXT("Could not convert to string"));
    mp_obj_t obj = mp_obj_new_str(temp,r);
    return obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(fix16_to_string_obj, to_string);

STATIC mp_obj_t to_bytes(mp_obj_t self_in) {
    mp_obj_fix16_t *self = MP_OBJ_TO_PTR(self_in);
    char temp[16]; //max length according to the string function docs
	const int r = qsprint(self->n, temp, sizeof temp);
    if (r==-1)
		mp_raise_ValueError(MP_ERROR_TEXT("Could not convert to string"));
    mp_obj_t obj = mp_obj_new_bytes((byte *)temp,r);
    return obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(fix16_to_bytes_obj, to_bytes);

STATIC const mp_rom_map_elem_t fix16_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_tobytes), MP_OBJ_FROM_PTR(&fix16_to_bytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_tostr), MP_OBJ_FROM_PTR(&fix16_to_string_obj) }
};
STATIC MP_DEFINE_CONST_DICT(fix16_locals_dict, fix16_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    PicoNumber_type_Number,
    MP_QSTR_PicoNumber,
    MP_TYPE_FLAG_NONE,
    make_new, fix16_make_new,
    locals_dict, &fix16_locals_dict
    );

// Define all attributes of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
STATIC const mp_rom_map_elem_t fix16_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_PicoNumber) },
    { MP_ROM_QSTR(MP_QSTR_PicoNumber),    MP_ROM_PTR(&PicoNumber_type_Number) },
    { MP_ROM_QSTR(MP_QSTR_add),    MP_ROM_PTR(&fix16_add_obj) },
    { MP_ROM_QSTR(MP_QSTR_sub),    MP_ROM_PTR(&fix16_sub_obj) },
    { MP_ROM_QSTR(MP_QSTR_mul),    MP_ROM_PTR(&fix16_mul_obj) },
    { MP_ROM_QSTR(MP_QSTR_div),    MP_ROM_PTR(&fix16_div_obj) },
    { MP_ROM_QSTR(MP_QSTR_fdiv),    MP_ROM_PTR(&fix16_fdiv_obj) },
    { MP_ROM_QSTR(MP_QSTR_rem),    MP_ROM_PTR(&fix16_modulus_obj) },
    { MP_ROM_QSTR(MP_QSTR_pow),    MP_ROM_PTR(&fix16_pow_obj) },
    { MP_ROM_QSTR(MP_QSTR_invert),    MP_ROM_PTR(&fix16_invert_obj) },
    { MP_ROM_QSTR(MP_QSTR_or),    MP_ROM_PTR(&fix16_or_obj) },
    { MP_ROM_QSTR(MP_QSTR_and),    MP_ROM_PTR(&fix16_and_obj) },
    { MP_ROM_QSTR(MP_QSTR_xor),    MP_ROM_PTR(&fix16_xor_obj) },
    { MP_ROM_QSTR(MP_QSTR_lsl),    MP_ROM_PTR(&fix16_lsl_obj) },
    { MP_ROM_QSTR(MP_QSTR_asr),    MP_ROM_PTR(&fix16_asr_obj) },
    { MP_ROM_QSTR(MP_QSTR_lsr),    MP_ROM_PTR(&fix16_lsr_obj) },
    { MP_ROM_QSTR(MP_QSTR_rotl),    MP_ROM_PTR(&fix16_rotl_obj) },
    { MP_ROM_QSTR(MP_QSTR_rotr),    MP_ROM_PTR(&fix16_rotr_obj) },
    { MP_ROM_QSTR(MP_QSTR_less),    MP_ROM_PTR(&fix16_less_obj) },
    { MP_ROM_QSTR(MP_QSTR_eqless),    MP_ROM_PTR(&fix16_eqless_obj) },
    { MP_ROM_QSTR(MP_QSTR_more),    MP_ROM_PTR(&fix16_more_obj) },
    { MP_ROM_QSTR(MP_QSTR_eqmore),    MP_ROM_PTR(&fix16_eqmore_obj) },
    { MP_ROM_QSTR(MP_QSTR_equal),    MP_ROM_PTR(&fix16_equal_obj) },
    { MP_ROM_QSTR(MP_QSTR_notequal),    MP_ROM_PTR(&fix16_notequal_obj) },
};
STATIC MP_DEFINE_CONST_DICT(fix16_module_globals, fix16_module_globals_table);

// Define module object.
const mp_obj_module_t fix16_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&fix16_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_PicoNumber, fix16_user_cmodule);
