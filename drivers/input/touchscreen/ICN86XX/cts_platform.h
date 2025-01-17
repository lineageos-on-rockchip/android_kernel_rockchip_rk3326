#ifndef CTS_PLATFORM_H
#define CTS_PLATFORM_H

#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/bitops.h>
#include <linux/ctype.h>
#include <linux/unaligned/access_ok.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/spinlock.h>
#include <linux/rtmutex.h>
#include <linux/byteorder/generic.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/string.h>
#include <linux/suspend.h>
#include <linux/wakelock.h>
#include <linux/firmware.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#endif /* CONFIG_OF */

#include <linux/fb.h>
#include <linux/notifier.h>
#include <linux/time.h>
#include <linux/sched.h>

#ifdef CFG_CTS_DRM_NOTIFIER
#include <linux/msm_drm_notify.h>
#endif

#include "cts_config.h"

extern bool cts_show_debug_log;
extern struct timeval start_tv;
extern struct timeval end_tv;

#ifndef LOG_TAG
#define LOG_TAG         ""
#endif /* LOG_TAG */
#define cts_err(fmt, ...)   \
    print_info("<E>CTS-" LOG_TAG " " fmt"\n", ##__VA_ARGS__)
#define cts_warn(fmt, ...)  \
    print_info("<W>CTS-" LOG_TAG " " fmt"\n", ##__VA_ARGS__)
#define cts_info(fmt, ...)  \
    print_info("<I>CTS-" LOG_TAG " " fmt"\n", ##__VA_ARGS__)
    
#define cts_dbg(fmt, ...)   \
    do {                                                    \
        if (cts_show_debug_log)                                     \
            print_info("<D>CTS-" LOG_TAG " "fmt"\n", ##__VA_ARGS__);   \
    } while(0)

struct cts_device;
struct cts_device_touch_msg;
struct cts_device_gesture_info;

struct cts_platform_data {
    int irq;
    int int_gpio;
#ifdef CFG_CTS_HAS_RESET_PIN
    int rst_gpio;
#endif /* CFG_CTS_HAS_RESET_PIN */

    u32 res_x;
    u32 res_y;

#ifdef CONFIG_CTS_VIRTUALKEY
    u8  vkey_num;
    u8  vkey_state;
    u8  vkey_keycodes[CFG_CTS_MAX_VKEY_NUM];
#endif /* CONFIG_CTS_VIRTUALKEY */

    struct i2c_client *i2c_client;
    struct cts_device *cts_dev;

    struct input_dev *ts_input_dev;

#ifndef CONFIG_GENERIC_HARDIRQS
    struct work_struct ts_irq_work;
#endif /* CONFIG_GENERIC_HARDIRQS */

    struct rt_mutex dev_lock;
    struct spinlock irq_lock;
    bool            irq_is_disable;

#ifdef CONFIG_CTS_GESTURE
    u8  gesture_num;
    u8  gesture_keymap[CFG_CTS_NUM_GESTURE][2];
    bool irq_wake_enabled;
#endif /* CONFIG_CTS_GESTURE */

#ifdef CFG_CTS_FORCE_UP
	struct timer_list touch_event_timeout_timer;
#endif

#ifdef CONFIG_CTS_PM_FB_NOTIFIER
    struct notifier_block fb_notifier;
#endif /* CONFIG_CTS_PM_FB_NOTIFIER */

    u8 i2c_fifo_buf[CFG_CTS_MAX_I2C_XFER_SIZE];

};

extern size_t cts_plat_get_max_i2c_xfer_size(struct cts_platform_data *pdata);
extern u8 *cts_plat_get_i2c_xfer_buf(struct cts_platform_data *pdata, 
    size_t xfer_size);
extern int cts_plat_i2c_write(struct cts_platform_data *pdata, u8 i2c_addr,
        const void *src, size_t len, int retry, int delay);
extern int cts_plat_i2c_read(struct cts_platform_data *pdata, u8 i2c_addr,
        const u8 *wbuf, size_t wlen, void *rbuf, size_t rlen,
        int retry, int delay);
extern int cts_plat_is_i2c_online(struct cts_platform_data *pdata, u8 i2c_addr);

extern int cts_init_platform_data(struct cts_platform_data *pdata,
        struct i2c_client *i2c_client);
extern int cts_deinit_platform_data(struct cts_platform_data *pdata);
extern int cts_plat_request_resource(struct cts_platform_data *pdata, struct i2c_client *i2c_client);
extern void cts_plat_free_resource(struct cts_platform_data *pdata);

extern int cts_plat_request_irq(struct cts_platform_data *pdata);
extern void cts_plat_free_irq(struct cts_platform_data *pdata);
extern int cts_plat_enable_irq(struct cts_platform_data *pdata);
extern int cts_plat_disable_irq(struct cts_platform_data *pdata);

#if 1 //def CFG_CTS_HAS_RESET_PIN
extern int cts_plat_reset_device(struct cts_platform_data *pdata);
#else /* CFG_CTS_HAS_RESET_PIN */
//static inline int cts_plat_reset_device(struct cts_platform_data *pdata) {return 0;}
#endif /* CFG_CTS_HAS_RESET_PIN */

extern int cts_plat_init_touch_device(struct cts_platform_data *pdata);
extern void cts_plat_deinit_touch_device(struct cts_platform_data *pdata);
extern int cts_plat_process_touch_msg(struct cts_platform_data *pdata,
        struct cts_device_touch_msg *msgs, int num);
extern int cts_plat_release_all_touch(struct cts_platform_data *pdata);

#ifdef CONFIG_CTS_VIRTUALKEY
extern int cts_plat_init_vkey_device(struct cts_platform_data *pdata);
extern void cts_plat_deinit_vkey_device(struct cts_platform_data *pdata);
extern int cts_plat_process_vkey(struct cts_platform_data *pdata, u8 vkey_state);
extern int cts_plat_release_all_vkey(struct cts_platform_data *pdata);
#else /* CONFIG_CTS_VIRTUALKEY */
static inline int cts_plat_init_vkey_device(struct cts_platform_data *pdata) {return 0;}
static inline void cts_plat_deinit_vkey_device(struct cts_platform_data *pdata) {}
static inline int cts_plat_process_vkey(struct cts_platform_data *pdata, u8 vkey_state) {return 0;}
static inline int cts_plat_release_all_vkey(struct cts_platform_data *pdata) {return 0;}
#endif /* CONFIG_CTS_VIRTUALKEY */

#ifdef CONFIG_CTS_GESTURE
extern int cts_plat_enable_irq_wake(struct cts_platform_data *pdata);
extern int cts_plat_disable_irq_wake(struct cts_platform_data *pdata);

extern int cts_plat_init_gesture(struct cts_platform_data *pdata);
extern void cts_plat_deinit_gesture(struct cts_platform_data *pdata);
extern int cts_plat_process_gesture_info(struct cts_platform_data *pdata,
        struct cts_device_gesture_info *gesture_info);
#else /* CONFIG_CTS_GESTURE */
static inline int cts_plat_init_gesture(struct cts_platform_data *pdata) {return 0;}
static inline void cts_plat_deinit_gesture(struct cts_platform_data *pdata)  {}
#endif /* CONFIG_CTS_GESTURE */

#endif /* CTS_PLATFORM_H */

