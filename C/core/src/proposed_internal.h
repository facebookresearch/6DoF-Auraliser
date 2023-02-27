/**
 * @file proposed_internal.h
 * @ingroup PROPOSED
 * @brief Internal header for the proposed method
 *
 * This code is derived and built upon the saf_hades module (GPLv2 License)
 * found in the the Spatial_Audio_Framework:
 * https://github.com/leomccormack/Spatial_Audio_Framework/tree/master/framework/modules/saf_hades
 * Copyright (c) 2021 - Leo McCormack & Janani Fernandez
 * Copyright (c) Meta Platforms, Inc. All Rights Reserved
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
 */

#ifndef __PROPOSED_INTERNAL_H_INCLUDED__
#define __PROPOSED_INTERNAL_H_INCLUDED__

#include "proposed_analysis.h"
#include "proposed_synthesis.h"
#include "saf.h"
#include "saf_externals.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Maximum supported blocksize */
#define PROPOSED_MAX_BLOCKSIZE ( 4096 )

/** Maximum supported number of simultaneous sources */
#define PROPOSED_MAX_K ( 3 )

/** Maximum rendering frequency in Hz */
#define PROPOSED_MAX_RENDERING_FREQ ( 16e3f )

/** FLAG: Whether to use MUSIC rather than PWD */
#define PROPOSED_USE_MUSIC ( 1 ) // MUST BE 1, PWD NOT IMPLEMENTED YET!

/** +/- elevation window when scanning for DoA, [10..90] */
#define PROPOSED_ELEV_SCANNING_WINDOW_DEG ( 20.0f )

/** Helper struct for averaging covariance matrices (block-wise) */
typedef struct _CxMic{
    float_complex Cx[PROPOSED_MAX_NMICS*PROPOSED_MAX_NMICS];
}CxMic;

/* ========================================================================== */
/*                           Main Internal Structs                            */
/* ========================================================================== */

/** Main structure for proposed analysis */
typedef struct _proposed_analysis_data
{
    /* User parameters (defined at intialisation stage) */
    float fs;                             /**< Host samplerate, Hz */
    int hopsize;                          /**< Filterbank hop size (blocksize must be divisable by this */
    int blocksize;                        /**< Number of samples to process at a time (note that 1 doa and diffuseness estimate is made per block) */
    float* h_array;                       /**< Array impulse responses; FLAT: nDirs x nMics x h_len */
    float* array_dirs_deg;                /**< Array grid dirs in degrees; FLAT: nDirs x 2 */
    float* array_dirs_xyz;                /**< Array grid coordinates (unit vectors and only used by grid-based estimators); FLAT: nDirs x 3 */
    int nDirs;                            /**< Number of ATFs/scanning directions */
    int nMics;                            /**< Number of microphones */
    int h_len;                            /**< Length of impulse responses, in samples */
      
    /* Optional user parameters (that can also be manipulated at run-time) */
    float covAvgCoeff;                    /**< Temporal averaging coefficient [0 1] */
    float maximumAnalysisFreq;            /**< Maximum analysis frequency in Hz */
    
    /* For optional plotting purposes  */
    float* grid_histogram;                /**< Histogram for the scanning directions; nDirs x 1 */

    /* Time-frequency transform and array data */
    void* hFB_enc;                        /**< Time-frequency transform handle */
    int nBands;                           /**< Number of frequency bands */
    int timeSlots;                        /**< Number of time slots */
    int filterbankDelay;                  /**< Filterbank delay, in time-domain samples */
    float* freqVector;                    /**< Centre frequencies; nBands x 1 */
    float_complex* DCM_array;             /**< Diffuse covariance matrix (computed over all grid directions and weighted); FLAT: nBands x nMics x nMics */
    float_complex* H_array;               /**< Array IRs in the frequency domain; FLAT: nBands x nMics x nDirs */
    float_complex* H_array_w;             /**< Array IRs in the frequency domain spatially weightend; FLAT: nBands x nMics x nDirs */

    /* DoA and diffuseness estimator data */
    void* hEig;                           /**< handle for the eigen solver */
    float_complex** T;                    /**< for covariance whitening; nBands x (nMics x nMics) */
    void* hDoA;                           /**< DoA estimator handle */
    int nScan;                            /**< Number of scanning directions */
    float* scan_dirs_deg;                 /**< Scanning grid dirs in degrees; FLAT: nScan x 2 */
    float* scan_dirs_xyz;                 /**< Scanning grid dirs in Cartesian coordinates; FLAT: nScan x 3 */
    float_complex* H_scan_w;              /**< Array IRs used for scanning, in the frequency domain; FLAT: nBands x nMics x nScan */
    int* scan_idx;                        /**< Scanning grid indices; nScan x 1 */
    float_complex* W;                     /**< Diffuse integration weighting matrix; FLAT: nDirs x nDirs */

    /* Run-time variables */
    float** inputBlock;                   /**< Input frame; nMics x blocksize */
    CxMic* Cx;                            /**< Current (time-averaged) covariance matrix per band; nBands x 1 */
    float_complex* V;                     /**< Eigen vectors; FLAT: nMics x nMics */
    float_complex* Vn;                    /**< Noise subspace; FLAT: nMics x (nMics-K) */
    float* lambda;                        /**< Eigenvalues; nMics x 1 */

}proposed_analysis_data;

