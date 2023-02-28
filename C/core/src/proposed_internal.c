/**
 * @file proposed_internal.c
 * @ingroup PROPOSED
 * @brief Internal source for the proposed method
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

#include "proposed_internal.h"

void proposed_findNearestGridIndices
(
    float* grid_dirs_xyz,
    float* target_dirs_xyz,
    int nDirs,
    int nTarget,
    int* indices
)
{
    int i, j;
    float max_val, current_val;
    
    /* determine which 'grid_dirs' indices are the closest to 'target_dirs' */
    for(i=0; i<nTarget; i++){
        max_val = -2.23e10f;
        for(j=0; j<nDirs; j++){
            current_val = grid_dirs_xyz[j*3] * target_dirs_xyz[i*3] +
                          grid_dirs_xyz[j*3+1] * target_dirs_xyz[i*3+1] +
                          grid_dirs_xyz[j*3+2] * target_dirs_xyz[i*3+2];
            if(current_val > max_val)
                indices[i] = j;
            if(current_val>max_val){
                indices[i] = j;
                max_val = current_val;
            }
        }
    }
}

typedef struct _array2binauralMagLS_data {
    float_complex** invAA_H;
    float_complex *M, *H_mod, *H_mod_gains;
    float_complex *AW, *AWHH;
    
}array2binauralMagLS_data;

void proposed_array2binauralMagLS_create
(
    void** phA2B,
    float_complex* ATFs,
    int nBands,
    int nMics,
    int nDirs
)
{
    *phA2B = malloc1d(sizeof(array2binauralMagLS_data));
    array2binauralMagLS_data *h = (array2binauralMagLS_data*)(*phA2B);
    int i, band;
    float_complex AAH[PROPOSED_MAX_NMICS*PROPOSED_MAX_NMICS];
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f);
    
    h->AWHH = malloc1d(nMics*nMics*sizeof(float_complex));
    h->AW = malloc1d(nMics*nDirs*sizeof(float_complex));
    h->M = malloc1d(NUM_EARS*nMics*sizeof(float_complex));
    h->H_mod = malloc1d(NUM_EARS*nDirs*sizeof(float_complex));
    h->H_mod_gains = malloc1d(NUM_EARS*nDirs*sizeof(float_complex));
    
    /* Precompute the inverse A*A^H matrices */
    h->invAA_H = (float_complex**)malloc2d(nBands, nMics*nMics, sizeof(float_complex));
    for (band=0; band<nBands; band++){
        cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, nMics, nMics, nDirs, &calpha,
                    &ATFs[band*nMics*nDirs], nDirs,
                    &ATFs[band*nMics*nDirs], nDirs, &cbeta,
                    AAH, nMics);
        for(i=0; i<nMics; i++)
            AAH[i*nMics+i] = craddf(AAH[i*nMics+i], 0.0001f);
        utility_cinv(NULL, AAH, h->invAA_H[band], nMics);
    }
}

void proposed_array2binauralMagLS_destroy
(
    void ** const phA2B
)
{
    array2binauralMagLS_data *h = (array2binauralMagLS_data*)(*phA2B);
    
    if(h!=NULL){
        free(h->invAA_H);
        free(h->AWHH);
        free(h->AW);
        free(h->M);
        free(h->H_mod);
        free(h->H_mod_gains);
        h = NULL;
    }
}

/*
 * Adapted from the getBinDecoder_MAGLS() function found here (ISC license):
 * https://github.com/leomccormack/Spatial_Audio_Framework/blob/master/framework/modules/saf_hoa/saf_hoa_internal.c
 */
