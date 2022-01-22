#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
#include "pwm_dma_led.h"
#include "pwm_dma_task.h"

PWM_DMA_LED_ACTION pwm_led_effects[PWM_LED_CHANNEL_MAX_COUNT];

void dma_finishedCallback(PWM_DMA_DATA_STRUCT* p_dma_data)
{
    int dma_id = (int)(p_dma_data - pwm_dma_data);
    pwm_led_effects[dma_id].eff_inter_data.last_tickcount = osKernelGetTickCount();
    pwm_led_effects[dma_id].eff_inter_data.data_ready = 0;
}


int color_direct_change(uint8_t* color_buf,uint8_t step_value,uint8_t direct)
{
    int value,i;
    int    r_steps = step_value;
    if (direct == 0){ // Up direct

        for(i = 0; i <3; i++){
            value = 255 - color_buf[i];
            if (value == 0){
                return 0;
            }
            if(value < r_steps){
                r_steps = value;
            }
        }
    }else{
        r_steps = step_value;
        for(i = 0; i < 3; i++){
            if(color_buf[i] == EFF_BREATH_MIN){
                return 0;
            }
            value = color_buf[i] - EFF_BREATH_MIN;
            if(value >0 && value < r_steps){
                r_steps = value;
            }
        }
    }

    return r_steps;
}

void update_effect_breath(uint32_t dma_id)
{
    int i;
    uint8_t* p_colors = pwm_dma_data[dma_id].p_dma_colors;
    uint8_t steps = color_direct_change(p_colors,EFF_BREATH_STEPS,
                                    pwm_led_effects[dma_id].eff_inter_data.direct);
    if(steps == 0){
        pwm_led_effects[dma_id].eff_inter_data.direct++;
        if (pwm_led_effects[dma_id].eff_inter_data.direct > 1){
            pwm_led_effects[dma_id].eff_inter_data.direct = 0;
        }  
        steps = EFF_BREATH_STEPS;            
    }
    // Change First LED Color
    for(i = 0; i < 3;i++){
        if (pwm_led_effects[dma_id].eff_inter_data.direct == 0){ //UP
            if(p_colors[i] != 0 ){
                p_colors[i] += steps;
            }
        }else {
            if(p_colors[i] != 0 ){
                p_colors[i] -= steps;
            }
        }

    }

    // Change other LED Colors
    p_colors = (uint8_t*)(pwm_dma_data[dma_id].p_dma_colors + 3);
    for(i = 1; i < pwm_dma_data[dma_id].total_leds; i++){
        p_colors[0] = pwm_dma_data[dma_id].p_dma_colors[0];
        p_colors[1] = pwm_dma_data[dma_id].p_dma_colors[1];
        p_colors[2] = pwm_dma_data[dma_id].p_dma_colors[2];
        p_colors += 3;                                
    }
}

void update_effect_marquee(uint32_t dma_id)
{
    static uint8_t temp_buffer[10*3];
    uint8_t *p_dest,*p_colors;
    int i;
    uint32_t retain_leds = pwm_dma_data[dma_id].total_leds - pwm_led_effects[dma_id].grp_led_count;
    // Save the latest group led color to temp buffer
    p_colors = (uint8_t*)(pwm_dma_data[dma_id].p_dma_colors + 
                            3*(retain_leds));
    
    p_dest = temp_buffer;
    for( i = 0; i < pwm_led_effects[dma_id].grp_led_count; i++ ){
        p_dest[0] = p_colors[0];
        p_dest[1] = p_colors[1];
        p_dest[2] = p_colors[2];
        p_dest += 3;
        p_colors +=3;                
    }

    // Move data
    p_colors = (uint8_t*)(pwm_dma_data[dma_id].p_dma_colors + 
                            3*(retain_leds-1));
    p_dest = p_colors + 3;                        

    for(i = 0; i < retain_leds; i++){
        p_dest[0] = p_colors[0];
        p_dest[1] = p_colors[1];
        p_dest[2] = p_colors[2];
        p_dest -= 3;
        p_colors -=3;          
    }

    // Copy Temp data to header
    p_colors = pwm_dma_data[dma_id].p_dma_colors;
    p_dest = temp_buffer;
    for(i = 0; i < pwm_led_effects[dma_id].grp_led_count; i++){
        p_colors[0] = p_dest[0];
        p_colors[1] = p_dest[1];
        p_colors[2] = p_dest[2];            
        p_dest += 3;
        p_colors +=3;         
    }
}

