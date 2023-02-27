/**
 * @file proposed_synthesis.c
 * @ingroup PROPOSED
 * @brief Source for the proposed synthesis
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

#include "proposed_synthesis.h"
#include "proposed_internal.h"

#define NUM_DIRECTIVITIES ( 9 )
#define DIRECTIVITY_ORDER_MAX ( 4 )
const float directivity_freqs_hz[NUM_DIRECTIVITIES] = {176.776695296637f,  353.553390593274f, 707.106781186548f, 1414.21356237310f, 2828.42712474619f, 5656.85424949238f, 11313.7084989848f, 22627.4169979695f, 1e10f};
const float directivity_orders[NUM_DIRECTIVITIES] = {0.4f, 0.5f, 0.65f, 0.8f, 0.95f, 1.2f, 1.8f, 3.2f, 3.9f};

/* ========================================================================== */
/*                            PROPOSED Synthesis                              */
/* ========================================================================== */

void proposed_synthesis_create
(
    proposed_synthesis_handle* const phSyn,
    proposed_analysis_handle const hAna,
    proposed_binaural_config* binConfig,
    PROPOSED_HRTF_INTERP_OPTIONS interpOption,
    int favour2Daccuracy,
    int enableEPbeamformers,
    int enableDiffEQ_HRTFs,
    int enableDiffEQ_ATFs
)
{
    proposed_synthesis_data* s = (proposed_synthesis_data*)malloc1d(sizeof(proposed_synthesis_data));
    *phSyn = (proposed_synthesis_handle)s;
    proposed_analysis_data *a = (proposed_analysis_data*)(hAna);
    int band, i, j;
    proposed_binaural_config* bConfig;
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f); /* blas */

    /* User configuration parameters */
    s->interpOption = interpOption;
 
    /* Default user parameters */
    s->eq = malloc1d(a->nBands * sizeof(float));
    s->streamBalance = malloc1d(a->nBands * sizeof(float));
    for(band = 0; band<a->nBands; band++){
        s->eq[band] = 1.0;              /* Flat EQ */
        s->streamBalance[band] = 1.0f;  /* 50/50 direct/ambient balance (i.e., no biasing) */
    }
    s->synAvgCoeff = 0.92f;  /* How much averaging of current mixing matrices with the previous mixing matrices */
    s->maxBSMFreq = 9e3f;
    s->maxMagLSFreq = 1.5e3f;
    s->linear2parBalance = 1.0f;

    /* Things relevant to the synthesiser, which are copied from the analyser to keep things aligned */
    s->fs = a->fs;
    s->nBands = a->nBands;
    s->hopsize = a->hopsize;
    s->blocksize = a->blocksize;
    s->nDirs = a->nDirs;
    s->nMics = a->nMics;
    s->H_array = malloc1d(s->nBands*(s->nMics)*(s->nDirs)*sizeof(float_complex));
    memcpy(s->H_array, a->H_array, s->nBands*(s->nMics)*(s->nDirs)*sizeof(float_complex));
    s->DCM_array = malloc1d(s->nBands*(s->nMics)*(s->nMics)*sizeof(float_complex));
    memcpy(s->DCM_array, a->DCM_array, s->nBands*(s->nMics)*(s->nMics)*sizeof(float_complex));
    s->W = malloc1d(s->nDirs*(s->nDirs)*sizeof(float_complex));
    memcpy(s->W, a->W, s->nDirs*(s->nDirs)*sizeof(float_complex));
    s->array_dirs_deg = malloc1d((s->nDirs)*2*sizeof(float));
    memcpy(s->array_dirs_deg, a->array_dirs_deg, (s->nDirs)*2*sizeof(float));
    s->array_dirs_xyz = (float**)malloc2d((s->nDirs), 3, sizeof(float));
    memcpy(FLATTEN2D(s->array_dirs_xyz), a->array_dirs_xyz, (s->nDirs)*3*sizeof(float));
    s->timeSlots = a->timeSlots;
    s->freqVector = malloc1d(s->nBands*sizeof(float));
    memcpy(s->freqVector, a->freqVector, s->nBands*sizeof(float));

    /* Time-frequency transform */
    afSTFT_create(&(s->hFB_dec), 0, NUM_EARS, s->hopsize, 1, 0, AFSTFT_BANDS_CH_TIME);
 
    /* Copy binaural configuration */
    s->binConfig = malloc1d(sizeof(proposed_binaural_config));
    bConfig = s->binConfig;
    bConfig->lHRIR = binConfig->lHRIR;
    bConfig->nHRIR = binConfig->nHRIR;
    bConfig->hrir_fs = binConfig->hrir_fs;
    bConfig->hrirs = malloc1d(bConfig->nHRIR * NUM_EARS * (bConfig->lHRIR) * sizeof(float));
    memcpy(bConfig->hrirs, binConfig->hrirs, bConfig->nHRIR * NUM_EARS * (bConfig->lHRIR) * sizeof(float));
    bConfig->hrir_dirs_deg = malloc1d(bConfig->nHRIR*2*sizeof(float));
    memcpy(bConfig->hrir_dirs_deg, binConfig->hrir_dirs_deg, bConfig->nHRIR*2*sizeof(float));
    
    /* DIRECT-STREAM Pre-process HRTFs, interpolate them for the scanning grid */
    s->H_bin = calloc1d(s->nBands*NUM_EARS*(s->nDirs),sizeof(float_complex));
    proposed_getInterpolatedHRTFs(hAna, interpOption, bConfig, a->array_dirs_deg, s->nDirs, enableDiffEQ_HRTFs, s->H_bin);
    
    /* AMBIENT-STREAM */
    s->nDiff = __Tdesign_degree_21_nPoints;
    s->diff_dirs_xyz = malloc1d(s->nDiff*3*sizeof(float));
    unitSph2cart((float*)__Tdesign_degree_21_dirs_deg, s->nDiff, 1, s->diff_dirs_xyz);
    s->H_array_diff = calloc1d(s->nBands*s->nMics*(s->nDiff),sizeof(float_complex));
    s->H_bin_diff = calloc1d(s->nBands*NUM_EARS*(s->nDiff),sizeof(float_complex));
    s->diff_indices = malloc1d(s->nDiff*sizeof(int));
    proposed_findNearestGridIndices(FLATTEN2D(s->array_dirs_xyz), s->diff_dirs_xyz, s->nDirs, s->nDiff, s->diff_indices);
    for(band=0; band<s->nBands; band++){
        for(i=0; i<s->nMics; i++)
            for(j=0; j<s->nDiff; j++)
                s->H_array_diff[band*s->nMics*s->nDiff + i*s->nDiff + j] = s->H_array[band*s->nMics*s->nDirs + i*s->nDirs + s->diff_indices[j]];
        for(i=0; i<NUM_EARS; i++)
            for(j=0; j<s->nDiff; j++)
                s->H_bin_diff[band*NUM_EARS*s->nDiff + i*s->nDiff + j] = s->H_bin[band*NUM_EARS*s->nDirs + i*s->nDirs + s->diff_indices[j]];
    }
    
    /* Diffuse-field equalisation term, as in [2] */
    float_complex* D_array, *D_bin;
    D_array = malloc1d(s->nBands*s->nMics*s->nMics*sizeof(float_complex));
    D_bin = malloc1d(s->nBands*NUM_EARS*NUM_EARS*sizeof(float_complex));
    diffCohMtxMeas(s->H_array_diff, s->nBands, s->nMics, s->nDiff, NULL, D_array);
    cblas_sscal(/*re+im*/2*s->nBands*s->nMics*s->nMics, 1.0f/(float)s->nDiff, (float*)D_array, 1);
    diffCohMtxMeas(s->H_bin_diff, s->nBands, NUM_EARS, s->nDiff, NULL, D_bin);
    s->diffEQ = malloc1d(s->nBands*sizeof(float));
    for (band = 0; band < s->nBands; band++)
        s->diffEQ[band] = enableDiffEQ_ATFs ? SAF_MIN(sqrtf(1.0f / cblas_scasum(s->nMics, &D_array[band * s->nMics * s->nMics], s->nMics + 1)), 2.0f) : 1.0f;  //cblas_scasum(NUM_EARS, &s->H_bin_diff[band * NUM_EARS * NUM_EARS], NUM_EARS + 1)
    free(D_array);
    free(D_bin);
     
    /* BSM 6DoF baseline */
    s->M_BSM = malloc1d(s->nBands*NUM_EARS*s->nMics*sizeof(float_complex));
    s->diff_pos_xyz = malloc1d(s->nDiff*3*sizeof(float));
    s->diff_dirs_xyz_new = malloc1d(s->nDiff*3*sizeof(float));
    s->diff_dirs_xyz_rot = malloc1d(s->nDiff*3*sizeof(float));
    s->diff_gains = calloc1d(s->nDiff, sizeof(float));
    proposed_array2binauralMagLS_create(&s->hBSM, s->H_array_diff, s->nBands, s->nMics, s->nDiff);
       
    /* Linear 6DoF baseline */
    s->nPWD = favour2Daccuracy ? 24 : __Tdesign_degree_6_nPoints;
    s->pwd_dirs_xyz = malloc1d(s->nPWD*3*sizeof(float));
    s->pwd_pos_xyz = malloc1d(s->nPWD*3*sizeof(float));
    s->pwd_dirs_xyz_rot = malloc1d(s->nPWD*3*sizeof(float));
    float pwd_dirs_deg[24][2] = { {0.0f} };
    if(favour2Daccuracy){
        for (i = 0; i < s->nPWD; i++)
            pwd_dirs_deg[i][0] = (float)i * 360.0f / (float)s->nPWD - 180.0f;
        unitSph2cart((float*)pwd_dirs_deg, s->nPWD, 1, s->pwd_dirs_xyz);
    }
    else{ 
        unitSph2cart((float*)__Tdesign_degree_6_dirs_deg, s->nPWD, 1, s->pwd_dirs_xyz);
    }
    s->pwd_indices = malloc1d(s->nPWD*sizeof(int));
    s->pwd_gains = malloc1d(s->nPWD*sizeof(float));
    proposed_findNearestGridIndices(FLATTEN2D(s->array_dirs_xyz), s->pwd_dirs_xyz, s->nDirs, s->nPWD, s->pwd_indices);
    s->M_PWD = malloc1d(s->nBands*s->nPWD*s->nMics*sizeof(float_complex));
    float_complex* Ad;
    Ad = malloc1d(s->nMics*s->nPWD*sizeof(float_complex));
    s->M_HRTFs = malloc1d(s->nBands*NUM_EARS*s->nPWD*sizeof(float_complex));
    float_complex* U, *V;
    U = V = NULL;
    if(enableEPbeamformers){
        U = malloc1d(SAF_MAX(s->nMics, s->nPWD)*SAF_MAX(s->nMics, s->nPWD)*sizeof(float_complex));
        V = malloc1d(SAF_MAX(s->nMics, s->nPWD)*SAF_MAX(s->nMics, s->nPWD)*sizeof(float_complex));
    }
    for(band=0; band<s->nBands; band++){
        for(i=0; i<s->nMics; i++)
            for(j=0; j<s->nPWD; j++)
                Ad[i*s->nPWD + j] = s->H_array[band*s->nMics*s->nDirs + i*s->nDirs + s->pwd_indices[j]];
        if(enableEPbeamformers){
            /* As it is done in [2]: */
            utility_csvd(NULL, Ad, s->nMics, s->nPWD, U, NULL, V, NULL);
            if (s->nPWD>s->nMics){
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, s->nPWD, s->nMics, s->nMics, &calpha,
                            V, s->nPWD,
                            U, s->nMics, &cbeta,
                            &s->M_PWD[band*s->nPWD*s->nMics], s->nMics);
            }
            else{
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, s->nPWD, s->nMics, s->nPWD, &calpha,
                            V, s->nPWD,
                            U, s->nMics, &cbeta,
                            &s->M_PWD[band*s->nPWD*s->nMics], s->nMics);
            }
            cblas_sscal(2*s->nPWD*s->nMics, sqrtf(1.0f/(float)s->nMics), (float*)&s->M_PWD[band*s->nPWD*s->nMics], 1);
        }
        else{
            utility_cpinv(NULL, Ad, s->nMics, s->nPWD, &s->M_PWD[band*s->nPWD*s->nMics]);
            cblas_sscal(2*s->nPWD*s->nMics, sqrtf((float)s->nMics)/(float)s->nMics, (float*)&s->M_PWD[band*s->nPWD*s->nMics], 1);
        }
        for(i=0; i<NUM_EARS; i++)
            for(j=0; j<s->nPWD; j++)
                s->M_HRTFs[band*NUM_EARS*s->nPWD + i*s->nPWD + j] = s->H_bin[band*NUM_EARS*s->nDirs + i*s->nDirs + s->pwd_indices[j]];
    }
    free(Ad);
    if(enableEPbeamformers){
        free(U);
        free(V);
    }
    
    /* Run-time variables */
    utility_cpinv_create(&(s->hPinv), s->nMics, s->nMics);
    utility_cglslv_create(&(s->hLinSolve), s->nMics, s->nMics);
    utility_cinv_create(&(s->hInv), s->nMics);
    s->As   = malloc1d(s->nMics*PROPOSED_MAX_K*sizeof(float_complex));
    s->Ds   = malloc1d(PROPOSED_MAX_K*s->nMics*sizeof(float_complex));
    s->Dd   = malloc1d(s->nMics*s->nMics*sizeof(float_complex));
    s->new_M_par = malloc1d(NUM_EARS*(s->nMics)*sizeof(float_complex));
    s->new_M_lin = malloc1d(NUM_EARS*(s->nMics)*sizeof(float_complex));
    s->M_par = (float_complex**)malloc2d(s->nBands, NUM_EARS*(s->nMics), sizeof(float_complex));
    s->M  = (float_complex**)malloc2d(s->nBands, NUM_EARS*(s->nMics), sizeof(float_complex));

    /* Run-time audio buffers */
    s->outTF = (float_complex***)malloc3d(s->nBands, NUM_EARS, s->timeSlots, sizeof(float_complex));
    s->outTD = (float**)malloc2d(NUM_EARS, s->blocksize, sizeof(float));

    /* Flush run-time buffers with zeros */
    proposed_synthesis_reset((*phSyn));
}

