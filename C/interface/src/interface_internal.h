/**
 * @file interface_interal.h
 * @brief interface to the core
 *
 * Mostly copied from the interface written for the saf_hades module found here:
 * https://github.com/jananifernandez/HADES
 * Copyright (c) 2021 - Janani Fernandez & Leo McCormack,  (GPLv2 License)
 *
 * @author Leo McCormack
 * @date 10th August 2022
 */

#ifndef __INTERFACE_INTERNAL_H_INCLUDED__
#define __INTERFACE_INTERNAL_H_INCLUDED__

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include "interface.h"
#include "proposed.h"
#include "saf.h"
#include "saf_externals.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Current status of the processing loop
 *
 * These are used to keep things thread-safe. i.e., the core will not be
 * initialised if the currently configured core is being used to process a
 * block of audio. Likewise, if the core is being initialised, then the
 * "process" functions are bypassed.
 */
typedef enum {
    PROC_STATUS_ONGOING = 0, /**< Core is processing input audio, and should
                              *   not be reinitialised at this time. */
    PROC_STATUS_NOT_ONGOING  /**< Core is not processing input audio, and may
                              *   be reinitialised if needed. */
}PROC_STATUS;


/* ========================================================================== */
/*                            Internal Parameters                             */
/* ========================================================================== */

#ifndef FRAME_SIZE
# define FRAME_SIZE ( 256 )
#endif
#define MAX_NUM_SH_SIGNALS ( (MAX_SH_ORDER + 1)*(MAX_SH_ORDER + 1)  )    /* (L+1)^2 */
#define HOP_SIZE ( 128 )
#if (FRAME_SIZE % HOP_SIZE != 0)
# error "FRAME_SIZE must be an integer multiple of HOP_SIZE"
#endif 

/* ========================================================================== */
/*                                 Structures                                 */
/* ========================================================================== */

/** Main structure for the interface */
typedef struct _interface {
    /* audio buffers and afSTFT stuff */
    float** inputFrameTD;                    /**< Input frame; nInputs x FRAME_SIZE */ 
    float** outputFrameTD;                   /**< Output frame; nOutputs x FRAME_SIZE */
    float fs;                                /**< Sampling rate */

    /* Internal */
    int MAIR_SOFA_isLoadedFLAG;              /**< 0: no MAIR SOFA file has been loaded, so do not render audio; 1: SOFA file HAS been loaded */
    proposed_analysis_handle hAna;           /**< Analysis handle */
    proposed_synthesis_handle hSyn;          /**< Synthesis handle */
    proposed_param_container_handle hPCon;   /**< Parameter Container handle */
    proposed_signal_container_handle hSCon;  /**< Signal Container handle */
    INTERFACE_CORE_STATUS coreStatus;        /**< see #INTERFACE_CORE_STATUS */
    float progressBar0_1;                    /**< Progress bar value [0..1] */
    char* progressBarText;                   /**< Progress bar text; INTERFACE_PROGRESSBARTEXT_CHAR_LENGTH x 1*/
    PROC_STATUS procStatus;                  /**< see #_PROC_STATUS */
    float head_orientation_xyz[3];           /**< Head orientation as unit length Cartesian vector */

    /* Local copy of internal parameter vectors (for optional thread-safe GUI plotting) */
    int nBands_local;                        /**< Number of bands used for plotting */
    int nDirs_local;                         /**< Number of directions */
    float* freqVector_local;                 /**< Local frequency vector; nBands_local x 1 */
    float* streamBalBands_local;             /**< Local stream balance vector; nBands_local x 1 */
    float* grid_dirs_xyz_local;              /**< Local grid directions in Cartesian coords; nDirs_local x 1 */
    float* histogram_local;                  /**< Local histogram; nDirs_local x 1 */

    /* IR data */
    int nMics;                               /**< Number of microphones/hydrophones in the array */
    int nDirs;                               /**< Number of measurement directions/IRs */
    int IRlength;                            /**< Length of IRs, in samples */
    float IR_fs;                             /**< Sample rate used for measuring the IRs */

    /* user parameters */
    int favour2Daccuracy;
    int enableEPbeamformers;
    int enableDiffEQ_HRTFs;
    int enableDiffEQ_ATFs;
    INTERFACE_DOF_OPTIONS renderingMode;     /**< See #INTERFACE_DOF_OPTIONS */
    proposed_binaural_config binConfig;      /**< Binaural configuration settings */
    char* sofa_filepath_MAIR;                /**< microphone array IRs; absolute/relative file path for a sofa file */
    int useDefaultHRIRsFLAG;                 /**< 0: use specified sofa file, 1: use default HRIR set */
    char* sofa_filepath_HRIR;                /**< HRIRs; absolute/relevative file path for a sofa file */ 
    float yaw;                               /**< yaw (Euler) rotation angle, in degrees */
    float roll;                              /**< roll (Euler) rotation angle, in degrees */
    float pitch;                             /**< pitch (Euler) rotation angle, in degrees */
    int bFlipYaw;                            /**< flag to flip the sign of the yaw rotation angle */
    int bFlipPitch;                          /**< flag to flip the sign of the pitch rotation angle */
    int bFlipRoll;                           /**< flag to flip the sign of the roll rotation angle */
    INTERFACE_DISTANCE_MAPS distMapOption;   /**< see #INTERFACE_DISTANCE_MAPS */
    float sourceDistance;                    /**< Source distance in metres */
    int enableSourceDirectivity;             /**< Flag, 0: disabled, 1: enabled */
    float x;                                 /**< x coordinate, in metres */
    float y;                                 /**< y coordinate, in metres */
    float z;                                 /**< z coordinate, in metres */
    int bFlipX;                              /**< flag to flip the sign of the x coordinate */
    int bFlipY;                              /**< flag to flip the sign of the y coordinate */
    int bFlipZ;                              /**< flag to flip the sign of the z coordinate */
  
} interface_data;


/* ========================================================================== */
/*                             Internal Functions                             */
/* ========================================================================== */
  
/**
 * Sets core status.
 *
 * @param[in] hInt      interface handle
 * @param[in] newStatus Core status (see #INTERFACE_CORE_STATUS enum)
 */
void interface_setCoreStatus(void* const hInt,
                             INTERFACE_CORE_STATUS newStatus);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __INTERFACE_INTERNAL_H_INCLUDED__ */
