#include <stdio.h>
#include "sdkconfig.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define OLED_MASTER_FREQ_HZ 100000
#define OLED_MASTER_SCL_IO 25
#define OLED_MASTER_SDA_IO 26

#define PAGE_NUM 8      // 8 pages each 8 bits "tall"
#define SEGMENT_NUM 128 // 128 segments "wide"


