idf_component_register(SRCS "main.c"
                            "src/power_control.c"
                            "src/gpio_control.c"
                            "src/button_control.c"
                            "src/display_control.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver)