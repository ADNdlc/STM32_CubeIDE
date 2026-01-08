#include "project_cfg.h"
#if USE_Simulator
#include "elog.h"
#include "virtual_device.h"
#include "thing_model.h"
#include "assets_handler/res_manager.h"

#define DEV_ID "test2"

static bool device_prop_set_cb(struct thing_device_t* dev, const char* prop_id, thing_value_t value)
{
    if (strcmp(dev->device_id, DEV_ID) == 0)
    {
        if (strcmp(prop_id, "led0") == 0)
        {
            log_i("Hardware: led0 set to %d", value.b);
            return true;
        }
        else if (strcmp(prop_id, "led1") == 0)
        {
            log_i("Hardware: led1 set to %d", value.b);
            return true;
        }
    }
    return false;
}

void devices_init(void) {
    log_i("Initializing hardware devices for Thing Model...");

    static thing_property_t device_props[] = {
        {.id = "led0",
         .name = "Light0",
         .type = THING_PROP_TYPE_SWITCH,
         .cloud_sync = true},
        {.id = "led1",
         .name = "Light1",
         .type = THING_PROP_TYPE_SWITCH,
         .cloud_sync = true},
    };

    thing_device_t device_tmpl = { .device_id = DEV_ID,
                                  .name = "TestDev",
                                  .prop_count = 2,
                                  .icon = res_get_src(RES_IMG_IMG_LIGHT),
                                  .properties = device_props,
                                  .on_prop_set = device_prop_set_cb };

    // 3. 注册物模型
    thing_model_register(&device_tmpl);


    log_i("Device registration completed.");
}

#endif

