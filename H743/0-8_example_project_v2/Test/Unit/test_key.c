#include "test_config.h"

#if ENABLE_TEST_KEY
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include "test_framework.h"
#include "elog.h"

#define LOG_TAG "TEST_KEY"

static gpio_key_t* key_main;
static gpio_key_t* key_main1;
static gpio_key_t* key_main2;
static KeyObserver key_observer;
static KeyObserver key_observer1;
static KeyObserver key_observer2;

static void on_key_event(gpio_key_t *key, KeyEvent event) {
    switch (event) {
        case KeyEvent_SinglePress:
            log_i("Key0 Event: Single Press");
            break;
        case KeyEvent_DoublePress:
            log_i("Key0 Event: Double Press");
            break;
        case KeyEvent_TriplePress:
            log_i("Key0 Event: Triple Press");
            break;
        case KeyEvent_LongPress:
            log_i("Key0 Event: Long Press");
            break;
        default:
            log_i("Key0 Event: Unknown %d", event);
            break;
    }
}

static void on_key_event1(gpio_key_t *key, KeyEvent event) {
    switch (event) {
        case KeyEvent_SinglePress:
            log_i("Key1 Event: Single Press");
            break;
        case KeyEvent_DoublePress:
            log_i("Key1 Event: Double Press");
            break;
        case KeyEvent_TriplePress:
            log_i("Key1 Event: Triple Press");
            break;
        case KeyEvent_LongPress:
            log_i("Key1 Event: Long Press");
            break;
        default:
            log_i("Key1 Event: Unknown %d", event);
            break;
    }
}

static void on_key_event2(gpio_key_t *key, KeyEvent event) {
    switch (event) {
        case KeyEvent_SinglePress:
            log_i("Key2 Event: Single Press");
            break;
        case KeyEvent_DoublePress:
            log_i("Key2 Event: Double Press");
            break;
        case KeyEvent_TriplePress:
            log_i("Key2 Event: Triple Press");
            break;
        case KeyEvent_LongPress:
            log_i("Key2 Event: Long Press");
            break;
        default:
            log_i("Key2 Event: Unknown %d", event);
            break;
    }
}

static void test_key_setup(void) {  
    gpio_driver_t *key_gpio = gpio_driver_get(GPIO_ID_KEY0);
    gpio_driver_t *key_gpio1 = gpio_driver_get(GPIO_ID_KEY1);
    gpio_driver_t *key_gpio2 = gpio_driver_get(GPIO_ID_KEY2);
    if (key_gpio == NULL || key_gpio1 == NULL || key_gpio2 == NULL) {
        log_e("GPIO_ID_KEY0 or GPIO_ID_KEY1 or GPIO_ID_KEY2 not found");
        return;
    }
    
    key_main = Key_Create(key_gpio, 0);
    key_observer.callback = on_key_event;
    Key_RegisterObserver(key_main, &key_observer);
    
    key_main1 = Key_Create(key_gpio1, 0);
    key_observer1.callback = on_key_event1;
    Key_RegisterObserver(key_main1, &key_observer1);
    
    key_main2 = Key_Create(key_gpio2, 0);
    key_observer2.callback = on_key_event2;
    Key_RegisterObserver(key_main2, &key_observer2);
    
    log_i("Key Test Setup: Initialized PA4 button (Active Low).");
}

static void test_key_loop(void) {
    Key_Update(key_main);
    Key_Update(key_main1);
    Key_Update(key_main2);
}

static void test_key_teardown(void) {
	Key_Destroy(key_main);
	Key_Destroy(key_main1);
	Key_Destroy(key_main2);
    log_i("Key Test Teardown: Done.");
}

REGISTER_TEST(Key, "Detect Single/Double/Long press on PA4",
              test_key_setup, test_key_loop,
              test_key_teardown);

#endif
