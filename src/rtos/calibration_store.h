#ifndef CALIBRATION_STORE_H
#define CALIBRATION_STORE_H

#include <stdbool.h>

bool calibration_store_load(float *mq7_ro, float *mq135_ro);
bool calibration_store_save(float mq7_ro, float mq135_ro);

#endif
