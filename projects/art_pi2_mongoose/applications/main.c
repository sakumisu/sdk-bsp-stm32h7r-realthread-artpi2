#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "mongoose.h"
#include "net.h"

#define LED_PIN         GET_PIN(O, 5)
#define KEY_PIN         GET_PIN(C, 13)
#define BLINK_PERIOD_MS 1000

static void timer_fn(void *arg)
{
    rt_pin_write(LED_PIN, !rt_pin_read(LED_PIN));
    (void) arg;
}

static void run_mongoose(void)
{
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  mg_log_set(MG_LL_DEBUG);
  mg_timer_add(&mgr, BLINK_PERIOD_MS, MG_TIMER_REPEAT, timer_fn, NULL);
  net_init(&mgr);
  for (;;)
  {
    mg_mgr_poll(&mgr, 0);
    rt_thread_mdelay(10);
  }
}

int main(void)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT);

    while (rt_pin_read(KEY_PIN))
    {
      rt_pin_write(LED_PIN, !rt_pin_read(LED_PIN));
      rt_thread_mdelay(100);
    }

    run_mongoose();

    return RT_EOK;
}

#include "stm32h7rsxx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal XSPI2_BASE */
    SCB->VTOR = XSPI2_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);
