/**
 * @file proposed_analysis.h
 * @ingroup PROPOSED
 * @brief Header for the proposed analysis
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
 * Copyright (c) Meta Platforms, Inc. All Rights Reserved*
 */

#ifndef __PROPOSED_ANALYSIS_H_INCLUDED__
#define __PROPOSED_ANALYSIS_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Maximum number of microphones */
#define PROPOSED_MAX_NMICS ( 64 )

/** Handle for the proposed analysis data */
typedef struct _proposed_analysis_data* proposed_analysis_handle;

/** Handle for the proposed parameter container data */
typedef struct _proposed_param_container_data* proposed_param_container_handle;

/** Handle for the proposed signal container data */
typedef struct _proposed_signal_container_data* proposed_signal_container_handle;

/* ========================================================================== */
/*                            PROPOSED Analysis                               */
/* ========================================================================== */

/**
 * Creates and returns a handle to an instance of a proposed analysis object
 *
 * @param[in] phAna          (&) address of proposed analysis handle
 * @param[in] fs             Samplerate, Hz
 * @param[in] hopsize        Filterbank hopsize
 * @param[in] blocksize      Number of time-domain samples to process at a time
 * @param[in] h_array        ATF responses; FLAT: nDirs x nMics x h_len
 * @param[in] array_dirs_deg ATF dirs [azi elev] in degrees; FLAT: nDirs x 2
 * @param[in] nDirs          Number of measurement directions
 * @param[in] nMics          Number of microphones
 * @param[in] h_len          Length of impulse responses, in samples
 */
void proposed_analysis_create(/* Input Arguments */
                              proposed_analysis_handle* const phAna,
                              float fs,
                              int hopsize,
                              int blocksize,
                              float* h_array,
                              float* array_dirs_deg,
                              int nDirs,
                              int nMics,
                              int h_len);

/**
 * Destroys an instance of a proposed analysis object
 *
 * @param[in] phAna (&) address of proposed analysis handle
 */
void proposed_analysis_destroy(/* Input Arguments */
                               proposed_analysis_handle* const phAna);

/**
 * Flushes run-time buffers with zeros
 *
 * Call this ONCE before calling proposed_analysis_apply()
 *
 * @param[in] hAna proposed analysis handle
 */
void proposed_analysis_reset(proposed_analysis_handle const hAna);

/**
 * Performs proposed analysis
 *
 * @note See proposed_param_container_create() and
 *       proposed_signal_container_create() for creating the parameter and signal
 *       containers, respectively. The former contains the estimated spatial
 *       parameters (a diffuseness measure, DoA for each source), while
 *       the latter contains the input signals in the time-frequency domain,
 *       and their spatial covariance matrices per band. These containers can
 *       then be passed to proposed_synthesis_apply() to reproduce the encoded
 *       scene over the target setup.
 *
 * @param[in]  hAna      proposed analysis handle
 * @param[in]  input     Input buffer; nChannels x blocksize
 * @param[in]  nChannels Number of channels in input buffer
 * @param[in]  blocksize Number of samples in input buffer
 * @param[out] hPCon     proposed parameter container handle
 * @param[out] hSCon     proposed signal container handle
 */
void proposed_analysis_apply(/* Input Arguments */
                             proposed_analysis_handle const hAna,
                             float** input,
                             int nChannels,
                             int blocksize,
                             /* Output Arguments */
                             void* const hPCon,
                             void* const hSCon);

/**
 * Returns a pointer to the frequency vector (read-only)
 *
 * @param[in]  hAna   proposed analysis handle
 * @param[out] nBands (&) Number of bands (set to NULL if not needed)
 * @returns pointer to freqVector (or NULL if hAna is not initialised);
 *          nBands x 1
 */
const float* proposed_analysis_getFrequencyVectorPtr(/* Input Arguments */
                                                     proposed_analysis_handle const hAna,
                                                     /* Output Arguments */
                                                     int* nBands);

/**
 * Returns a pointer to the grid directions (in Cartesian coords) (read-only)
 *
 * @param[in]  hAna  proposed analysis handle
 * @param[out] nDirs (&) Number of directions (set to NULL if not needed)
 * @returns pointer to grid directions (or NULL if hAna is not initialised);
 *          FLAT: nDirs x 3
 */