void proposed_array2binauralMagLS
(
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
    float_complex* M_array2bin
)
{
    array2binauralMagLS_data *h = (array2binauralMagLS_data*)(hA2B);
    int band, i, j;
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f);
      
    /* Calculate mixing matrix per band */
    for (band=0; band<nBands; band++){
        if(centre_freqs[band]<=maxFreq_Hz){
            if(centre_freqs[band]<=magLScutoff_Hz){
                cblas_ccopy(NUM_EARS*nDirs, &hrtf_interp[band*NUM_EARS*nDirs], 1, h->H_mod, 1);
                for(i=0; i<nDirs; i++){
                    ((float*)(&h->H_mod[0*nDirs+i]))[0] *= weights[i];
                    ((float*)(&h->H_mod[0*nDirs+i]))[1] *= weights[i];
                    ((float*)(&h->H_mod[1*nDirs+i]))[0] *= weights[i];
                    ((float*)(&h->H_mod[1*nDirs+i]))[1] *= weights[i];
                }
//                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, nMics, nDirs, nDirs, &calpha,
//                            &ATFs[band*nMics*nDirs], nDirs,
//                            W_gains, nDirs, &cbeta,
//                            h->AW, nDirs);
//
                
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, nMics, NUM_EARS, nDirs, &calpha,
                            &ATFs[band*nMics*nDirs], nDirs, //h->AW, nDirs,
                            h->H_mod, nDirs, &cbeta, //&hrtf_interp[band*NUM_EARS*nDirs], nDirs, &cbeta,
                            h->AWHH, NUM_EARS);
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, nMics, NUM_EARS, nMics, &calpha,
                            h->invAA_H[band], nMics,
                            h->AWHH, NUM_EARS, &cbeta,
                            h->M, NUM_EARS);
            }
            else{
                /* Remove itd from high frequency HRTFs */
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, nDirs, nMics, &calpha,
                            &M_array2bin[(band-1)*NUM_EARS*nMics] , nMics,
                            &ATFs[band*nMics*nDirs], nDirs, &cbeta,
                            h->H_mod, nDirs);
                for(i=0; i<NUM_EARS*nDirs; i++)
                    h->H_mod[i] = ccmulf(cmplxf(cabsf(hrtf_interp[band*NUM_EARS*nDirs + i]), 0.0f), cexpf(cmplxf(0.0f, atan2f(cimagf(h->H_mod[i]), crealf(h->H_mod[i])))));
                
                for(i=0; i<nDirs; i++){
                    ((float*)(&h->H_mod[0*nDirs+i]))[0] *= weights[i];
                    ((float*)(&h->H_mod[0*nDirs+i]))[1] *= weights[i];
                    ((float*)(&h->H_mod[1*nDirs+i]))[0] *= weights[i];
                    ((float*)(&h->H_mod[1*nDirs+i]))[1] *= weights[i];
                }
                
//                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, nMics, nDirs, nDirs, &calpha,
//                            &ATFs[band*nMics*nDirs], nDirs,
//                            W_gains, nDirs, &cbeta,
//                            h->AW, nDirs);
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, nMics, NUM_EARS, nDirs, &calpha,
                            &ATFs[band*nMics*nDirs], nDirs, //h->AW, nDirs,
                            h->H_mod, nDirs, &cbeta,
                            h->AWHH, NUM_EARS);
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, nMics, NUM_EARS, nMics, &calpha,
                            h->invAA_H[band], nMics,
                            h->AWHH, NUM_EARS, &cbeta,
                            h->M, NUM_EARS);
            }
            
            for(i=0; i<nMics; i++)
                for(j=0; j<NUM_EARS; j++)
                    M_array2bin[band*NUM_EARS*nMics + j*nMics + i] = conjf(h->M[i*NUM_EARS+j]); /* ^H */
        }
    } 
}

