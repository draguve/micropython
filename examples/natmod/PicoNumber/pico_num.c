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
#include "fix16.h"

mp_obj_full_type_t mp_type_fix16;

// This is the internal state of a Factorial instance.
typedef struct {
    mp_obj_base_t base;
    fix16_t n;
} mp_obj_fix16_t;

// Essentially Factorial.__new__ (but also kind of __init__).
// Takes a single argument (the number to find the factorial of)
STATIC mp_obj_t fix16_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args_in) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, type);
    o->n = fix16_from_int(mp_obj_get_int(args_in[0]));

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t add(mp_obj_t a_obj,mp_obj_t b_obj) {
    // Extract the integer from the MicroPython input object
    mp_obj_fix16_t *a = MP_OBJ_TO_PTR(a_obj);
    mp_obj_fix16_t *b = MP_OBJ_TO_PTR(b_obj);

//    mp_obj_fix16_t *o = mp_obj_malloc(mp_obj_fix16_t, type);
    a->n = fix16_add(a->n,b->n);

    return MP_OBJ_FROM_PTR(a);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fix16_add_obj, add);

// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    // This must be first, it sets up the globals dict and other things
    MP_DYNRUNTIME_INIT_ENTRY

    mp_store_global(MP_QSTR_add, MP_OBJ_FROM_PTR(&fix16_add_obj));

    // Initialise the type.
    mp_type_fix16.base.type = (void*)&mp_type_type;
    mp_type_fix16.flags = MP_TYPE_FLAG_NONE;
    mp_type_fix16.name = MP_QSTR_PicoNumber;
    MP_OBJ_TYPE_SET_SLOT(&mp_type_fix16, make_new, fix16_make_new, 0);
//    factorial_locals_dict_table[0] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_calculate), MP_OBJ_FROM_PTR(&factorial_calculate_obj) };
//    MP_OBJ_TYPE_SET_SLOT(&mp_type_factorial, locals_dict, (void*)&factorial_locals_dict, 1);

    // Make the Factorial type available on the module.
    mp_store_global(MP_QSTR_PicoNumber, MP_OBJ_FROM_PTR(&mp_type_fix16));

    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}
