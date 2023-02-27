/**
 * @file interface.h
 * @brief interface to the core
 *
 * Mostly copied from the interface written for the saf_hades module found here:
 * https://github.com/jananifernandez/HADES
 * Copyright (c) 2021 - Janani Fernandez & Leo McCormack,  (GPLv2 License)
 *
 * @author Leo McCormack
 * @date 10th August 2022
 */

#ifndef __INTERFACE_H_INCLUDED__
#define __INTERFACE_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ========================================================================== */
/*                 INTERFACE Renderer Configurations Options                  */
/* ========================================================================== */

/** Maximum distance the user can move from the capture point */
#define INTERFACE_PERIMETER_DISTANCE_M ( 4.0f )

/** Distance map options */
typedef enum {
    INTERFACE_DISTANCE_MAP_USE_PARAM = 1, /**< (Default) use sph projection */
    INTERFACE_DISTANCE_MAP_1SRC,          /**< 1 source scenario for LT */
    INTERFACE_DISTANCE_MAP_2SRC,          /**< 2 source scenario for LT */
    INTERFACE_DISTANCE_MAP_3SRC           /**< 3 source scenario for LT */
    
}INTERFACE_DISTANCE_MAPS;
#define INTERFACE_NUM_DISTANCE_MAPS ( 4 )

/** Available degrees-of-freedom options the core can be configured for */
typedef enum {
    CORE_0DOF = 1,          /**< Fixed-head rendering */
    CORE_1DOF_ROTATIONS,    /**< Only z-axis (yaw) head-rotations permitted */
    CORE_3DOF_ROTATIONS,    /**< Only yaw-pitch-roll head-rotations permitted */
    CORE_3DOF_TRANSLATIONS, /**< Only x,y,z translations permitted */
    CORE_6DOF               /**< Both yaw-pitch-roll head-rotations and x,y,z
                             *   translations permitted */
} INTERFACE_DOF_OPTIONS;

/**
 * Current status of the core
 *
 * These can be used to find out whether the core is initialised, currently
 * in the process of intialising, or it is not yet initialised.
 */
typedef enum {
    CORE_STATUS_INITIALISED = 0, /**< Core is initialised and ready to process
                                  *   input audio. */
    CORE_STATUS_NOT_INITIALISED, /**< Core has not yet been initialised, or the
                                  *   core configuration has changed.
                                  *   Input audio should not be processed. */
    CORE_STATUS_INITIALISING     /**< Core is currently being initialised,
                                  *   input audio should not be processed. */
} INTERFACE_CORE_STATUS;

/** Length of progress bar string */
#define INTERFACE_PROGRESSBARTEXT_CHAR_LENGTH ( 256 )

/** Maximum number of input/output channels supported */
#define INTERFACE_MAX_NUM_CHANNELS ( 64 )

/** Maximum number of input channels supported */
#define INTERFACE_MAX_NUM_INPUTS ( INTERFACE_MAX_NUM_CHANNELS )

/** Maximum number of output channels supported */
#define INTERFACE_MAX_NUM_OUTPUTS ( INTERFACE_MAX_NUM_CHANNELS )


/* ========================================================================== */
/*                               Main Functions                               */
/* ========================================================================== */

/**
 * Creates an instance of the mighty interface
 *
 * @param[in] phInt (&) address of interface handle
 */
void interface_create(void** const phInt);

/**
 * Destroys an instance of the mighty interface
 *
 * @param[in] phInt (&) address of interface handle
 */
void interface_destroy(void** const phInt);

/**
 * Initialises an instance of interface
 *
 * @param[in] hInt       interface handle
 * @param[in] samplerate Host samplerate.
 */
void interface_init(void* const hInt,
                    int samplerate);
    
/**
 * Intialises the core based on current global/user parameters
 *
 * @param[in] hInt interface handle
 */
void interface_initCore(void* const hInt);

/**
 * Performs the processing
 *
 * @param[in] hInt     interface handle
 * @param[in] inputs   Input channel buffers; 2-D array: nInputs x nSamples
 * @param[in] outputs  Output channel buffers; 2-D array: nOutputs x nSamples
 * @param[in] nInputs  Number of input channels
 * @param[in] nOutputs Number of output channels
 * @param[in] nSamples Number of samples in 'inputs'/'output' matrices
 */
void interface_process(void* const hInt,
                       float** const inputs,
                       float** const outputs,
                       int nInputs,
                       int nOutputs,
                       int nSamples);


/* ========================================================================== */
/*                                Set Functions                               */
/* ========================================================================== */

/**
 * Sets all intialisation flags to 1; i.e. re-initialise all settings/variables
 * as interface is currently configured, at next available opportunity.
 */
