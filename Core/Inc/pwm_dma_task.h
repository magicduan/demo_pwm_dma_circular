#ifndef __PWM_DMA_TASK_H
#define __PWM_DMA_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif

#define EFF_BREATH_STEPS  20
#define EFF_BREATH_MIN    0x05
typedef enum {
    LED_EFFECT_NONE = 0, 
    LED_EFFECT_MARQUEE, 
    LED_EFFECT_BREATH,
    LED_EFFECT_WATER
}LED_EFFECTION;

typedef enum{
    EFF_STAT_INIT = 0,
    EFF_STAT_START,
    EFF_STAT_STOP,
    EFF_STAT_RUNNING
}EFFECION_STATUS;

typedef struct {
    uint8_t  direct; // 0 is for up, 1 for down : used in Breath and water effection
    uint8_t  org_color[3]; // Color for Water effection
    int16_t cur_led; // Current led for water
    uint8_t  data_ready; // 1: LED color data is ready, 0: nead remake LED color data
    uint32_t last_tickcount; // the last tickount value
}EFFECT_INTER_DATA;

typedef struct {
    uint8_t     effection;
    uint8_t     grp_led_count; //For Marquee and Water effection, the led nums for evey group.
                               // The total len of led color array shoule be times of grp_led_count; 
    uint32_t    interval; //Interval of lef effection, tick count
    uint8_t     status; 
    EFFECT_INTER_DATA eff_inter_data;
}PWM_DMA_LED_ACTION;

void pwm_led_run();
void pwm_led_effect_start(uint32_t dma_id);
void pwm_led_effect_stop(uint32_t dma_id);
void pwm_led_effect_set(uint32_t dma_id,uint8_t effection, uint32_t eff_interval,uint8_t grp_leds, uint32_t led_color );

extern PWM_DMA_DATA_STRUCT pwm_dma_data[PWM_LED_CHANNEL_MAX_COUNT];  
#ifdef __cplusplus
}
#endif

#endif 