void proposed_synthesis_destroy
(
    proposed_synthesis_handle* const phSyn
)
{
    proposed_synthesis_data *s = (proposed_synthesis_data*)(*phSyn);

    if (s != NULL) {
        /* Free user parameters */
        free(s->eq);
        free(s->streamBalance);
        free(s->binConfig);

        /* Free things copied from analyser */
        free(s->H_array);
        free(s->DCM_array);
        free(s->W);
        free(s->array_dirs_deg);
        free(s->array_dirs_xyz);
        free(s->freqVector);

        /* Free time-frequency transform */
        afSTFT_destroy(&(s->hFB_dec));

        /* HRTF and diffuse rendering variables */
        free(s->H_bin);
        free(s->diff_dirs_xyz);
        free(s->H_array_diff);
        free(s->H_bin_diff);
        free(s->diff_indices);
        free(s->diffEQ);
      
        /* AMBIENT-STREAM  - BSM */
        free(s->M_BSM);
        free(s->diff_pos_xyz);
        free( s->diff_dirs_xyz_new);
        free(s->diff_dirs_xyz_rot);
        proposed_array2binauralMagLS_destroy(&s->hBSM);

        /* Linear 6DoF baseline */
        free(s->pwd_dirs_xyz);
        free(s->pwd_pos_xyz);
        free(s->pwd_dirs_xyz_rot);
        free(s->pwd_indices);
        free(s->pwd_gains);
        free(s->M_PWD);
        free(s->M_HRTFs);
       
        /* Run-time variables */
        utility_cpinv_destroy(&(s->hPinv));
        utility_cglslv_destroy(&(s->hLinSolve));
        utility_cinv_destroy(&(s->hInv));
        free(s->As);
        free(s->Ds);
        free(s->Dd);
        free(s->new_M_par);
        free(s->new_M_lin);
        free(s->M_par);
        free(s->M);
         
        /* Run-time audio buffers */
        free(s->outTF);
        free(s->outTD);

        free(s);
        s = NULL;
        (*phSyn) = NULL;
    }
}