void interface_refreshSettings(void* const hInt);

void interface_setFavour2Daccuracy(void* const hInt, int newState);
void interface_setEnableEPbeamformers(void* const hInt, int newState);
void interface_setEnableDiffEQ_HRTFs(void* const hInt, int newState);
void interface_setEnableDiffEQ_ATFs(void* const hInt, int newState);

/** See #INTERFACE_DOF_OPTIONS */
void interface_setDOFoption(void* const hInt, INTERFACE_DOF_OPTIONS newOption);

/** Sets the analysis averaging coefficient, [0..1] */
void interface_setAnalysisAveraging(void* const hInt,
                                    float newValue);

/** Sets the synthesis averaging coefficient, [0..1] */
void interface_setSynthesisAveraging(void* const hInt,
                                     float newValue);

/** Sets the maximum analysis frequency (Hz) */
void interface_setMaximumAnalysisFreq(void* const hInt,
                                      float newValue);

/** Sets the maximum BSM frequency (Hz), above which we use PWD */
void interface_setMaximumBSMFreq(void* const hInt,
                                 float newValue);

/** Sets the maximum MagLS frequency (Hz) */
void interface_setMaximumMagLSFreq(void* const hInt,
                                   float newValue);

/** Sets the linear to parametric balance */
void interface_setLinear2ParametricBalance(void* const hInt,
                                           float newValue);

/**
 * Copies the stream balance values from local, to the internal compass config
 */
void interface_setStreamBalanceFromLocal(void* const hInt);

/**
 * Sets the balance between direct and ambient streams (default=1, 50%/50%) for
 * ONE specific frequency band.
 *
 * @param[in] hInt     interface handle
 * @param[in] newValue New balance, 0: fully ambient, 1: balanced, 2: fully
 *                     direct
 * @param[in] bandIdx  Frequency band index
 */
void interface_setStreamBalance(void* const hInt,
                                float newValue,
                                int bandIdx);

/**
 * Sets the balance between direct and ambient streams (default=1, 50%/50%) for
 * ALL frequency bands.
 *
 * @param[in] hInt     interface handle
 * @param[in] newValue new balance, 0: fully ambient, 1: balanced, 2: fully
 *                     direct
 */
void interface_setStreamBalanceAllBands(void* const hInt,
                                        float newValue);

/**
 * Sets the file path for a .sofa file
 *
 * @param[in] hInt       interface handle
 * @param[in] path       File path to .sofa file (WITH file extension)
 */
void interface_setSofaFilePathMAIR(void* const hInt, const char* path);

/**
 * Sets flag to dictate whether the default HRIRs in the Spatial_Audio_Framework
 * should be used, or a custom HRIR set loaded via a SOFA file.
 * Note: if the custom set failes to load correctly, interface will revert to the
 * defualt set. Use interface_getUseDefaultHRIRsflag() to check if loading was
 * successful.
 *
 * @param[in] hInt       interface handle
 * @param[in] newState   0: use custom HRIR set, 1: use default HRIR set
 */
void interface_setUseDefaultHRIRsflag(void* const hInt, int newState);

/**
 * Sets the file path for a .sofa file
 *
 * @param[in] hInt       interface handle
 * @param[in] path       File path to .sofa file (WITH file extension)
 */
void interface_setSofaFilePathHRIR(void* const hInt, const char* path);

/** Sets the 'yaw' rotation angle, in degrees */
void interface_setYaw(void* const hInt, float newYaw_deg);

/** Sets the 'pitch' rotation angle, in degrees */
void interface_setPitch(void* const hInt, float newPitch);

/** Sets the 'roll' rotation angle, in degrees */
void interface_setRoll(void* const hInt, float newRoll);

/** Sets a flag as to whether to "flip" the sign of the current 'yaw' angle */
void interface_setFlipYaw(void* const hInt, int newState);

/** Sets a flag as to whether to "flip" the sign of the current 'pitch' angle */
void interface_setFlipPitch(void* const hInt, int newState);

/** Sets a flag as to whether to "flip" the sign of the current 'roll' angle */
void interface_setFlipRoll(void* const hInt, int newState);

/** See #INTERFACE_DISTANCE_MAPS */
void interface_setDistanceMapMode(void* const hInt, INTERFACE_DISTANCE_MAPS newMap);

/** Sets the source distance (from the origin), in metres */
void interface_setSourceDistance(void* const hInt, float newValue);

/** Sets the perimeter distance (from the origin), in metres */
void interface_setPerimeterDistance(void* const hInt, float newValue);

/** Sets the source directivity flag */
void interface_setEnableSourceDirectivity(void* const hInt, int newState);

