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
#include "py/dynruntime.h"
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

mp_map_elem_t fix16_locals_dict_table[2];
STATIC MP_DEFINE_CONST_DICT(fix16_locals_dict, fix16_locals_dict_table);

// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    // This must be first, it sets up the globals dict and other things
    MP_DYNRUNTIME_INIT_ENTRY

    //put the add function as a static global function
    mp_store_global(MP_QSTR_add, MP_OBJ_FROM_PTR(&fix16_add_obj));
    mp_store_global(MP_QSTR_sub, MP_OBJ_FROM_PTR(&fix16_sub_obj));
    mp_store_global(MP_QSTR_mul, MP_OBJ_FROM_PTR(&fix16_mul_obj));
    mp_store_global(MP_QSTR_div, MP_OBJ_FROM_PTR(&fix16_div_obj));
    mp_store_global(MP_QSTR_fdiv, MP_OBJ_FROM_PTR(&fix16_fdiv_obj));
    mp_store_global(MP_QSTR_fdiv, MP_OBJ_FROM_PTR(&fix16_modulus_obj));
    mp_store_global(MP_QSTR_pow, MP_OBJ_FROM_PTR(&fix16_pow_obj));

    // Initialise the type.
    mp_type_fix16.base.type = (void*)&mp_type_type;
    mp_type_fix16.flags = MP_TYPE_FLAG_NONE;
    mp_type_fix16.name = MP_QSTR_PicoNumber;
    MP_OBJ_TYPE_SET_SLOT(&mp_type_fix16, make_new, fix16_make_new, 0);

    fix16_locals_dict_table[0] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_tostr), MP_OBJ_FROM_PTR(&fix16_to_string_obj) };
    fix16_locals_dict_table[1] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_tobytes), MP_OBJ_FROM_PTR(&fix16_to_bytes_obj) };
    MP_OBJ_TYPE_SET_SLOT(&mp_type_fix16, locals_dict, (void*)&fix16_locals_dict, 2);

    // Make the Factorial type available on the module.
    mp_store_global(MP_QSTR_PicoNumber, MP_OBJ_FROM_PTR(&mp_type_fix16));

    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}
