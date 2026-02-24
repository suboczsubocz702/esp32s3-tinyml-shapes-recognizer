idf_component_register(SRCS  "main.cpp" "display.c" "image_generator.c" "model.cc"
            INCLUDE_DIRS "."
	    REQUIRES esp-tflite-micro 
	    esp_driver_spi
	    esp_driver_gpio
	    esp_timer)