/** Sets the listener position x coordinate (relative to origin), in metres */
void interface_setX(void* const hInt, float newX);

/** Sets the listener position y coordinate (relative to origin), in metres */
void interface_setY(void* const hInt, float newY);

/** Sets the listener position z coordinate (relative to origin), in metres */
void interface_setZ(void* const hInt, float newZ);

/** Sets a flag as to whether to "flip" the sign of the current x coordinate */
void interface_setFlipX(void* const hInt, int newState);

/** Sets a flag as to whether to "flip" the sign of the current y coordinate */
void interface_setFlipY(void* const hInt, int newState);

/** Sets a flag as to whether to "flip" the sign of the current z coordinate */
void interface_setFlipZ(void* const hInt, int newState);


/* ========================================================================== */
/*                                Get Functions                               */
/* ========================================================================== */

/**
 * Returns the processing framesize (i.e., number of samples processed with
 * every _process() call )
 */
int interface_getFrameSize(void);

/**
 * Returns current core status (see #INTERFACE_CORE_STATUS enum)
 */
INTERFACE_CORE_STATUS interface_getCoreStatus(void* const hInt);

/**
 * Returns current intialisation/processing progress, between 0..1
 *  - 0: intialisation/processing has started
 *  - 1: intialisation/processing has ended
 */
float interface_getProgressBar0_1(void* const hInt);

/**
 * Returns current intialisation/processing progress text
 *
 * @warning "text" string should be (at least) of length:
 *          #INTERFACE_PROGRESSBARTEXT_CHAR_LENGTH
 *
 * @param[in]  hInt interface handle
 * @param[out] text Process bar text; #INTERFACE_PROGRESSBARTEXT_CHAR_LENGTH x 1
 */
void interface_getProgressBarText(void* const hInt, char* text);

int interface_getFavour2Daccuracy(void* const hInt);
int interface_getEnableEPbeamformers(void* const hInt);
int interface_getEnableDiffEQ_HRTFs(void* const hInt);
int interface_getEnableDiffEQ_ATFs(void* const hInt);

/** Returns current DoF option (see #INTERFACE_DOF_OPTIONS) */
INTERFACE_DOF_OPTIONS interface_getDOFoption(void* const hInt);
 
/** Returns the analysis averaging coefficient, [0..1] */
float interface_getAnalysisAveraging(void* const hInt);

/** Returns the synthesis averaging coefficient, [0..1] */
float interface_getSynthesisAveraging(void* const hInt);

/** Returns the maximum spatial analysis frequency in Hz */
float interface_getMaximumAnalysisFreq(void* const hInt);
 
/** Returns the maximum BSM frequency in Hz */
float interface_getMaximumBSMFreq(void* const hInt);

/** Returns the maximum BSMMagLSfrequency in Hz */
float interface_getMaximumMagLSFreq(void* const hInt);

/** Returns the linear to parametric balance */
float interface_getLinear2ParametricBalance(void* const hInt);

/**
 * Returns the balance between direct and ambient streams (default=1, 50%/50%)
 * for ONE specific frequency band
 *
 * @param[in] hInt    interface handle
 * @param[in] bandIdx Frequency band index
 * @returns The current balance value, 0: fully ambient, 1: balanced, 2: fully
 *          direct
 */
float interface_getStreamBalance(void* const hInt,
                                 int bandIdx);
    
/**
 * Returns the balance between direct and ambient streams (default=1, 50%/50%)
 * for the FIRST frequency band
 *
 * @param[in] hInt interface handle
 * @returns Current balance, 0: fully ambient, 1: balanced, 2: fully direct
 */
float interface_getStreamBalanceAllBands(void* const hInt); /* returns the first value */

/**
 * Returns pointers for the balance between direct and ambient streams
 * (default=1, 50%/50%) for ALL frequency bands.
 *
 * @param[in]  hInt      interface handle
 * @param[out] pX_vector (&) Frequency vector; pNpoints x 1
 * @param[out] pY_values (&) Balance values per frequency; pNpoints x 1
 * @param[out] pNpoints  (&) Number of frequencies/balance values
 */
void interface_getStreamBalanceLocalPtrs(void* const hInt,
                                         float** pX_vector,
                                         float** pY_values,
                                         int* pNpoints);

/**
 * Returns pointers to the histogram
 *
 * @param[in]  hInt      interface handle
 * @param[out] pY_values (&) histogram; pNpoints x 1
 * @param[out] pNpoints  (&) Number of directions
 */
void interface_getHistogramLocalPtrs(void* const hInt,
                                     float** pY_values,
                                     int* pNpoints);

