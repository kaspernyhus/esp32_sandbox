#pragma once

#include "esp_err.h"

typedef enum {
    ML_PLAY,
    ML_PAUSE,
    ML_STOP,
    ML_NEXT,
    ML_PREV
} commands_t;

typedef enum {
    BS3200,
    BS9000
} models_t;


esp_err_t setIP(char *ip);
esp_err_t getIP(char *ip);

esp_err_t setModel(models_t model);
esp_err_t getModel(models_t *model);
esp_err_t command(commands_t command);