void update_effect_water(uint32_t dma_id)
{
    uint8_t* p_colors;
    uint8_t* p_org_color;
    int i;
    if ( pwm_led_effects[dma_id].eff_inter_data.direct == 0 ){
        if (pwm_dma_data[dma_id].total_leds < (pwm_led_effects[dma_id].eff_inter_data.cur_led 
                                                    + pwm_led_effects[dma_id].grp_led_count))
        {
            pwm_led_effects[dma_id].eff_inter_data.direct = 1;   
            pwm_led_effects[dma_id].eff_inter_data.cur_led -= pwm_led_effects[dma_id].grp_led_count;                                        
        }
    }else{
        if ( pwm_led_effects[dma_id].eff_inter_data.cur_led < 0 )
        {
            pwm_led_effects[dma_id].eff_inter_data.direct = 0;
            pwm_led_effects[dma_id].eff_inter_data.cur_led = 0; 
        }
    }
    
    if ( pwm_led_effects[dma_id].eff_inter_data.direct == 0 )
    {

        p_colors = pwm_dma_data[dma_id].p_dma_colors + 3*pwm_led_effects[dma_id].eff_inter_data.cur_led;
        p_org_color = pwm_led_effects[dma_id].eff_inter_data.org_color;

        for(i = 0; i < pwm_led_effects[dma_id].grp_led_count; i++){
            p_colors[0] = p_org_color[0];
            p_colors[1] = p_org_color[1];
            p_colors[2] = p_org_color[2];            
            p_colors +=3;                 
        }
        pwm_led_effects[dma_id].eff_inter_data.cur_led += pwm_led_effects[dma_id].grp_led_count;
    }else{

        p_colors = pwm_dma_data[dma_id].p_dma_colors + 3*pwm_led_effects[dma_id].eff_inter_data.cur_led;
        p_org_color = pwm_led_effects[dma_id].eff_inter_data.org_color;

        for(i = 0; i < pwm_led_effects[dma_id].grp_led_count; i++){
            p_colors[0] = 0;
            p_colors[1] = 0;
            p_colors[2] = 0;            
            p_colors +=3;                 
        }
        pwm_led_effects[dma_id].eff_inter_data.cur_led -= pwm_led_effects[dma_id].grp_led_count;        
    }
}

void update_led_colors(uint32_t dma_id)
{
    switch(pwm_led_effects[dma_id].effection){
        case LED_EFFECT_ONCE:
            pwm_led_effects[dma_id].eff_inter_data.data_ready = 1;
            break;        
        case LED_EFFECT_BREATH:
            update_effect_breath(dma_id);
            pwm_led_effects[dma_id].eff_inter_data.data_ready = 1;
            break;
        
        case LED_EFFECT_MARQUEE:
            update_effect_marquee(dma_id);
            pwm_led_effects[dma_id].eff_inter_data.data_ready = 1;        
            break;

        case LED_EFFECT_WATER:
            update_effect_water(dma_id);
            pwm_led_effects[dma_id].eff_inter_data.data_ready = 1;  
            break;      
        default:
            break;      
    }
}


