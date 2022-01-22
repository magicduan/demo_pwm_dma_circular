# Use less memory to implement control WS2812 LED IC in STM32 G431RB Board
* implement WS2812 LED IC control in G431RB board.
* Use DMA Circular mode and data width "Byte" to reduce the memory usage for big LED numbers 
* Can control PWM_LED_CHANNEL_MAX_COUNT(default = 4) WS2812 LED IC in same time.
* Usage: 
   void pwm_dma_init(uint32_t dma_id, TIM_HandleTypeDef *htim, uint32_t channel,
                 uint8_t* p_colors, uint32_t leds_count );
   int pwm_dma_send(uint32_t dma_id,uint8_t b_block);
   
   The function "pwm_dma_init" used to set the DMA Timer,channel and LED colors.
   The function "pwm_dma_send" used to send DMA data according to dma_ix.
                 
* The main code implements is in "pwm_dma_led.h" and "pwm_dma_led.c"
# Add LED effctions such as WaterFlow, Breathe, Marquee 
* Implemented LED effection : WaterFlow, Breathe, Marquee
* Usage:
    /* Set the effection according to LED_EFFECT_ONCE(Just Send once LED data),LED_EFFECT_MARQUEE, LED_EFFECT_BREATH,LED_EFFECT_WATER */
    int pwm_led_effect_set(uint32_t dma_id,uint8_t effection, uint32_t eff_interval,uint8_t grp_leds, uint32_t led_color );
    /* Start Effection */
    void pwm_led_effect_start(uint32_t dma_id);
    /* Stop Effection in block mode or no-block mode */
    int pwm_led_effect_stop(uint32_t dma_id,uint8_t b_block,uint32_t timeout);
    /* Call this function in loop task, which will update the led data to implement effection*/
    void pwm_led_run();
* The main code implements is in "pwm_dma_task.c"
*