void proposed_getInterpolatedHRTFs
(
    proposed_analysis_handle const hAna,
    PROPOSED_HRTF_INTERP_OPTIONS interpOption,
    proposed_binaural_config* binConfig,
    float* target_dirs_deg,
    int nTargetDirs,
    int ENABLE_DIFF_EQ,
    float_complex* hrtf_interp
)
{
    proposed_analysis_data *a = (proposed_analysis_data*)(hAna);
    int band, i, j, ntable, ntri;
    int* idx;
    float* itds_s, *interpTable, *w;
    float_complex*** hrtf_fb;    /* nBands x NUM_EARS x N_dirs */

    /* Pass HRIRs through the filterbank */
    hrtf_fb = (float_complex***)malloc3d(a->nBands, NUM_EARS, binConfig->nHRIR, sizeof(float_complex));
    HRIRs2HRTFs_afSTFT(binConfig->hrirs, binConfig->nHRIR, binConfig->lHRIR, a->hopsize, 1, 0, FLATTEN3D(hrtf_fb));

    /* Integration weights */
    if (cblas_sasum(nTargetDirs, target_dirs_deg+1, 2)/(float)nTargetDirs<0.0001)
        w = NULL;
    else{
        if(nTargetDirs<1200){
            w = malloc1d(nTargetDirs*sizeof(float));
            getVoronoiWeights(target_dirs_deg, nTargetDirs, 0, w);
        }
        else
            w=NULL;
    }

    /* estimate the ITDs for each HRIR */
    itds_s = malloc1d(binConfig->nHRIR*sizeof(float));
    estimateITDs(binConfig->hrirs, binConfig->nHRIR, binConfig->lHRIR, binConfig->hrir_fs, itds_s);

    /* Apply HRTF interpolation */
    switch(interpOption){
        case PROPOSED_HRTF_INTERP_NEAREST:
            /* Quantise to nearest hrir direction */
            idx = malloc1d(nTargetDirs*sizeof(int));
            findClosestGridPoints(binConfig->hrir_dirs_deg, binConfig->nHRIR, target_dirs_deg, nTargetDirs, 1, idx, NULL, NULL);
            for(band=0; band<a->nBands; band++)
                for(i=0; i<NUM_EARS; i++)
                    for(j=0; j<nTargetDirs; j++)
                        hrtf_interp[band*NUM_EARS*nTargetDirs + i*nTargetDirs + j] = hrtf_fb[band][i][idx[j]];

            /* Diffuse-field EQ without phase-simplification */
            diffuseFieldEqualiseHRTFs(nTargetDirs, itds_s, a->freqVector, a->nBands, w, ENABLE_DIFF_EQ, 0, hrtf_interp);
            free(idx);
            break;

        case PROPOSED_HRTF_INTERP_TRIANGULAR:
            /* Interpolation table */
            interpTable = NULL;
            generateVBAPgainTable3D_srcs(target_dirs_deg, nTargetDirs, binConfig->hrir_dirs_deg, binConfig->nHRIR, 0, 0, 0.0f, &interpTable, &ntable, &ntri);
            VBAPgainTable2InterpTable(interpTable, nTargetDirs, binConfig->nHRIR);

            /* Interpolate */
            interpHRTFs(FLATTEN3D(hrtf_fb), itds_s, a->freqVector, interpTable, binConfig->nHRIR, a->nBands, nTargetDirs, hrtf_interp);
            
            /* Diffuse-field EQ without phase-simplification */
            diffuseFieldEqualiseHRTFs(nTargetDirs, itds_s, a->freqVector, a->nBands, w, ENABLE_DIFF_EQ, 0, hrtf_interp);

            /* Clean-up */
            free(interpTable);
            break;
    }

    /* Clean-up */
    free(itds_s);
    free(hrtf_fb);
    free(w);
}

/** Internal data structure for sdMUSIC */
typedef struct _proposed_sdMUSIC_data {
    int nMics, nDirs;
    float_complex* VnA;
    float* grid_dirs_xyz;
    float* abs_VnA;
    float* pSpec;
    float* pSpecInv;
    float* P_minus_peak;
    float* VM_mask;

}proposed_sdMUSIC_data;

