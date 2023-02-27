/**
 * @file interface_interal.c
 * @brief interface to the core
 *
 * Mostly copied from the interface written for the saf_hades module found here:
 * https://github.com/jananifernandez/HADES
 * Copyright (c) 2021 - Janani Fernandez & Leo McCormack,  (GPLv2 License)
 *
 * @author Leo McCormack
 * @date 10th August 2022
 */

#include "interface.h"
#include "interface_internal.h"

void interface_setCoreStatus(void* const hInt, INTERFACE_CORE_STATUS newStatus)
{
    interface_data *pData = (interface_data*)(hInt);
    if(newStatus==CORE_STATUS_NOT_INITIALISED){
        /* Pause until current initialisation is complete */
        while(pData->coreStatus == CORE_STATUS_INITIALISING)
            SAF_SLEEP(10);
    }
    pData->coreStatus = newStatus;
}

