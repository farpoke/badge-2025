#include <py/runtime.h>

static const mp_rom_map_elem_t badge_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_badge) },
};
static MP_DEFINE_CONST_DICT(badge_module_globals, badge_module_globals_table);

const mp_obj_module_t badge_module = { // NOLINT
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&badge_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_badge, badge_module);
