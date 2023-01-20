#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define STACK_SIZE 2048

static const char *TAG = "main";

StackType_t xMasterStack[STACK_SIZE];
StaticTask_t xMasterTCB;

void MasterTask(void* parameters){
    while(1){
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    xTaskCreateStatic(MasterTask, "xMaster", STACK_SIZE, (void*)1,  1, xMasterStack, &xMasterTCB);
}