const float* proposed_analysis_getGridDirsXYZPtr(/* Input Arguments */
                                                 proposed_analysis_handle const hAna,
                                                 /* Output Arguments */
                                                 int* nDirs);

/**
 * Returns a pointer to the histogram vector (read-only)
 *
 * @param[in]  hAna  proposed analysis handle
 * @param[out] nDirs (&) Number of directions (set to NULL if not needed)
 * @returns pointer to histogram data (or NULL if hAna is not initialised);
 *          nDirs x 1
 */
const float* proposed_analysis_getHistogramPtr(/* Input Arguments */
                                               proposed_analysis_handle const hAna,
                                               /* Output Arguments */
                                               int* nDirs);

/** Returns number of frequency bands (or 0 if hAna is not initialised) */
int proposed_analysis_getNbands(proposed_analysis_handle const hAna);

/** Returns number of grid directions (or 0 if hAna is not initialised) */
int proposed_analysis_getNDirs(proposed_analysis_handle const hAna);

/**
 * Returns a pointer to the covariance matrix averaging scalar [0..1], which can
 * be changed at run-time
 *
 * @param[in] hAna proposed analysis handle
 * @returns pointer to the covariance matrix averaging scalar (or NULL if hAna
 *          is not initialised); 1 x 1
 */
float* proposed_analysis_getCovarianceAvagingCoeffPtr(proposed_analysis_handle const hAna);

/**
 * Returns a pointer to the maximum analysis frequency in Hz, which can be
 * changed at run-time
 *
 * @param[in] hAna proposed analysis handle
 * @returns pointer to the maximum analysis frequency in Hz (or NULL if hAna
 *          is not initialised); 1 x 1
 */
float* proposed_analysis_getMaxAnalysisFreqPtr(proposed_analysis_handle const hAna);

/**
 * Returns the analyser processing delay, in samples
 *
 * @note The total delay for an analyser -> synthesiser configuration is
 *       computed as:
 *           proposed_analysis_getProcDelay() + proposed_synthesis_getProcDelay().
 */
int proposed_analysis_getProcDelay(proposed_analysis_handle const hAna);


/* ========================================================================== */
/*                      Parameter and Signal Containers                       */
/* ========================================================================== */

/**
 * Creates an instance of a container used for storing the parameters
 * estimated by an analyser for one 'blocksize'
 *
 * @note There should be one container per analyser, but this container can be
 *       passed to multiple different synthesisers. You may also create multiple
 *       containers, fill them using an analyser, store them, and pass them to
 *       the synthesiser(s) later.
 *
 * @param[in] phPCon (&) address of proposed parameter container handle
 * @param[in] hAna   proposed analysis handle
 */
void proposed_param_container_create(/* Input Arguments */
                                     proposed_param_container_handle* const phPCon,
                                     proposed_analysis_handle const hAna);

/**
 * Destroys an instance of a proposed parameter container
 *
 * @param[in] phPCon (&) address of proposed parameter container handle
 */
void proposed_param_container_destroy(/* Input Arguments */
                                      proposed_param_container_handle* const phPCon);

/**
 * Creates an instance of a container used for storing the TF-domain audio
 * returned by an analyser for one 'blocksize'
 *
 * @note There should be one container per analyser, but this container can be
 *       passed to multiple different synthesisers. You may also create multiple
 *       containers, fill them using an analyser, store them, and pass them to
 *       the synthesiser(s) later.
 *
 * @param[in] phSCon (&) address of proposed signal container handle
 * @param[in] hAna   proposed analysis handle
 */
void proposed_signal_container_create(proposed_signal_container_handle* const phSCon,
                                      proposed_analysis_handle const hAna);

/**
 * Destroys an instance of a proposed signal container
 *
 * @param[in] phSCon (&) address of proposed signal container handle
 */
void proposed_signal_container_destroy(/* Input Arguments */
                                       proposed_signal_container_handle* const phSCon);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __PROPOSED_ANALYSIS_H_INCLUDED__ */