void proposed_synthesis_reset
(
    proposed_synthesis_handle const hSyn
)
{
    proposed_synthesis_data *s;
    if(hSyn==NULL)
        return;
    s = (proposed_synthesis_data*)(hSyn);

    /* Zero buffers, matrices etc. */
    afSTFT_clearBuffers(s->hFB_dec);
    memset(FLATTEN2D(s->M_par), 0, s->nBands*NUM_EARS*(s->nMics)*sizeof(float_complex));
    memset(FLATTEN2D(s->M), 0, s->nBands*NUM_EARS*(s->nMics)*sizeof(float_complex));
}

void proposed_synthesis_apply
(
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
    float** output
)
{
    proposed_synthesis_data *s = (proposed_synthesis_data*)(hSyn);
    proposed_param_container_data *pcon = (proposed_param_container_data*)(hPCon);
    proposed_signal_container_data *scon = (proposed_signal_container_data*)(hSCon); 
    int i, j, ch, nMics, band, K;
    int doa_idx[PROPOSED_MAX_K], gain_idx[PROPOSED_MAX_K];
    float a, b, synAvgCoeff, lin2parBalance, streamBalance, norm, Rzyx[3][3], maxBSMFreq;
    float src_gains[PROPOSED_MAX_K];
    float src_dirs_xyz[PROPOSED_MAX_K][3], src_dirs_xyz_rot[PROPOSED_MAX_K][3], src_pos_xyz[PROPOSED_MAX_K][3];
    float pwd_dirs_xyz[64][3]; 
    float_complex h_dir[NUM_EARS*PROPOSED_MAX_K];
    float_complex new_Md[NUM_EARS*PROPOSED_MAX_NMICS];
    int dir;
    float order_directivity, src_dist_m_MAP;
    int order_directivity_cl, order_directivity_fl;
    float c_n[DIRECTIVITY_ORDER_MAX+1];
    float c_n_fl[DIRECTIVITY_ORDER_MAX+1];
    float c_nm_base[ORDER2NSH(DIRECTIVITY_ORDER_MAX)];
    float c_nm[ORDER2NSH(DIRECTIVITY_ORDER_MAX)];
    float y_nm[ORDER2NSH(DIRECTIVITY_ORDER_MAX)];
    float src_dir_rad_before[2], src_range_deg;
    float src_dir_rad_after[2], src_dir_rad_incl_after[2];
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f); /* blas */
    CxMic Cx_betaI, inv_Cx_betaI;
    float_complex AH_Cx[PROPOSED_MAX_NMICS*PROPOSED_MAX_NMICS], AH_Cx_A[PROPOSED_MAX_K*PROPOSED_MAX_K], inv_AH_Cx_A[PROPOSED_MAX_K*PROPOSED_MAX_K];

    maxBSMFreq = s->maxBSMFreq;
    nMics = s->nMics;
    synAvgCoeff = SAF_CLAMP((s->synAvgCoeff), 0.0f, 0.99f);
    lin2parBalance = SAF_CLAMP((s->linear2parBalance), 0.0f, 0.8f);
    src_range_deg = 13.0f;
    
    /* Update ambient rendering matrices to account for head-rotations (no translation) */
    /* Rotation matrix */
    euler2rotationMatrix(ypr_rad[0], ypr_rad[1], ypr_rad[2], 0, EULER_ROTATION_YAW_PITCH_ROLL, Rzyx);
    
    /* Compute rotated BSM matrices up to the maximum specified frequency: */
    if (maxBSMFreq>0.1f){
        cblas_scopy(s->nDiff*3, s->diff_dirs_xyz, 1, s->diff_pos_xyz, 1);
        cblas_sscal(s->nDiff*3, src_dist_m, s->diff_pos_xyz, 1);
        for(j=0; j<s->nDiff; j++){
            /* New source direction */
            s->diff_dirs_xyz_new[j*3+0] = s->diff_pos_xyz[j*3+0] - xyz_m[0];
            s->diff_dirs_xyz_new[j*3+1] = s->diff_pos_xyz[j*3+1] - xyz_m[1];
            s->diff_dirs_xyz_new[j*3+2] = s->diff_pos_xyz[j*3+2] - xyz_m[2];
            norm = L2_norm3(&s->diff_dirs_xyz_new[j*3]);
            s->diff_dirs_xyz_new[j*3+0] /= norm;
            s->diff_dirs_xyz_new[j*3+1] /= norm;
            s->diff_dirs_xyz_new[j*3+2] /= norm;
            
            /* Account for 1/R law */
            s->diff_gains[j] = src_dist_m/(getDistBetween2Points(&s->diff_pos_xyz[j*3], xyz_m)+0.0001f);
        }
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, s->nDiff, 3, 3, 1.0f,
                    s->diff_dirs_xyz_new, 3,
                    (float*)Rzyx, 3, 0.0f,
                    s->diff_dirs_xyz_rot, 3);
        proposed_findNearestGridIndices(FLATTEN2D(s->array_dirs_xyz), s->diff_dirs_xyz_rot, s->nDirs, s->nDiff, s->diff_indices);
        for(band=0; band<s->nBands; band++){
            if(s->freqVector[band] <= maxBSMFreq)
                for(i=0; i<NUM_EARS; i++)
                    for(j=0; j<s->nDiff; j++)
                        s->H_bin_diff[band*NUM_EARS*s->nDiff + i*s->nDiff + j] = s->H_bin[band*NUM_EARS*s->nDirs + i*s->nDirs + s->diff_indices[j]];
        }
    }
    proposed_array2binauralMagLS(s->hBSM, s->H_array_diff, s->H_bin_diff, s->diff_gains, s->freqVector, s->nBands, s->nMics, s->nDiff, s->maxMagLSFreq, maxBSMFreq, s->M_BSM);
    
    /* Rotatate HRTFs for the linear decoder */
    cblas_scopy(s->nPWD*3, s->pwd_dirs_xyz, 1, s->pwd_pos_xyz, 1);
    cblas_sscal(s->nPWD*3, src_dist_m, s->pwd_pos_xyz, 1);
    for(j=0; j<s->nPWD; j++){
        /* New source direction */
        pwd_dirs_xyz[j][0] = s->pwd_pos_xyz[j*3+0] - xyz_m[0];
        pwd_dirs_xyz[j][1] = s->pwd_pos_xyz[j*3+1] - xyz_m[1];
        pwd_dirs_xyz[j][2] = s->pwd_pos_xyz[j*3+2] - xyz_m[2];
        norm = L2_norm3((float*)pwd_dirs_xyz[j]);
        pwd_dirs_xyz[j][0] /= norm;
        pwd_dirs_xyz[j][1] /= norm;
        pwd_dirs_xyz[j][2] /= norm;
        
        /* Account for 1/R law */
        s->pwd_gains[j] = (src_dist_m/(getDistBetween2Points(&s->pwd_pos_xyz[j*3], xyz_m)+0.0001f));
    }
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, s->nPWD, 3, 3, 1.0f,
                (float*)pwd_dirs_xyz, 3,
                (float*)Rzyx, 3, 0.0f,
                (float*)s->pwd_dirs_xyz_rot, 3);
    proposed_findNearestGridIndices(FLATTEN2D(s->array_dirs_xyz), (float*)s->pwd_dirs_xyz_rot, s->nDirs, s->nPWD, s->pwd_indices);
    for(band=0; band<s->nBands; band++){
        if(s->freqVector[band]>maxBSMFreq)
            for(i=0; i<NUM_EARS; i++)
                for(j=0; j<s->nPWD; j++)
                    s->M_HRTFs[band*NUM_EARS*s->nPWD + i*s->nPWD + j] = crmulf(s->H_bin[band*NUM_EARS*s->nDirs + i*s->nDirs + s->pwd_indices[j]], SAF_MIN(s->pwd_gains[j], 8.0f));  // needed for pinv // 2*sqrtf(0.5f)*
    }
     
    /* Loop over bands and compute the mixing matrices */
    for (band = 0; band < s->nBands; band++) {
        /* Pull estimated (and possibly modified) spatial parameters for this band */
        K = pcon->nSrcs[band];
        memcpy(doa_idx, pcon->doa_idx[band], K*sizeof(int));
        memcpy(gain_idx, pcon->gains_idx[band], K*sizeof(int));
        memcpy(src_gains, pcon->src_gains[band], K*sizeof(float));

        /* Optional biasing (e.g. to conduct de-reverberation or to emphasise reverberation) [4] */
        streamBalance = SAF_CLAMP(s->streamBalance[band], 0.0f, 2.0f);
        if(streamBalance<1.0f){
            a = streamBalance;        /* pump more direct energy into output */
            b = 1.0f;                 /* pass ambient stream as normal */
        }
        else {
            a = 1.0f;                 /* pass source stream as normal */
            b = 2.0f - streamBalance; /* pump less ambient energy into output */
        }
        
        /* Linear baseline method */
        if(s->freqVector[band]<=maxBSMFreq)
            cblas_ccopy(NUM_EARS*nMics, &s->M_BSM[band*NUM_EARS*nMics], 1, s->new_M_lin, 1);
        else{
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, nMics, s->nPWD, &calpha,
                        &s->M_HRTFs[band*NUM_EARS*s->nPWD], s->nPWD,
                        &s->M_PWD[band*s->nPWD*nMics], nMics, &cbeta,
                        s->new_M_lin, nMics);
        }
        
        /* Parametric method */
        if(K>0 && s->freqVector[band]<PROPOSED_MAX_RENDERING_FREQ){
            /* Analysed source directions */
            for(j=0; j<K; j++){
                src_dirs_xyz[j][0] = s->array_dirs_xyz[gain_idx[j]][0];
                src_dirs_xyz[j][1] = s->array_dirs_xyz[gain_idx[j]][1];
                src_dirs_xyz[j][2] = s->array_dirs_xyz[gain_idx[j]][2];
                unitCart2sph(src_dirs_xyz[j], 1, 0, src_dir_rad_before);
                
                switch(dist_map){
                    case PROPOSED_DISTANCE_MAP_1SRC:
                        if (src_dir_rad_before[0]*180.0f/SAF_PI >-50.0f-src_range_deg && src_dir_rad_before[0]*180.0f/SAF_PI <-50.0f+src_range_deg)
                            src_dist_m_MAP = src_dist_m;
                        else
                            src_dist_m_MAP = 5.0f;
                        break;
                    case PROPOSED_DISTANCE_MAP_2SRC:
                        if (src_dir_rad_before[0]*180.0f/SAF_PI >-50.0f-src_range_deg && src_dir_rad_before[0]*180.0f/SAF_PI <-50.0f+src_range_deg)
                            src_dist_m_MAP = src_dist_m;
                        else if (src_dir_rad_before[0]*180.0f/SAF_PI >-120.0f-src_range_deg && src_dir_rad_before[0]*180.0f/SAF_PI <-120.0f+src_range_deg)
                            src_dist_m_MAP = src_dist_m;
                        else
                            src_dist_m_MAP = 5.0f;
                        break;
                    case PROPOSED_DISTANCE_MAP_3SRC:
                        if (src_dir_rad_before[0]*180.0f/SAF_PI >-50.0f-src_range_deg && src_dir_rad_before[0]*180.0f/SAF_PI <-50.0f+src_range_deg)
                            src_dist_m_MAP = src_dist_m;
                        else if (src_dir_rad_before[0]*180.0f/SAF_PI >-120.0f-src_range_deg && src_dir_rad_before[0]*180.0f/SAF_PI <-120.0f+src_range_deg)
                            src_dist_m_MAP = src_dist_m;
                        else if (src_dir_rad_before[0]*180.0f/SAF_PI >60.0f-src_range_deg && src_dir_rad_before[0]*180.0f/SAF_PI <60.0f+src_range_deg)
                            src_dist_m_MAP = src_dist_m;
                        else
                            src_dist_m_MAP = 5.0f;
                        break;
                    case PROPOSED_DISTANCE_MAP_USE_PARAM:
                        src_dist_m_MAP = src_dist_m;
                        break;
                }
                
                /* Multiply unit vector by source distance to get its position */
                src_pos_xyz[j][0] = src_dirs_xyz[j][0] * src_dist_m_MAP;
                src_pos_xyz[j][1] = src_dirs_xyz[j][1] * src_dist_m_MAP;
                src_pos_xyz[j][2] = src_dirs_xyz[j][2] * src_dist_m_MAP;
                
                /* New source direction */
                src_dirs_xyz[j][0] = src_pos_xyz[j][0] - xyz_m[0];
                src_dirs_xyz[j][1] = src_pos_xyz[j][1] - xyz_m[1];
                src_dirs_xyz[j][2] = src_pos_xyz[j][2] - xyz_m[2];
                norm = L2_norm3((float*)src_dirs_xyz[j]);
                src_dirs_xyz[j][0] /= norm;
                src_dirs_xyz[j][1] /= norm;
                src_dirs_xyz[j][2] /= norm;
                unitCart2sph(src_dirs_xyz[j], 1, 0, src_dir_rad_after);
                src_dir_rad_incl_after[0] = src_dir_rad_after[0];
                src_dir_rad_incl_after[1] = SAF_PI/2.0f - src_dir_rad_after[1];
                
                /* Account for 1/R law */
                src_gains[j] = src_dist_m_MAP/(getDistBetween2Points(src_pos_xyz[j], xyz_m)+0.0001f);
             
                /* Account for source directivity */
                if(enableSrcD){
                    order_directivity = 0;
                    for(dir=NUM_DIRECTIVITIES-1; dir>=0; dir--)
                        if(s->freqVector[band]<directivity_freqs_hz[dir])
                            order_directivity = directivity_orders[dir];
                    order_directivity_cl = (int)(order_directivity+1.0f);
                    order_directivity_fl = (int)order_directivity;
                    beamWeightsCardioid2Spherical(order_directivity_cl, (float*)c_n);
                    beamWeightsCardioid2Spherical(order_directivity_fl, (float*)c_n_fl);
                    cblas_sscal(order_directivity_cl+1, (order_directivity-(float)order_directivity_fl), c_n, 1);
                    cblas_saxpy(order_directivity_fl+1, (1.0f - (order_directivity-(float)order_directivity_fl)), c_n_fl, 1, c_n, 1);
                    rotateAxisCoeffsReal(order_directivity_cl, c_n, SAF_PI/2.0f-src_dir_rad_before[1], src_dir_rad_before[0], c_nm_base);
                    rotateAxisCoeffsReal(order_directivity_cl, c_n, SAF_PI/2.0f-src_dir_rad_after[1],  src_dir_rad_after[0],  c_nm);
                    getSHreal_recur(order_directivity_cl, src_dir_rad_incl_after, 1, y_nm);
                    src_gains[j] *= fabsf( cblas_sdot(ORDER2NSH(order_directivity_cl), c_nm_base, 1, y_nm, 1) / (cblas_sdot(ORDER2NSH(order_directivity_cl), c_nm, 1, y_nm, 1) +0.0001f) );
                    
                    /* Maximum gain permitted is 18dB: */
                    src_gains[j] = SAF_MIN(src_gains[j], 8.0f);
                }
                 
                /* Apply head-rotation */
                cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, K, 3, 3, 1.0f,
                            (float*)src_dirs_xyz, 3,
                            (float*)Rzyx, 3, 0.0f,
                            (float*)src_dirs_xyz_rot, 3);
                proposed_findNearestGridIndices(FLATTEN2D(s->array_dirs_xyz), (float*)src_dirs_xyz_rot, s->nDirs, K, gain_idx);
            }
            
            /* Source array steering vectors for the estimated DoAs */
            for(i=0; i<nMics; i++)
                for(j=0; j<K; j++)
                    s->As[i*K+j] = s->H_array[band*nMics*(s->nDirs) + i*(s->nDirs) + doa_idx[j]];

            /* HRTF for these reproduction DoAs */
            for(i=0; i<NUM_EARS; i++)
                for(j=0; j<K; j++)
                    h_dir[i*K+j] = crmulf(s->H_bin[band*NUM_EARS*(s->nDirs) + i*(s->nDirs) + gain_idx[j]], src_gains[j]);

            /* Source mixing matrix (beamforming towards the estimated DoAs) */