/**
* @brief Initial the effection for dma_id
* 
* @param dma_id: the PWM_DMA item (0 - PWM_LED_CHANNEL_MAX_COUNT-1)
* @param effection: the effection of LED strips refer to LED_EFFECTION
* @param eff_interval: interval time  
* @param grp_leds: the number of each groups LEDs for LED_EFFECT_BREATH and LED_EFFECT_WATER
* @param led_color: the led color for LED_EFFECT_BREATH and LED_EFFECT_WATER
* @retval 0: Success; 
*/
int pwm_led_effect_set(uint32_t dma_id,uint8_t effection, uint32_t eff_interval,uint8_t grp_leds, uint32_t led_color )
{
    if (pwm_dma_data[dma_id].htim == NULL){
        return PWM_DMA_ERROR_INIT;
    }
    pwm_led_effects[dma_id].effection = effection;
    pwm_led_effects[dma_id].interval = eff_interval;
    pwm_led_effects[dma_id].grp_led_count = grp_leds;
    pwm_led_effects[dma_id].status = EFF_STAT_INIT; 

    pwm_led_effects[dma_id].eff_inter_data.last_tickcount = 0;
    pwm_led_effects[dma_id].eff_inter_data.data_ready = 1;
    pwm_dma_data[dma_id].send_finishedCallback = dma_finishedCallback;
    if (pwm_dma_data[dma_id].p_dma_colors != NULL){  
        uint8_t color[3];
        int i;
        color[0] = (uint8_t)((led_color >> 16) & 0x000000FF);
        color[1] = (uint8_t)((led_color >> 8) & 0x000000FF);
        color[2] = (uint8_t)(led_color & 0x000000FF);
        uint8_t *p_colors = pwm_dma_data[dma_id].p_dma_colors;
        switch(pwm_led_effects[dma_id].effection){
            case LED_EFFECT_BREATH:
                for(i = 0; i < pwm_dma_data[dma_id].total_leds; i++){
                    p_colors[0] = color[0];
                    p_colors[1] = color[1];
                    p_colors[2] = color[2]; 
                    p_colors += 3;                                  
                }
                pwm_led_effects[dma_id].eff_inter_data.direct = 0;
                break;
            case LED_EFFECT_WATER:
                for(i = 0; i < grp_leds; i++){
                    p_colors[0] = color[0];
                    p_colors[1] = color[1];
                    p_colors[2] = color[2]; 
                    p_colors += 3;                                  
                }
                pwm_led_effects[dma_id].eff_inter_data.org_color[0] = color[0];
                pwm_led_effects[dma_id].eff_inter_data.org_color[1] = color[1];
                pwm_led_effects[dma_id].eff_inter_data.org_color[2] = color[2];                                
                pwm_led_effects[dma_id].eff_inter_data.direct = 0;
                pwm_led_effects[dma_id].eff_inter_data.cur_led += grp_leds;
                break; 
            default: 
                // For LED_EFFECT_MARQUEE, use the led colors;
                break;               
        }
    }

    return PWM_DMA_OK;
}

void pwm_led_effect_start(uint32_t dma_id)
{
    pwm_led_effects[dma_id].status = EFF_STAT_START;
}

int pwm_led_effect_stop(uint32_t dma_id,uint8_t b_block,uint32_t timeout)
{
    pwm_led_effects[dma_id].status = EFF_STAT_STOP;
    if (b_block){
        int count = 0;
        while(pwm_dma_data[dma_id].inter_dma_data.b_busy && count < timeout){
            count++;
            osDelay(1);
        }
        if (count >= timeout){
            return PWM_DMA_ERROR_BUSY;
        }
    }

    return PWM_DMA_OK;
}

void pwm_led_effect_run(uint32_t dma_id)
{
    if (pwm_led_effects[dma_id].effection == LED_EFFECT_NONE ||
        pwm_dma_data[dma_id].htim == NULL ||
        pwm_led_effects[dma_id].status == EFF_STAT_INIT ||
        pwm_led_effects[dma_id].status == EFF_STAT_STOP ||        
        pwm_dma_data[dma_id].inter_dma_data.b_busy == 1){
            return; 
    }

    if (pwm_led_effects[dma_id].eff_inter_data.data_ready == 0) { 
        update_led_colors(dma_id);
    }

    uint32_t diff_tickCount = osKernelGetTickCount() - pwm_led_effects[dma_id].eff_inter_data.last_tickcount;
    if (pwm_led_effects[dma_id].eff_inter_data.last_tickcount == 0 ||
        diff_tickCount >= pwm_led_effects[dma_id].interval){
            if (pwm_dma_send(dma_id,0) != 0){
                return;
            };
            if (pwm_led_effects[dma_id].effection == LED_EFFECT_ONCE){
                pwm_led_effects[dma_id].status = EFF_STAT_STOP;
            }
    }
}

/**
* @brief Run the effection for LED trips.
*        You should Call the funtion in LED effection task
* @retval None
*/
void pwm_led_run()
{
    for (int i = 0; i < PWM_LED_CHANNEL_MAX_COUNT; i++){    
        pwm_led_effect_run(i);
    }
}