/** Main structure for proposed synthesis */
typedef struct _proposed_synthesis_data
{
    /* User parameters */
    proposed_binaural_config* binConfig; /**< Internal copy of user configuration */
    PROPOSED_HRTF_INTERP_OPTIONS interpOption; /**< HRIR interpolation option, see #PROPOSED_HRTF_INTERP_OPTIONS */

    /* Optional user parameters (that can also be manipulated at run-time) */
    float* eq;                       /**< Gain factor per band; nBands x 1 */
    float* streamBalance;            /**< Stream balance per band (0:fully diffuse, 1:balanced, 2:fully direct); nBands x 1 */
    float synAvgCoeff;               /**< Mixing matrix averaging coefficent [0..1] */
    float maxBSMFreq;                /**< Frequency up to which to use BSM before switching to Ambisonics [0..fs/2] */
    float maxMagLSFreq;              /**< Frequency up to which to use MagLS optimisation */
    float linear2parBalance;         /**< Linear to parametric balance [0..1] */

    /* Things relevant to the synthesiser, which are copied from the proposed_analysis_create() to keep everything aligned */
    float fs;                        /**< Host samplerate, Hz */
    int nBands;                      /**< Number of bands in the time-frequency transform domain */
    int hopsize;                     /**< hopsize in samples */
    int blocksize;                   /**< blocksize in samples */
    int nDirs;                       /**< Number of measurement/scanning directions */
    int nMics;                       /**< Number of microphones */
    float_complex* H_array;          /**< Array IRs in the frequency domain; FLAT: nBands x nMics x nDirs */
    float* array_dirs_deg;           /**< Array measurement dirs in degrees; FLAT: nDirs x 2 */
    float** array_dirs_xyz;          /**< Array measurement dirs as Cartesian coordinates of unit length; nDirs x 3 */
    int timeSlots;                   /**< Number of time frames in the time-frequency transform domain */
    float* freqVector;               /**< Frequency vector (band centre frequencies); nBands x 1 */
    float_complex* DCM_array;        /**< Diffuse coherence matrix for the array; FLAT: nBands x nMics x nMics */
    float_complex* W;                /**< Diffuse integration weighting matrix; FLAT: nDirs x nDirs */

    /* Time-frequency transform */
    void* hFB_dec;                   /**< Filterbank handle */
    
    /* Direct-stream rendering data */
    float_complex* H_bin;            /**< To spatialise the source beamformers; FLAT: nBands x #NUM_EARS x nDirs */
    
    /* Ambient-rendering data */
    int nDiff;                       /**< Number of directions used for rendering */
    float* diff_dirs_xyz;            /**< Diffuse-grid dirs as Cartesian coordinates of unit length; FLAT: nDiff x 3 */
    int* diff_indices;               /**< Indices into measurement grid; nDiff x 1 */
    float_complex* H_array_diff;     /**< Rendering Array IRs in the frequency domain; FLAT: nBands x nMics x nDiff */
    float_complex* H_bin_diff;       /**< Rendering HRTFs (can include rotations in the case of BSM); FLAT: nBands x #NUM_EARS x nDiff */
    float* diffEQ;                   /**< diffuse-field EQ, as described in [2]; nBands x 1 */
    
    /* Ambient-rendering data - BSM */
    float_complex* M_BSM;            /**< FLAT: nBands x #NUM_EARS x nMics */
    float* diff_pos_xyz;             /**< Diff dirs as Cartesian coordinates of positions on the sphere; FLAT: nDiff x 3 */
    float* diff_dirs_xyz_new;        /**<; FLAT: nDiff x 3 */
    float* diff_dirs_xyz_rot;        /**< ROTATED Diffuse-grid dirs as Cartesian coordinates of unit length; FLAT: nDiff x 3 */
    float* diff_gains;               /**<  nDiff x 1 */
    void* hBSM;                      /**< Handle for BSM implementation */

    /* Linear 6DoF baseline */
    int nPWD;                        /**< Number of plane-waves in the decomposition */
    float* pwd_dirs_xyz;             /**< Plane-wave decomposition dirs as Cartesian coordinates of unit length; FLAT: nPWD x 3 */
    float* pwd_pos_xyz;              /**< Plane-wave decomposition dirs as Cartesian coordinates of positions on the sphere; FLAT: nPWD x 3 */
    float* pwd_dirs_xyz_rot;         /**< ROTATED Plane-wave decomposition dirs as Cartesian coordinates of unit length; FLAT: nPWD x 3 */
    int* pwd_indices;                /**< Indices into measurement grid; nPWD x 1 */
    float* pwd_gains;                /**< Gains to apply per plane-wave; nPWD x 1 */
    float_complex* M_PWD;            /**< Beamforming weights for the PW directions; FLAT: nBands x nPWD x nMics */
    float_complex* M_HRTFs;          /**< HRTFs taking into account head orientation and position, and incl. 1/R gains; FLAT: nBands x #NUM_EARS x nPWD */
    
    /* Run-time variables */
    void* hPinv;                     /**< Handle for computing the Moore-Penrose pseudo inverse */
    void* hLinSolve;                 /**< Handle for solving linear equations (Ax=b) */
    void* hInv;                      /**< Handle for matrix inversion */
    float_complex* As;               /**< Array steering vector for DoA; FLAT: nMics x #PROPOSED_MAX_K */
    float_complex* Ds;               /**< Source beamforming matrix; FLAT: #PROPOSED_MAX_K x nMics */
    float_complex* Dd;               /**< Source beamforming matrix; FLAT: nMics x nMics */
    float_complex* new_M_par;        /**< New mixing matrix, for parametric rendering; FLAT: #NUM_EARS x nMics */
    float_complex* new_M_lin;        /**< New mixing matrix, for linear rendering only; FLAT: #NUM_EARS x nMics */
    float_complex** M_par;           /**< Mixing matrix per band for the parametric rendering; nBands x FLAT: (#NUM_EARS x nMics) */
    float_complex** M;               /**< Mixing matrix per band; nBands x FLAT: (#NUM_EARS x nMics) */

    /* Run-time audio buffers */
    float_complex*** outTF;          /**< nBands x #NUM_EARS x timeSlots */
    float** outTD;                   /**< output time-domain buffer; #NUM_EARS x blocksize */
 
} proposed_synthesis_data;