#if 1
            /* As in e.g. [2]: */
            cblas_ccopy(nMics*nMics, scon->Cx[band].Cx, 1, Cx_betaI.Cx, 1);
            for(i=0; i<nMics; i++)
                Cx_betaI.Cx[i*nMics+i] = craddf(Cx_betaI.Cx[i*nMics+i], 0.01f);
            utility_cinv(s->hInv, Cx_betaI.Cx, inv_Cx_betaI.Cx, nMics);
            cblas_cgemm(CblasRowMajor, CblasConjTrans, CblasNoTrans, K, nMics, nMics, &calpha,
                        s->As, K,
                        inv_Cx_betaI.Cx, nMics, &cbeta,
                        AH_Cx, nMics);
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, K, K, nMics, &calpha,
                        AH_Cx, nMics,
                        s->As, K, &cbeta,
                        AH_Cx_A, K);
            utility_cinv(s->hInv, AH_Cx_A, inv_AH_Cx_A, K);
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, K, nMics, K, &calpha,
                        inv_AH_Cx_A, K,
                        AH_Cx, nMics, &cbeta,
                        s->Ds, nMics); 
#else
            /* As in e.g. [1]: */
            utility_cpinv(s->hPinv, s->As, nMics, K, s->Ds);
#endif
            
            /* Residual mixing matrix */
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, nMics, nMics, K, &calpha,
                        s->As, K,
                        s->Ds, nMics, &cbeta,
                        s->Dd, nMics);
            for(i=0; i<nMics; i++)
                for(j=0; j<nMics; j++)
                    s->Dd[i*nMics+j] = i==j ? ccsubf(calpha, s->Dd[i*nMics+j]) : crmulf(s->Dd[i*nMics+j], -1.0f);

            /* Source stream */
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, nMics, K, &calpha,
                        h_dir, K,
                        s->Ds, nMics, &cbeta,
                        s->new_M_par, nMics);
            cblas_sscal(/*re+im*/2*NUM_EARS*nMics, a, (float*)s->new_M_par, 1);
            
            /* Ambient stream*/
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, nMics, nMics, &calpha,
                        s->new_M_lin, nMics,
                        s->Dd, nMics, &cbeta,
                        new_Md, nMics);