/**
 * Returns pointers to the grid directions (Cartesian coordinates - unit length)
 *
 * @param[in]  hInt      interface handle
 * @param[out] pY_values (&) grid directions; pNpoints x 3
 * @param[out] pNpoints  (&) Number of directions
 */
void interface_getGridDirectionsXYZLocalPtrs(void* const hInt,
                                             float** pY_values,
                                             int* pNpoints);

/**
 * Returns pointer to the head orientation (Cartesian coordinates - unit length)
 *
 * @param[in] hInt interface handle
 * @returns Pointer to head orientation; 3 x 1
 */
float* interface_getHeadOrientationPtr(void* const hInt);

/** Returns the number of frequency bands employed by core */
int interface_getNumberOfBands(void* const hInt);

/** Returns the number of microphones in the currently used array */
int interface_getNmicsArray(void* const hInt);

/** Returns the number of directions in the currently used array IR set */
int interface_getNDirsArray(void* const hInt);

/** Returns the length of the array IRs, in samples */
int interface_getIRlengthArray(void* const hInt);

/** Returns the array IR sample rate */
int interface_getIRsamplerateArray(void* const hInt);

/** Returns the number of directions in the currently used HRIR set */
int interface_getNDirsBin(void* const hInt);

/** Returns the length of the HRIRs, in samples */
int interface_getIRlengthBin(void* const hInt);

/** Returns the HRIR sample rate */
int interface_getIRsamplerateBin(void* const hInt);

/** Returns the DAW/Host sample rate */
int interface_getDAWsamplerate(void* const hInt);

/**
 * Returns the file path for a .sofa file
 *
 * @param[in] hInt interface handle
 * @returns File path to .sofa file (WITH file extension)
 */
char* interface_getSofaFilePathMAIR(void* const hInt);

/**
 * Returns the value of a flag used to dictate whether the default HRIRs in the
 * Spatial_Audio_Framework should be used, or a custom HRIR set loaded via a
 * SOFA file
 *
 * @param[in] hInt interface handle
 * @returns 0: use custom HRIR set, 1: use default HRIR set
 */
int interface_getUseDefaultHRIRsflag(void* const hInt);

/**
 * Returns the file path for a .sofa file
 *
 * @param[in] hInt interface handle
 * @returns File path to .sofa file (WITH file extension)
 */
char* interface_getSofaFilePathHRIR(void* const hInt);

/** Returns the 'yaw' rotation angle, in degrees */
float interface_getYaw(void* const hInt);

/** Returns the 'pitch' rotation angle, in degrees */
float interface_getPitch(void* const hInt);

/** Returns the 'roll' rotation angle, in degrees */
float interface_getRoll(void* const hInt);

/**
 * Returns a flag as to whether to "flip" the sign of the current 'yaw' angle
 * ('0' do not flip sign, '1' flip the sign)
 */
int interface_getFlipYaw(void* const hInt);

/**
 * Returns a flag as to whether to "flip" the sign of the current 'pitch' angle
 * ('0' do not flip sign, '1' flip the sign)
 */
int interface_getFlipPitch(void* const hInt);

/**
 * Returns a flag as to whether to "flip" the sign of the current 'roll' angle
 * ('0' do not flip sign, '1' flip the sign)
 */
int interface_getFlipRoll(void* const hInt);

/** See #INTERFACE_DISTANCE_MAPS */
INTERFACE_DISTANCE_MAPS interface_getDistanceMapMode(void* const hInt);

/** Returns the source distance (from the origin), in metres */
float interface_getSourceDistance(void* const hInt);

/** Returns the perimeter distance (from the origin), in metres */
float interface_getPerimeterDistance(void* const hInt);

/** Returns the source directivity flag */
int interface_getEnableSourceDirectivity(void* const hInt);

/** Returns the listener position x coordinate (relative to origin), in metres */
float interface_getX(void* const hInt);

/** Returns the listener position y coordinate (relative to origin), in metres */
float interface_getY(void* const hInt);

/** Returns the listener position z coordinate (relative to origin), in metres */
float interface_getZ(void* const hInt);

/** Returns a flag as to whether to "flip" the sign of the current x coordinate */
int interface_getFlipX(void* const hInt);

/** Returns a flag as to whether to "flip" the sign of the current y coordinate */
int interface_getFlipY(void* const hInt);

/** Returns a flag as to whether to "flip" the sign of the current z coordinate */
int interface_getFlipZ(void* const hInt);

/**
 * Returns the processing delay in samples (may be used for delay compensation
 * features)
 */
int interface_getProcessingDelay(void* const hInt);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __INTERFACE_H_INCLUDED__ */
