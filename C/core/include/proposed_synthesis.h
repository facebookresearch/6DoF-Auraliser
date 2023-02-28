/**
 * @file proposed_synthesis.h
 * @ingroup PROPOSED
 * @brief Header for the proposed synthesis
 *
 * This code is derived and built upon the saf_hades module (GPLv2 License)
 * found in the the Spatial_Audio_Framework:
 * https://github.com/leomccormack/Spatial_Audio_Framework/tree/master/framework/modules/saf_hades
 * Copyright (c) 2021 - Leo McCormack & Janani Fernandez
 *
 * The parametric rendering algorithms are based on similar time-frequency
 * domain acoustic scene analysis as used in e.g. [1-4]. This information is
 * then used to steer beamformers, which are then spatialised in the estimated
 * directions.
 * The residual stream then forgoes decorrelation (although this would be a nice
 * future addition), and employs either a least-squares [5] or MagLS solution
 * [6] for its rendering.
 *
 * @see [1] Politis, A., Tervo, S. and Pulkki, V., 2018, April. COMPASS: Coding
 *          and multidirectional parameterization of ambisonic sound scenes.
 *          In 2018 IEEE International Conference on Acoustics, Speech and
 *          Signal Processing (ICASSP) (pp. 6802-6806). IEEE.
 * @see [2] McCormack, L., Politis, A., Gonzalez, R., Lokki, T., and Pulkki,
 *          V., 2022. Parametric Ambisonic Encoding of Arbitrary Microphone
 *          Arrays, IEEE/ACM Transactions on Audio, Speech, and Language
 *          Processing 30, 2062-2075 https://doi.org/10.1109/TASLP.2022.3182857
 * @see [3] Fernandez, J., McCormack, L., Hyvärinen, P., Politis, A., and
 *          Pulkki V. 2022. "Enhancing binaural rendering of head-worn
 *          microphone arrays through the use of adaptive spatial covariance
 *          matching", The Journal of the Acoustical Society of America 151(4),
 *          2624-2635 https://doi.org/10.1121/10.0010109
 * @see [4] McCormack, L., Politis, A., and Pulkki, V., 2021, September.
 *          Parametric Spatial Audio Effects Based on the Multi-Directional
 *          Decomposition of Ambisonic Sound Scenes. In Proceedings of the 24th
 *          International Conference on Digital Audio Effects (DAFx20in21),
 *          (pp. 214-221).
 * @see [5] Madmoni, L., Donley, J., Tourbabin, V. and Rafaely, B., 2020,
 *          August. Beamforming-based binaural reproduction by matching of
 *          binaural signals. In Audio Engineering Society Conference: 2020 AES
 *          International Conference on Audio for Virtual and Augmented Reality.
 *          Audio Engineering Society.
 * @see [6] Deppisch, T., Helmholz, H. and Ahrens, J., 2021, September.
 *          End-to-end magnitude least squares binaural rendering of spherical
 *          microphone array signals. In 2021 Immersive and 3D Audio: from
 *          Architecture to Automotive (I3DA) (pp. 1-7). IEEE.
 *
 * @author Leo McCormack
 * @date 9th August 2022
 *
 * Copyright (c) Meta Platforms, Inc. All Rights Reserved
 */

#ifndef __PROPOSED_SYNTHESIS_H_INCLUDED__
#define __PROPOSED_SYNTHESIS_H_INCLUDED__

#include "proposed_analysis.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Handle for the proposed synthesis data */
typedef struct _proposed_synthesis_data* proposed_synthesis_handle;

/** Handle for the proposed translator data */
typedef struct _proposed_translator_data* proposed_translator_handle;

/* ========================================================================== */
/*                 PROPOSED Synthesis Configurations Options                  */
/* ========================================================================== */

/** Binaural configuration struct */
typedef struct _proposed_binaural_config{
    int lHRIR;                  /**< Length of HRIRs in samples */
    int nHRIR;                  /**< Number of HRIRs */
    int hrir_fs;                /**< HRIR sample rate */
    float* hrirs;               /**< Matrix of HRIR data;
                                 *   FLAT: nHRIR x #NUM_EARS x lHRIR */
    float* hrir_dirs_deg;       /**< HRTF directions in [azimuth elevation]
                                 *   format, in degrees; FLAT: nHRIR x 2 */
}proposed_binaural_config;

/** Distance map options for proposed_synthesis */
typedef enum {
    PROPOSED_DISTANCE_MAP_USE_PARAM,  /**< (Default) use spherical projection */
    PROPOSED_DISTANCE_MAP_1SRC,       /**< 1 source scenario for LT */
    PROPOSED_DISTANCE_MAP_2SRC,       /**< 2 source scenario for LT */
    PROPOSED_DISTANCE_MAP_3SRC        /**< 3 source scenario for LT */
}PROPOSED_DISTANCE_MAPS;

/** HRTF interpolation options for proposed_synthesis */
typedef enum {
    PROPOSED_HRTF_INTERP_NEAREST,    /**< Quantise to nearest measurement */
    PROPOSED_HRTF_INTERP_TRIANGULAR  /**< Triangular interpolation */
}PROPOSED_HRTF_INTERP_OPTIONS;


/* ========================================================================== */
/*                            PROPOSED Synthesis                              */
/* ========================================================================== */

/**
 * Creates and returns a handle to an instance of a proposed synthesis object
 *
 * @param[in] phSyn        (&) address of proposed synthesis handle
 * @param[in] hAna         proposed analysis handle
 * @param[in] binConfig    Binaural configuration
 * @param[in] interpOption see #PROPOSED_HRTF_INTERP_OPTIONS
 */