#if PROPOSED_USE_BSM_RESIDUAL
            cblas_saxpy(/*re+im*/2*NUM_EARS*nMics, s->diffEQ[band]*b * SAF_CLAMP(1.0f-sqrtf(xyz_m[0]*xyz_m[0] + xyz_m[1]*xyz_m[1] + xyz_m[2]*xyz_m[2]+0.0001)/src_dist_m, 0.0f, 1.0f), (float*)new_Md, 1, (float*)s->new_M_par, 1);
#else
            cblas_saxpy(/*re+im*/2*NUM_EARS*nMics, s->diffEQ[band]*b, (float*)new_Md, 1, (float*)s->new_M_par, 1);
#endif
        }
        else{
#if PROPOSED_USE_BSM_RESIDUAL
            cblas_ccopy(NUM_EARS*(s->nMics), &s->M_diff[band*NUM_EARS*nMics], 1, s->new_M, 1);
#else
            cblas_ccopy(NUM_EARS*(s->nMics), s->new_M_lin, 1, s->new_M_par, 1);
            cblas_sscal(2*NUM_EARS*s->nMics, s->diffEQ[band], (float*)s->new_M_par, 1);
#endif
        }
        
        /* Temporal averaging of parametric mixing matrices */
        cblas_sscal(/*re+im*/2*NUM_EARS*nMics, synAvgCoeff, (float*)s->M_par[band], 1);
        cblas_saxpy(/*re+im*/2*NUM_EARS*nMics, 1.0f-synAvgCoeff, (float*)s->new_M_par, 1, (float*)s->M_par[band], 1);
        
        /* Mix together the parametric rendering and the linear baseline */
        cblas_ccopy(NUM_EARS*nMics, s->M_par[band], 1, s->M[band], 1);
        cblas_sscal(/*re+im*/2*NUM_EARS*nMics, lin2parBalance, (float*)s->M[band], 1);
        cblas_saxpy(/*re+im*/2*NUM_EARS*nMics, s->diffEQ[band]*(1.0f-lin2parBalance), (float*)s->new_M_lin, 1, (float*)s->M[band], 1);
        
        /* Reduce the level by 6dB */
        cblas_sscal(/*re+im*/2*NUM_EARS*nMics, 0.5f, (float*)s->M[band], 1);
    }

    /* Apply mixing matrices */
    for(band=0; band<s->nBands; band++){
        cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, s->timeSlots, nMics, &calpha,
                    s->M[band], nMics,
                    FLATTEN2D(scon->inTF[band]), s->timeSlots, &cbeta,
                    FLATTEN2D(s->outTF[band]), s->timeSlots);
    }

    /* inverse time-frequency transform */
    afSTFT_backward_knownDimensions(s->hFB_dec, s->outTF, blocksize, NUM_EARS, s->timeSlots, s->outTD);

    /* Copy to output */
    for(ch=0; ch<SAF_MIN(nChannels, NUM_EARS); ch++)
        memcpy(output[ch], s->outTD[ch], blocksize*sizeof(float));
    for(; ch<nChannels; ch++)
        memset(output[ch], 0, blocksize*sizeof(float));
}