/** Parameter container to store the data from an analyser for one blocksize of audio */
typedef struct _proposed_param_container_data {
    int nBands;                      /**< Number of bands */

    /* Estimated Parameters */
    int* nSrcs;                      /**< Number of sources per band; nBands x 1 */
    float* diffuseness;              /**< Diffuseness value per band; nBands x 1 */
    int** doa_idx;                   /**< Beamforming direction index per band; nBands x #PROPOSED_MAX_K */
    int** gains_idx;                 /**< Reproduction direction index per band; nBands x #PROPOSED_MAX_K */

    /* Optional parameters */
    float** src_gains;               /**< Extra direct reproduction gain per band (default=1.0f); nBands x PROPOSED_MAX_K  */

} proposed_param_container_data;

/** Main structure for proposed radial (360degree) gain and direct-to-diffuse ratio editor */
typedef struct _proposed_radial_editor_data {
    int nBands;                      /**< Number of bands */
    int nDirs;                       /**< Number of grid/scanning directions */
    float* pArray_dirs_deg;          /**< Pointer to grid dirs in degrees; FLAT: nDirs x 2 */
    float* pArray_dirs_xyz;          /**< Pointer to grid dirs as Cartesian coordinates of unit length; FLAT: nDirs x 3 */

} proposed_radial_editor_data;

/** Signal container to store one block of TF-domain audio data */
typedef struct _proposed_signal_container_data {
    int nMics;                       /**< Number of spherical harmonic components */
    int nBands;                      /**< Number of bands in the time-frequency transform */
    int timeSlots;                   /**< Number of time frames in time-frequency transform */

    /* Covariance matrices and signal statistics computed during the analysis */
    CxMic* Cx;                       /**< NON-time-averaged covariance matrix per band; nBands x .Cx(nMics x nMics) */

    /* TF frame to carry over to a decoder */
    float_complex*** inTF;           /**< Input frame in TF-domain; nBands x nMics x timeSlots */

} proposed_signal_container_data;