void proposed_sdMUSIC_create
(
    void ** const phMUSIC,
    int nMics,
    float* grid_dirs_deg,
    int nDirs
)
{
    *phMUSIC = malloc1d(sizeof(proposed_sdMUSIC_data));
    proposed_sdMUSIC_data *h = (proposed_sdMUSIC_data*)(*phMUSIC);

    h->nMics = nMics;
    h->nDirs = nDirs;

    /* store cartesian coords of scanning directions (for optional peak finding) */
    h->grid_dirs_xyz = malloc1d(h->nDirs * 3 * sizeof(float));
    unitSph2cart(grid_dirs_deg, h->nDirs, 1, h->grid_dirs_xyz);

    /* for run-time */
    h->VnA = malloc1d(h->nMics * (h->nDirs) * sizeof(float_complex));
    h->abs_VnA = malloc1d(h->nMics * (h->nDirs) * sizeof(float));
    h->pSpec = malloc1d(h->nDirs*sizeof(float));
    h->pSpecInv = malloc1d(h->nDirs*sizeof(float));
    h->P_minus_peak = malloc1d(h->nDirs*sizeof(float));
    h->VM_mask = malloc1d(h->nDirs*sizeof(float));
}

void proposed_sdMUSIC_destroy
(
    void ** const phMUSIC
)
{
    proposed_sdMUSIC_data *h = (proposed_sdMUSIC_data*)(*phMUSIC);

    if (h != NULL) {
        free(h->grid_dirs_xyz);
        free(h->VnA);
        free(h->abs_VnA);
        free(h->pSpec);
        free(h->pSpecInv);
        free(h->P_minus_peak);
        free(h->VM_mask);
        free(h);
        h = NULL;
        *phMUSIC = NULL;
    }
}