void proposed_synthesis_create(/* Input Arguments */
                               proposed_synthesis_handle* const phSyn,
                               proposed_analysis_handle const hAna,
                               proposed_binaural_config* binConfig,
                               PROPOSED_HRTF_INTERP_OPTIONS interpOption,
                               int favour2Daccuracy,
                               int enableEPbeamformers,
                               int enableDiffEQ_HRTFs,
                               int enableDiffEQ_ATFs);

/**
 * Destroys an instance of proposed synthesis
 *
 * @param[in] phSyn (&) address of proposed synthesis handle
 */
void proposed_synthesis_destroy(/* Input Arguments */
                                proposed_synthesis_handle* const phSyn);

/**
 * Flushes run-time buffers with zeros
 *
 * Call this ONCE before calling proposed_synthesis_apply()
 *
 * @param[in] hSyn proposed synthesis handle
 */
void proposed_synthesis_reset(proposed_synthesis_handle const hSyn);

/**
 * Performs proposed synthesis
 *
 * @note If nChannels is higher than the number required by the configuration,
 *       then these extra channels are zero'd. If there are too few, then
 *       the channels are truncated.
 *
 * @param[in]  hSyn       proposed synthesis handle
 * @param[in]  hPCon      proposed parameter container handle
 * @param[in]  hSCon      proposed signal container handle
 * @param[in]  ypr_rad    Yaw-Pitch-Roll rotation angles (in radians); 3 x 1
 * @param[in]  xyz_m      Listener position w.r.t to origin (in metres); 3 x 1
 * @param[in]  src_dist_m Assumed source distance in metres
 * @param[in]  enableSrcD Flag, 1: enable source directivity modelling, 0: nope
 * @param[in]  nChannels  Number of channels in output buffer
 * @param[in]  blocksize  Number of samples in output buffer
 * @param[out] output     Output buffer; nChannels x blocksize
 */
void proposed_synthesis_apply(/* Input Arguments */
                              proposed_synthesis_handle const hSyn,
                              proposed_param_container_handle  const hPCon,
                              proposed_signal_container_handle const hSCon,
                              float* ypr_rad,
                              float* xyz_m,
                              PROPOSED_DISTANCE_MAPS dist_map,
                              float src_dist_m,
                              int enableSrcD,
                              int nChannels,
                              int blocksize,
                              /* Output Arguments */
                              float** output);

/**
 * Returns a pointer to the eq vector, which can be changed at run-time
 *
 * @param[in]  hSyn   proposed synthesis handle
 * @param[out] nBands (&) Number of bands (set to NULL if not needed)
 * @returns pointer to the eq vector (or NULL if hSyn is not initialised);
 *          nBands x 1
 */
float* proposed_synthesis_getEqPtr(proposed_synthesis_handle const hSyn,
                                   int* nBands);

/**
 * Returns a pointer to the stream balance vector [0..2], which can be changed
 * at run-time
 *
 * @param[in]  hSyn   proposed synthesis handle
 * @param[out] nBands (&) Number of bands (set to NULL if not needed)
 * @returns pointer to the stream balance vector (or NULL if hSyn is not
 *          initialised); nBands x 1
 */
float* proposed_synthesis_getStreamBalancePtr(proposed_synthesis_handle const hSyn,
                                              int* nBands);

/**
 * Returns a pointer to the synthesis averaging coefficient scalar [0..1], which
 * can be changed at run-time
 *
 * @param[in]  hSyn   proposed synthesis handle
 * @returns pointer to the mixing matrix averaging coeff scalar (or NULL if hSyn
 *          is not initialised); 1 x 1
 */
float* proposed_synthesis_getSynthesisAveragingCoeffPtr(proposed_synthesis_handle const hSyn);

/**
 * Returns a pointer to the maximum BSM frequency in Hz, which can be changed
 * at run-time
 *
 * @param[in] hSyn proposed synthesis handle
 * @returns pointer to the maximum BSM frequency in Hz (or NULL if hAna is not
 *          initialised); 1 x 1
 */
float* proposed_synthesis_getMaxBSMFreqPtr(proposed_synthesis_handle const hSyn);

/**
 * Returns a pointer to the maximum MagLS frequency in Hz, which can be changed
 * at run-time
 *
 * @param[in] hSyn proposed synthesis handle
 * @returns pointer to the maximum MagLS frequency in Hz (or NULL if hAna is not
 *          initialised); 1 x 1
 */
float* proposed_synthesis_getMaxMagLSFreqPtr(proposed_synthesis_handle const hSyn);

/**
 * Returns a pointer to the linear to parametric balance level
 *
 * @param[in] hSyn proposed synthesis handle
 * @returns pointer to the maximum linear2parametric bal (or NULL if hAna is not
 *          initialised); 1 x 1
 */
float* proposed_synthesis_getLinear2ParametricBalancePtr(proposed_synthesis_handle const hSyn);
 
/**
 * Returns the synthesiser processing delay, in samples
 *
 * @note This is not inclusive of the time-frequency transform delay, as you
 *       may get this using proposed_analysis_getProcDelay(). The total delay is:
 *       proposed_analysis_getProcDelay() + proposed_synthesis_getProcDelay().
 */
int proposed_synthesis_getProcDelay(proposed_synthesis_handle const hSyn);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __PROPOSED_SYNTHESIS_H_INCLUDED__ */