/* ========================================================================== */
/*                             Internal Functions                             */
/* ========================================================================== */

/**
 * Finds nearest grid indices
 */
void proposed_findNearestGridIndices(float* grid_dirs_xyz,
                                     float* target_dirs_xyz,
                                     int nDirs,
                                     int nTarget,
                                     int* indices);

/**
 * Creates an instance of the BSM implementation
 *
 * @param[in,out]  phA2B  pointer to handle
 * @param[in]      ATFs   array TFs; FLAT: nBands x nMics x nDirs
 * @param[in]      nBands Number of frequency bands
 * @param[in]      nMics  Number of microphones in array
 * @param[in]      nDirs  Number of measurement directions
 */
void proposed_array2binauralMagLS_create(/* Input Arguments */
                                         void** phA2B,
                                         float_complex* ATFs,
                                         int nBands,
                                         int nMics,
                                         int nDirs);

/**
 * Destroys an instance of the BSM implementation
 *
 * @param[in,out]  phA2B  pointer to handle
 */
void proposed_array2binauralMagLS_destroy(/* Input Arguments */
                                          void ** const phA2B);

/**
 * Computes array to binaural mixing matrices, with MagLS optimisations [1,2]
 *
 * @param[in]  hA2B           handle
 * @param[in]  ATFs           array TFs; FLAT: nBands x nMics x nDirs
 * @param[in]  hrtf_interp    HRTFs; FLAT: nBands x #NUM_EARS x nDirs
 * @param[in]  centre_freqs   Centre frequencies; nBands x 1
 * @param[in]  nBands         Number of frequency bands
 * @param[in]  nMics          Number of microphones in array
 * @param[in]  nDirs          Number of measurement directions
 * @param[in]  magLScutoff_Hz MagLS cutoff freq, in Hz
 * @param[in]  maxFreq_Hz     Maximum freq up to which to compute BSM, in Hz
 * @param[out] M_array2bin    Mixing matrix; FLAT: nBands x #NUM_EARS x nMics
 *
 * @see [1] Madmoni, L., Donley, J., Tourbabin, V. and Rafaely, B., 2020,
 *          August. Beamforming-based binaural reproduction by matching of
 *          binaural signals. In Audio Engineering Society Conference: 2020 AES
 *          International Conference on Audio for Virtual and Augmented Reality.
 *          Audio Engineering Society.
 * @see [2] Deppisch, T., Helmholz, H. and Ahrens, J., 2021, September.
 *          End-to-end magnitude least squares binaural rendering of spherical
 *          microphone array signals. In 2021 Immersive and 3D Audio: from
 *          Architecture to Automotive (I3DA) (pp. 1-7). IEEE.
 */
void proposed_array2binauralMagLS(/* Input Arguments */
                                  void* hA2B,
                                  float_complex* ATFs,
                                  float_complex* hrtf_interp,
                                  float* weights, //float_complex* W_gains,
                                  float* centre_freqs,
                                  int nBands,
                                  int nMics,
                                  int nDirs,
                                  float magLScutoff_Hz,
                                  float maxFreq_Hz,
                                  /* Output Arguments */
                                  float_complex* M_array2bin);

/**
 * Binaural filter interpolator
 *
 * @param[in]  hAna            proposed analysis handle
 * @param[in]  interpOption    see #PROPOSED_HRTF_INTERP_OPTIONS
 * @param[in]  binConfig       Binaural configuration
 * @param[in]  target_dirs_deg Target/interpolation dirs, in degrees;
 *                             FLAT: nTargetDirs x 2
 * @param[in]  nTargetDirs     Number of target/interpolation directions
 * @param[in]  ENABLE_DIFF_EQ  Flag, 1: enabled diffuse-field EQ, 0: disabled
 * @param[out] hrtf_interp     The interpolated HRTFs;
 *                             nBands x #NUM_EARS x nTargetDirs
 */
void proposed_getInterpolatedHRTFs(/* Input Arguments */
                                   proposed_analysis_handle const hAna,
                                   PROPOSED_HRTF_INTERP_OPTIONS interpOption,
                                   proposed_binaural_config* binConfig,
                                   float* target_dirs_deg,
                                   int nTargetDirs,
                                   int ENABLE_DIFF_EQ,
                                   /* Output Arguments */
                                   float_complex* hrtf_interp);