void proposed_sdMUSIC_compute
(
    void* const hMUSIC,
    float_complex* A_grid, /* nMics x nDirs */
    float_complex *Vn, /* nMics x (nMics - nSrcs) */
    int nSrcs,
    float* P_music,
    int* peak_inds
)
{
    proposed_sdMUSIC_data *h = (proposed_sdMUSIC_data*)(hMUSIC);
    int i, k, VnD2, peak_idx;
    float kappa, scale;
    float VM_mean[3];
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f);

    VnD2 = h->nMics - nSrcs; /* noise subspace second dimension length */

    /* derive the pseudo-spectrum value for each grid direction */
    cblas_cgemm(CblasRowMajor, CblasConjTrans, CblasNoTrans, h->nDirs, VnD2, h->nMics, &calpha,
                A_grid, h->nDirs,
                Vn, VnD2, &cbeta,
                h->VnA, VnD2);
    utility_cvabs(h->VnA, (h->nDirs)*VnD2, h->abs_VnA);
    for (i = 0; i < (h->nDirs); i++)
        h->pSpecInv[i] = cblas_sdot(VnD2, &(h->abs_VnA[i*VnD2]), 1, &(h->abs_VnA[i*VnD2]), 1);
    //h->pSpec[i] = 1.0f / (h->pSpecInv[i] + 2.23e-10f);
    utility_svrecip(h->pSpecInv, h->nDirs, h->pSpec);

    /* Output pseudo-spectrum */
    if(P_music!=NULL)
        cblas_scopy(h->nDirs, h->pSpec, 1, P_music, 1);

    /* Peak-finding */
    if(peak_inds!=NULL){
        kappa = 50.0f;
        scale = kappa/(2.0f*SAF_PI*expf(kappa)-expf(-kappa));
        cblas_scopy(h->nDirs, h->pSpec, 1, h->P_minus_peak, 1);

        /* Loop over the number of sources */
        for(k=0; k<nSrcs; k++){
            utility_simaxv(h->P_minus_peak, h->nDirs, &peak_idx);
            peak_inds[k] = peak_idx;
            if(k==nSrcs-1)
                break;
            VM_mean[0] = h->grid_dirs_xyz[peak_idx*3];
            VM_mean[1] = h->grid_dirs_xyz[peak_idx*3+1];
            VM_mean[2] = h->grid_dirs_xyz[peak_idx*3+2];

            /* Apply mask for next iteration */
            cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, h->nDirs, 1, 3, 1.0f,
                        h->grid_dirs_xyz, 3,
                        (const float*)VM_mean, 3, 0.0f,
                        h->VM_mask, 1);
            cblas_sscal(h->nDirs, kappa, h->VM_mask, 1);
            for(i=0; i<h->nDirs; i++)
                h->VM_mask[i] = expf(h->VM_mask[i]);             /* VM distribution */
            cblas_sscal(h->nDirs, scale, h->VM_mask, 1);
            for(i=0; i<h->nDirs; i++)
                h->VM_mask[i] = 1.0f/(0.00001f+(h->VM_mask[i])); /* inverse VM distribution */
            utility_svvmul(h->P_minus_peak, h->VM_mask, h->nDirs, h->pSpec);
            cblas_scopy(h->nDirs, h->pSpec, 1, h->P_minus_peak, 1);
        }
    }
}
//
//void sphPWD_create
//(
//    void ** const phPWD,
//    int order,
//    float* grid_dirs_deg,
//    int nDirs
//)
//{
//    *phPWD = malloc1d(sizeof(sphPWD_data));
//    sphPWD_data *h = (sphPWD_data*)(*phPWD);
//    int i, j;
//    float** grid_dirs_rad, **grid_svecs_tmp;
//
//    h->order = order;
//    h->nSH = ORDER2NSH(h->order);
//    h->nDirs = nDirs;
//
//    /* steering vectors for each grid direction  */
//    h->grid_svecs = malloc1d(h->nSH * (h->nDirs) * sizeof(float_complex));
//    grid_dirs_rad  = (float**)malloc2d(h->nDirs, 2, sizeof(float));
//    grid_svecs_tmp = (float**)malloc2d(h->nSH, h->nDirs, sizeof(float));
//    for(i=0; i<h->nDirs; i++){
//        grid_dirs_rad[i][0] = grid_dirs_deg[i*2]*SAF_PI/180.0f;
//        grid_dirs_rad[i][1] = SAF_PI/2.0f - grid_dirs_deg[i*2+1]*SAF_PI/180.0f;
//    }
//    getSHreal(h->order, FLATTEN2D(grid_dirs_rad), h->nDirs, FLATTEN2D(grid_svecs_tmp));
//    for(i=0; i<h->nSH; i++)
//        for(j=0; j<h->nDirs; j++)
//            h->grid_svecs[j*(h->nSH)+i] = cmplxf(grid_svecs_tmp[i][j], 0.0f);
//
//    /* store cartesian coords of scanning directions (for optional peak finding) */
//    h->grid_dirs_xyz = malloc1d(h->nDirs * 3 * sizeof(float));
//    unitSph2cart(grid_dirs_deg, h->nDirs, 1, h->grid_dirs_xyz);
//
//    /* for run-time */
//    h->A_Cx = malloc1d((h->nSH) * sizeof(float_complex));
//    h->pSpec = malloc1d(h->nDirs*sizeof(float));
//    h->P_minus_peak = malloc1d(h->nDirs*sizeof(float));
//    h->VM_mask = malloc1d(h->nDirs*sizeof(float));
//    h->P_tmp = malloc1d(h->nDirs*sizeof(float));
//
//    /* clean-up */
//    free(grid_dirs_rad);
//    free(grid_svecs_tmp);
//}
//
//void sphPWD_destroy
//(
//    void ** const phPWD
//)
//{
//    sphPWD_data *h = (sphPWD_data*)(*phPWD);
//
//    if (h != NULL) {
//        free(h->grid_dirs_xyz);
//        free(h->grid_svecs);
//        free(h->A_Cx);
//        free(h->pSpec);
//        free(h->P_minus_peak);
//        free(h->P_tmp);
//        free(h->VM_mask);
//        free(h);
//        h = NULL;
//        *phPWD = NULL;
//    }
//}
//
//void sphPWD_compute
//(
//    void* const hPWD,
//    float_complex* Cx,
//    int nSrcs,
//    float* P_map,
//    int* peak_inds
//)
//{
//    sphPWD_data *h = (sphPWD_data*)(hPWD);
//    int i, k, peak_idx;
//    float kappa, scale;
//    float VM_mean[3];
//    float_complex A_Cx_A;
//    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f);
//
//    /* derive the power-map value for each grid direction */
//    for (i = 0; i < (h->nDirs); i++){
//        cblas_cgemv(CblasRowMajor, CblasNoTrans, h->nSH, h->nSH, &calpha,
//                    Cx, h->nSH,
//                    &(h->grid_svecs[i*(h->nSH)]), 1, &cbeta,
//                    h->A_Cx, 1);
//        cblas_cdotu_sub(h->nSH, h->A_Cx, 1, &(h->grid_svecs[i*(h->nSH)]), 1, &A_Cx_A);
//        h->pSpec[i] = crealf(A_Cx_A);
//    }
//
//    /* Output power-map */
//    if(P_map!=NULL)
//        cblas_scopy(h->nDirs, h->pSpec, 1, P_map, 1);
//
//    /* Peak-finding */
//    if(peak_inds!=NULL){
//        kappa = 50.0f;
//        scale = kappa/(2.0f*SAF_PI*expf(kappa)-expf(-kappa));
//        cblas_scopy(h->nDirs, h->pSpec, 1, h->P_minus_peak, 1);
//
//        /* Loop over the number of sources */
//        for(k=0; k<nSrcs; k++){
//            utility_simaxv(h->P_minus_peak, h->nDirs, &peak_idx);
//            peak_inds[k] = peak_idx;
//            if(k==nSrcs-1)
//                break;
//            VM_mean[0] = h->grid_dirs_xyz[peak_idx*3];
//            VM_mean[1] = h->grid_dirs_xyz[peak_idx*3+1];
//            VM_mean[2] = h->grid_dirs_xyz[peak_idx*3+2];
//
//            /* Apply mask for next iteration */
//            cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, h->nDirs, 1, 3, 1.0f,
//                        h->grid_dirs_xyz, 3,
//                        (const float*)VM_mean, 3, 0.0f,
//                        h->VM_mask, 1);
//            cblas_sscal(h->nDirs, kappa, h->VM_mask, 1);
//            for(i=0; i<h->nDirs; i++)
//                h->VM_mask[i] = expf(h->VM_mask[i]);             /* VM distribution */
//            cblas_sscal(h->nDirs, scale, h->VM_mask, 1);
//            for(i=0; i<h->nDirs; i++)
//                h->VM_mask[i] = 1.0f/(0.00001f+(h->VM_mask[i])); /* inverse VM distribution */
//            utility_svvmul(h->P_minus_peak, h->VM_mask, h->nDirs, h->P_tmp);
//            cblas_scopy(h->nDirs, h->P_tmp, 1, h->P_minus_peak, 1);
//        }
//    }
//}

float proposed_comedie
(
    float* lambda,
    int N
)
{
    int i;
    float Nord, sum, g_0, mean_ev, g, sumAbsDiff;

    Nord = sqrtf((float)N)-1.0f;
    sum = 0.0f;
    for(i=0; i<N; i++)
        sum += lambda[i];
    if(sum < 0.0001f) /* FLT_EPS*FLT_MIN -/+ range */
        return 1.0f;
    else{
        g_0 = 2.0f*(powf(Nord+1.0f,2.0f)-1.0f);
        mean_ev = (1.0f/(powf(Nord+1.0f,2.0f)))*sum;
        sumAbsDiff = 0.0f;
        for(i=0; i<N; i++)
            sumAbsDiff += fabsf(lambda[i]-mean_ev);
        g = (1.0f/mean_ev)*sumAbsDiff;
        /* due to numerical error small (10e-7) negative numbers were occuring
         * sometimes for the single plane-wave case; hence bounding it to >=0 */
        return SAF_MAX(1.0f-g/g_0, 0.0f);
    }
}
