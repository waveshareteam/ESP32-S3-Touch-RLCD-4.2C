idf_component_register(SRCS "esp_rlcd_st7306.c" "st7306_rgb565.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_lcd esp_driver_spi
                       PRIV_REQUIRES esp_driver_gpio esp_timer)