/**
 * Creates an instance of the space-domain MUSIC implementation
 *
 * @param[in] phMUSIC       (&) address of the sdMUSIC handle
 * @param[in] nMics         Number of microphones in the array
 * @param[in] grid_dirs_deg Scanning grid directions; FLAT: nDirs x 2
 * @param[in] nDirs         Number of scanning directions
 */
void proposed_sdMUSIC_create(void ** const phMUSIC,
                             int nMics,
                             float* grid_dirs_deg,
                             int nDirs);

/**
 * Destroys an instance of the spherical harmonic domain MUSIC implementation,
 * which may be used for computing pseudo-spectrums for visualisation/DoA
 * estimation purposes
 *
 * @param[in] phMUSIC    (&) address of the sdMUSIC handle
 */
void proposed_sdMUSIC_destroy(void ** const phMUSIC);

/**
 * Computes a pseudo-spectrum based on the MUSIC algorithm optionally returning
 * the grid indices corresponding to the N highest peaks (N=nSrcs)
 *
 * @warning The number of sources should not exceed: floor(nMics/2)!
 *
 * @param[in] hMUSIC    sdMUSIC handle
 * @param[in] A_grid    Scanning steering vectors; nMics x nDirs
 * @param[in] Vn        Noise subspace; FLAT: nSH x (nSH - nSrcs)
 * @param[in] nSrcs     Number of sources
 * @param[in] P_music   Pseudo-spectrum (set to NULL if not wanted); nDirs x 1
 * @param[in] peak_inds Indices corresponding to the "nSrcs" highest peaks in
 *                      the pseudo-spectrum (set to NULL if not wanted);
 *                      nSrcs x 1
 */
void proposed_sdMUSIC_compute(/* Input arguments */
                              void* const hMUSIC,
                              float_complex* A_grid,
                              float_complex* Vn,
                              int nSrcs,
                              /* Output arguments */
                              float* P_music,
                              int* peak_inds);

/**
 * Creates an instance of the space-domain PWD implementation
 *
 * @param[in] phPWD         (&) address of the sdPWD handle
 * @param[in] nMics         Number of microphones in the array
 * @param[in] grid_dirs_deg Scanning grid directions; FLAT: nDirs x 2
 * @param[in] nDirs         Number of scanning directions
 */
void proposed_sdPWD_create(void ** const phPWD,
                           int nMics,
                           float* grid_dirs_deg,
                           int nDirs);

/**
 * Destroys an instance of the spherical harmonic domain MUSIC implementation,
 * which may be used for computing pseudo-spectrums for visualisation/DoA
 * estimation purposes
 *
 * @param[in] phMUSIC    (&) address of the sdMUSIC handle
 */
void proposed_sdMUSIC_destroy(void ** const phMUSIC);

/**
 * Computes a pseudo-spectrum based on the MUSIC algorithm optionally returning
 * the grid indices corresponding to the N highest peaks (N=nSrcs)
 *
 * @warning The number of sources should not exceed: floor(nMics/2)!
 *
 * @param[in] hMUSIC    sdMUSIC handle
 * @param[in] A_grid    Scanning steering vectors; nMics x nDirs
 * @param[in] Vn        Noise subspace; FLAT: nSH x (nSH - nSrcs)
 * @param[in] nSrcs     Number of sources
 * @param[in] P_music   Pseudo-spectrum (set to NULL if not wanted); nDirs x 1
 * @param[in] peak_inds Indices corresponding to the "nSrcs" highest peaks in
 *                      the pseudo-spectrum (set to NULL if not wanted);
 *                      nSrcs x 1
 */
void proposed_sdMUSIC_compute(/* Input arguments */
                              void* const hMUSIC,
                              float_complex* A_grid,
                              float_complex* Vn,
                              int nSrcs,
                              /* Output arguments */
                              float* P_music,
                              int* peak_inds);

/**
 * Returns an estimate of the diffuseness, based on [1]
 *
 * @param[in] lambda Eigenvalues; N x 1
 * @param[in] N      Number of eigenvalues
 * @returns an estimate of the diffuseness
 *
 * @see [1] Epain, N. and Jin, C.T., 2016. Spherical harmonic signal covariance
 *          and sound field diffuseness. IEEE/ACM Transactions on Audio, Speech,
 *          and Language Processing, 24(10), pp.1796-1807.
 */
float proposed_comedie(float* lambda,
                       int N);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __PROPOSED_INTERNAL_H_INCLUDED__ */
