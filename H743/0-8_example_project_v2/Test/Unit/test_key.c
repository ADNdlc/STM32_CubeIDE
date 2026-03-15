#include "test_config.h"

#if ENABLE_TEST_KEY
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include "test_framework.h"
#include "elog.h"

#define LOG_TAG "TEST_KEY"

static gpio_key_t* key_main;
static KeyObserver key_observer;

static void on_key_event(gpio_key_t *key, KeyEvent event) {
    switch (event) {
        case KeyEvent_SinglePress:
            log_i("Key Event: Single Press");
            break;
        case KeyEvent_DoublePress:
            log_i("Key Event: Double Press");
            break;
        case KeyEvent_TriplePress:
            log_i("Key Event: Triple Press");
            break;
        case KeyEvent_LongPress:
            log_i("Key Event: Long Press");
            break;
        default:
            log_i("Key Event: Unknown %d", event);
            break;
    }
}

static void test_key_setup(void) {
    gpio_driver_t *key_gpio = gpio_driver_get(GPIO_ID_KEY0);
    if (key_gpio == NULL) {
        log_e("GPIO_ID_KEY0 not found");
        return;
    }
    
    key_main = Key_Create(key_gpio, 0);
    key_observer.callback = on_key_event;
    Key_RegisterObserver(key_main, &key_observer);
    
    log_i("Key Test Setup: Initialized PA4 button (Active Low).");
}

static void test_key_loop(void) {
    Key_Update(key_main);
}

static void test_key_teardown(void) {
    log_i("Key Test Teardown: Done.");
}

REGISTER_TEST(Key, "Detect Single/Double/Long press on PA4",
              test_key_setup, test_key_loop,
              test_key_teardown);

#endif