float* proposed_synthesis_getEqPtr
(
    proposed_synthesis_handle const hSyn,
    int* nBands
)
{
    proposed_synthesis_data *s;
    if(hSyn==NULL){
        if(nBands!=NULL)
            (*nBands) = 0;
        return NULL;
    }
    s = (proposed_synthesis_data*)(hSyn);
    if(nBands!=NULL)
       (*nBands) = s->nBands;
    return s->eq;
}

float* proposed_synthesis_getStreamBalancePtr
(
    proposed_synthesis_handle const hSyn,
    int* nBands
)
{
    proposed_synthesis_data *s;
    if(hSyn==NULL){
        if(nBands!=NULL)
            (*nBands) = 0;
        return NULL;
    }
    s = (proposed_synthesis_data*)(hSyn);
    if(nBands!=NULL)
       (*nBands) = s->nBands;
    return s->streamBalance;
}

float* proposed_synthesis_getSynthesisAveragingCoeffPtr
(
    proposed_synthesis_handle const hSyn
)
{
    proposed_synthesis_data *s;
    if(hSyn==NULL)
        return NULL;
    s = (proposed_synthesis_data*)(hSyn);
    return &(s->synAvgCoeff);
}

float* proposed_synthesis_getMaxBSMFreqPtr
(
    proposed_synthesis_handle const hSyn
)
{
    proposed_synthesis_data *s;
    if(hSyn==NULL)
        return NULL;
    s = (proposed_synthesis_data*)(hSyn);
    return &(s->maxBSMFreq);
}

float* proposed_synthesis_getMaxMagLSFreqPtr
(
    proposed_synthesis_handle const hSyn
)
{
    proposed_synthesis_data *s;
    if(hSyn==NULL)
        return NULL;
    s = (proposed_synthesis_data*)(hSyn);
    return &(s->maxMagLSFreq);
}

float* proposed_synthesis_getLinear2ParametricBalancePtr
(
    proposed_synthesis_handle const hSyn
)
{
    proposed_synthesis_data *s;
    if(hSyn==NULL)
        return NULL;
    s = (proposed_synthesis_data*)(hSyn);
    return &(s->linear2parBalance);
}
 
int proposed_synthesis_getProcDelay
(
    proposed_synthesis_handle const hSyn
)
{
    if(hSyn==NULL)
        return 0;
    return 0; /* Accounted for in proposed_analysis_getProcDelay() */
}
