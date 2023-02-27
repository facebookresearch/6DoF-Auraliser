/**
 * @file proposed_analysis.c
 * @ingroup PROPOSED
 * @brief Source for the proposed analysis
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

#include "proposed_analysis.h"
#include "proposed_internal.h"

/* ========================================================================== */
/*                            PROPOSED Analysis                               */
/* ========================================================================== */

void proposed_analysis_create
(
    proposed_analysis_handle* const phAna,
    float fs,
    int hopsize,
    int blocksize,
    float* h_array,
    float* array_dirs_deg,
    int nDirs,
    int nMics,
    int h_len
)
{
    proposed_analysis_data* a = (proposed_analysis_data*)malloc1d(sizeof(proposed_analysis_data));
    *phAna = (void*)a;
    int band, i, j, idx_max;
    float* w_tmp;
    float_complex *U, *E, *H_W;
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f); /* blas */

    nMics = SAF_MIN(nMics, PROPOSED_MAX_NMICS);
    assert(blocksize % hopsize == 0); /* Must be a multiple of hopsize */
    assert(blocksize<=PROPOSED_MAX_BLOCKSIZE);

    /* User parameters */ 
    a->fs = fs;
    a->hopsize = hopsize;
    a->blocksize = blocksize;
    a->h_array = malloc1d(nDirs*nMics*h_len*sizeof(float));
    memcpy(a->h_array, h_array, nDirs*nMics*h_len*sizeof(float));
    a->array_dirs_deg = malloc1d(nDirs*2*sizeof(float));
    memcpy(a->array_dirs_deg, array_dirs_deg, nDirs*2*sizeof(float));
    a->array_dirs_xyz = malloc1d(nDirs*3*sizeof(float));
    unitSph2cart(a->array_dirs_deg, nDirs, 1, a->array_dirs_xyz);
    a->nDirs = nDirs;
    a->nMics = nMics;
    a->h_len = h_len;
    a->covAvgCoeff = 0.3f;
    a->covAvgCoeff = SAF_CLAMP(a->covAvgCoeff, 0.0f, 0.99999f);
    a->maximumAnalysisFreq = 9e3f;
     
    /* Scale steering vectors so that the peak of loudest measurement is 1 */
    utility_simaxv(a->h_array, nDirs*nMics*h_len, &idx_max);
    cblas_sscal(nDirs*nMics*h_len, 1.0f/a->h_array[idx_max], a->h_array, 1);
    
    /* For optional plotting purposes  */
    a->grid_histogram = calloc1d(a->nDirs, sizeof(float));

    /* Initialise time-frequency transform  */
    a->timeSlots = a->blocksize/a->hopsize;
    afSTFT_create(&(a->hFB_enc), a->nMics, 0, a->hopsize, 1, 0, AFSTFT_BANDS_CH_TIME);
    a->nBands = afSTFT_getNBands(a->hFB_enc);
    a->freqVector = malloc1d(a->nBands*sizeof(float));
    a->filterbankDelay = afSTFT_getProcDelay(a->hFB_enc);
    afSTFT_getCentreFreqs(a->hFB_enc, a->fs, a->nBands, a->freqVector);
    a->H_array = malloc1d(a->nBands*(a->nMics)*(a->nDirs)*sizeof(float_complex));
    a->H_array_w = malloc1d(a->nBands*(a->nMics)*(a->nDirs)*sizeof(float_complex));
    afSTFT_FIRtoFilterbankCoeffs(a->h_array, a->nDirs, a->nMics, a->h_len, a->hopsize, 1, 0, a->H_array);
   
    /* Initialise DoA estimator */
    utility_cseig_create(&(a->hEig), a->nMics);
    a->nScan = 0;
    a->scan_dirs_deg = malloc1d(nDirs*2*sizeof(float));
    a->scan_dirs_xyz = malloc1d(nDirs*3*sizeof(float));
    for(i=0; i<nDirs; i++){
        if(a->array_dirs_deg[i*2+1]>-PROPOSED_ELEV_SCANNING_WINDOW_DEG && a->array_dirs_deg[i*2+1]<PROPOSED_ELEV_SCANNING_WINDOW_DEG){
            a->scan_dirs_deg[a->nScan*2] = a->array_dirs_deg[i*2];
            a->scan_dirs_deg[a->nScan*2+1] = a->array_dirs_deg[i*2+1];
            a->nScan++;
        }
    }
    unitSph2cart(a->scan_dirs_deg, a->nScan, 1, a->scan_dirs_xyz);
    a->scan_idx = malloc1d(a->nScan*sizeof(int));
#if PROPOSED_USE_MUSIC
    proposed_sdMUSIC_create(&(a->hDoA), a->nMics, a->scan_dirs_deg, a->nScan);
#else
//TODO: implement
  //  sphPWD_create
#endif
    proposed_findNearestGridIndices(a->array_dirs_xyz, a->scan_dirs_xyz, a->nDirs, a->nScan, a->scan_idx);
 
    /* Integration weights */
    a->W = calloc1d(a->nDirs*a->nDirs,sizeof(float_complex));
    if (cblas_sasum(a->nDirs, a->array_dirs_deg+1, 2)/(float)a->nDirs<0.001){
        for(i=0; i<a->nDirs; i++)
            a->W[i*(a->nDirs)+i] = cmplxf(1.0f, 0.0f);
    }
    else{
        w_tmp = malloc1d(a->nDirs*sizeof(float));
        if(a->nDirs<1200)
            getVoronoiWeights(a->array_dirs_deg, a->nDirs, 0, w_tmp);
        for(i=0; i<a->nDirs; i++)
            a->W[i*(a->nDirs)+i] = cmplxf(a->nDirs<1200 ? w_tmp[i] : 1.0f, 0.0f);
    }

    /* Compute diffuse coherence matrices */
    a->T = (float_complex**)malloc2d(a->nBands, a->nMics*(a->nMics), sizeof(float_complex));
    a->DCM_array = malloc1d(a->nBands*(a->nMics)*(a->nMics)*sizeof(float_complex));
    diffCohMtxMeas(a->H_array, a->nBands, a->nMics, a->nDirs, NULL, a->DCM_array);
    
    /* For spatial whitening of the spatial covariance matrix, such that it has an identity structure under diffuse-field conditions (see [2]) */
    a->H_scan_w = malloc1d(a->nBands*(a->nMics)*(a->nScan)*sizeof(float_complex));
    U = malloc1d(a->nMics*(a->nMics)*sizeof(float_complex));
    E = malloc1d(a->nMics*(a->nMics)*sizeof(float_complex));
    H_W = malloc1d(a->nMics*(a->nDirs)*sizeof(float_complex));
    for(band=0; band<a->nBands; band++){
        /* Diffuse covariance matrix */
        cblas_sscal(/*re+im*/2*(a->nMics)*(a->nMics), 1.0f/(float)a->nDirs, (float*)&(a->DCM_array[band*(a->nMics)*(a->nMics)]), 1);
         
        /* Decomposition of the diffuse covariance matrix */
        utility_cseig(a->hEig, &(a->DCM_array[band*(a->nMics)*(a->nMics)]), a->nMics, 1, U, E, NULL);

        /* Compute spatial whitening matrix */
        for(i=0; i<a->nMics; i++)
            E[i*a->nMics+i] = cmplxf(sqrtf(1.0f/(crealf(E[i*a->nMics+i])+2.23e-10f)), 0.0f);
        cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, a->nMics, a->nMics, a->nMics, &calpha,
                    E, a->nMics,
                    U, a->nMics, &cbeta,
                    a->T[band], a->nMics);

        /* Whiten the array steering vectors / anechoic acoustic transfer functions (ATFs) */
        cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, a->nMics, a->nDirs, a->nMics, &calpha,
                    a->T[band], a->nMics,
                    &(a->H_array[band*(a->nMics)*(a->nDirs)]), a->nDirs, &cbeta,
                    &(a->H_array_w[band*(a->nMics)*(a->nDirs)]), a->nDirs);
        
        /* Take subset */
        for(i=0; i<a->nMics; i++)
            for(j=0; j<a->nScan; j++)
                a->H_scan_w[band*(a->nMics)*(a->nScan) + i*(a->nScan) + j] = a->H_array_w[band*(a->nMics)*(a->nDirs) + i*(a->nDirs) + a->scan_idx[j]];
    }
    free(U);
    free(E);
    free(H_W);

    /* Run-time variables */
    a->inputBlock = (float**)malloc2d(a->nMics, a->blocksize, sizeof(float));
    a->Cx = malloc1d(a->nBands*sizeof(CxMic));
    a->V  = malloc1d((a->nMics)*(a->nMics)*sizeof(float_complex));
    a->Vn = malloc1d((a->nMics)*(a->nMics)*sizeof(float_complex));
    a->lambda = malloc1d((a->nMics)*sizeof(float));

    /* Flush run-time buffers with zeros */
    proposed_analysis_reset((*phAna));
}

void proposed_analysis_destroy
(
    proposed_analysis_handle* const phAna
)
{
    proposed_analysis_data *a = (proposed_analysis_data*)(*phAna);

    if (a != NULL) {
        free(a->h_array);
        free(a->H_array);
        free(a->H_array_w);
        free(a->DCM_array);
        free(a->W);
        free(a->T);
        free(a->array_dirs_xyz);
        free(a->array_dirs_deg);
        
        /* For optional plotting purposes  */
        free(a->grid_histogram);

        /* Destroy time-frequency transform  */
        afSTFT_destroy(&(a->hFB_enc));
        free(a->freqVector);

        /* Destroy DoA estimator */
        utility_cseig_destroy(&(a->hEig));
        proposed_sdMUSIC_destroy(&(a->hDoA));
        free(a->scan_dirs_deg);
        free(a->scan_dirs_xyz);
        free(a->H_scan_w);

        /* Free run-time variables */
        free(a->inputBlock);
        free(a->Cx);
        free(a->V);
        free(a->Vn);
        free(a->lambda);

        free(a);
        a = NULL;
        (*phAna) = NULL;
    }
}

void proposed_analysis_reset
(
    proposed_analysis_handle const hAna
)
{
    proposed_analysis_data *a;
    int band;
    if(hAna==NULL)
        return;
    a = (proposed_analysis_data*)(hAna);

    for(band=0; band<a->nBands; band++)
        memset(a->Cx[band].Cx, 0, PROPOSED_MAX_NMICS*PROPOSED_MAX_NMICS*sizeof(float_complex));
    
    /* For optional plotting */
    memset(a->grid_histogram, 0, a->nDirs*sizeof(float));
}

void proposed_analysis_apply
(
    proposed_analysis_handle const hAna,
    float** input,
    int nChannels,
    int blocksize,
    void* const hPCon,
    void* const hSCon
)
{
    proposed_analysis_data *a = (proposed_analysis_data*)(hAna);
    proposed_param_container_data *pcon = (proposed_param_container_data*)(hPCon);
    proposed_signal_container_data *scon = (proposed_signal_container_data*)(hSCon);
    int i, j, k, ch, band, K;
    int est_idx[PROPOSED_MAX_NMICS];
    float diffuseness;
    CxMic Cx_new, T_Cx, T_Cx_TH;
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f); /* blas */

    assert(blocksize==a->blocksize);

    /* Load time-domain data */
    for(ch=0; ch<SAF_MIN(nChannels, a->nMics); ch++)
        cblas_scopy(blocksize, input[ch], 1, a->inputBlock[ch], 1);
    for(; ch<a->nMics; ch++)
        memset(a->inputBlock[ch], 0, blocksize*sizeof(float));

    /* Forward time-frequency transform */
    afSTFT_forward_knownDimensions(a->hFB_enc, a->inputBlock, blocksize, a->nMics, a->timeSlots, scon->inTF); 

    /* Update covariance matrix per band */
    for(band=0; band<a->nBands; band++){
        cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, a->nMics, a->nMics, a->timeSlots, &calpha,
                    FLATTEN2D(scon->inTF[band]), a->timeSlots,
                    FLATTEN2D(scon->inTF[band]), a->timeSlots, &cbeta,
                    Cx_new.Cx, a->nMics);

        /* Make a copy for the signal container */
        cblas_ccopy(a->nMics*a->nMics, (float_complex*)Cx_new.Cx, 1, (float_complex*)scon->Cx[band].Cx, 1);

        /* Apply temporal averaging */
        cblas_sscal(/*re+im*/2*(a->nMics) * (a->nMics),      SAF_CLAMP(a->covAvgCoeff, 0.0f, 0.999f), (float*)a->Cx[band].Cx, 1);
        cblas_saxpy(/*re+im*/2*(a->nMics) * (a->nMics), 1.0f-SAF_CLAMP(a->covAvgCoeff, 0.0f, 0.999f), (float*)Cx_new.Cx, 1, (float*)a->Cx[band].Cx, 1);
    }

    /* Spatial parameter estimation per band */
    for (band = 0; band < a->nBands; band++) {
        if (a->freqVector[band]<a->maximumAnalysisFreq && a->freqVector[band]<PROPOSED_MAX_RENDERING_FREQ){
            /* Apply diffuse whitening process */
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, a->nMics, a->nMics, a->nMics, &calpha,
                        a->T[band], a->nMics,
                        a->Cx[band].Cx, a->nMics, &cbeta,
                        T_Cx.Cx, a->nMics);
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, a->nMics, a->nMics, a->nMics, &calpha,
                        T_Cx.Cx, a->nMics,
                        a->T[band], a->nMics, &cbeta,
                        T_Cx_TH.Cx, a->nMics);
            utility_cseig(a->hEig, T_Cx_TH.Cx, a->nMics, 1, a->V, NULL, a->lambda);

            /* Detect number of sources */
            diffuseness = proposed_comedie(a->lambda, a->nMics);
            K = SAF_MIN(SAF_MIN((a->nMics-1)*diffuseness+1, (1.0f-diffuseness)*(a->nMics)), (int)((float)a->nMics/2.0f));
            //K = SAF_MIN((a->nMics-1)*diffuseness+1, (int)((float)a->nMics/2.0f));
            K = SAF_MAX(K, 1); /* forcing at least one */
            
            /* Store diffuseness and source number estimates */
            pcon->nSrcs[band] = SAF_MIN(K, PROPOSED_MAX_K);
            pcon->diffuseness[band] = diffuseness;
            
            if (K>0){
                /* Apply DoA estimator */
                for(i=0; i<a->nMics; i++)
                    for(j=0, k=K; j<a->nMics-K; j++, k++)
                        a->Vn[i*(a->nMics-K)+j] = a->V[i*(a->nMics)+k];
                proposed_sdMUSIC_compute(a->hDoA, &(a->H_scan_w[band*(a->nMics)*(a->nScan)]), a->Vn, K, NULL, (int*)est_idx);

                /* Store */
                for(j=0; j<pcon->nSrcs[band]; j++){
                    pcon->doa_idx[band][j] = pcon->gains_idx[band][j] = a->scan_idx[est_idx[j]];
                    pcon->src_gains[band][j] = 1.0f; /* Default gains per band */
                    
                    /* For optional plotting */
                    a->grid_histogram[a->scan_idx[est_idx[j]]] += 1.0f;
                }
            }
        }
        else {
            /* "residual" only rendering (but of course, not subtracting anything) */
            pcon->nSrcs[band] = 0;
            pcon->diffuseness[band] = 1.0f;
             
            /* Or we extrapolate the parameters somehow? */
            if (band-1>0){
                K = pcon->nSrcs[band-1];
                pcon->nSrcs[band] = K;
                for(j=0; j<K; j++){
                    pcon->doa_idx[band][j]   = pcon->doa_idx[band-1][j];
                    pcon->gains_idx[band][j] = pcon->gains_idx[band-1][j];
                    pcon->src_gains[band][j] = pcon->src_gains[band-1][j];
                }
            }
        }
    }
}

const float* proposed_analysis_getFrequencyVectorPtr
(
    proposed_analysis_handle const hAna,
    int* nBands
)
{
    proposed_analysis_data *a;
    if(hAna==NULL){
        if(nBands!=NULL)
            (*nBands) = 0;
        return NULL;
    }
    a = (proposed_analysis_data*)(hAna);
    if(nBands!=NULL)
       (*nBands) = a->nBands;
    return (const float*)a->freqVector;
}

const float* proposed_analysis_getGridDirsXYZPtr
(
    proposed_analysis_handle const hAna,
    int* nDirs
)
{
    proposed_analysis_data *a;
    if(hAna==NULL){
        if(nDirs!=NULL)
            (*nDirs) = 0;
        return NULL;
    }
    a = (proposed_analysis_data*)(hAna);
    if(nDirs!=NULL)
       (*nDirs) = a->nDirs;
    return (const float*)a->array_dirs_xyz;
}

const float* proposed_analysis_getHistogramPtr
(
    proposed_analysis_handle const hAna,
    int* nDirs
)
{
    proposed_analysis_data *a;
    int max_ind;
    if(hAna==NULL){
        if(nDirs!=NULL)
            (*nDirs) = 0;
        return NULL;
    }
    a = (proposed_analysis_data*)(hAna);
    if(nDirs!=NULL)
       (*nDirs) = a->nDirs;
    utility_simaxv(a->grid_histogram, a->nDirs, &max_ind);
    if(a->grid_histogram[max_ind]>0.001f)
        cblas_sscal(a->nDirs, 1.0f/a->grid_histogram[max_ind], a->grid_histogram, 1);
    return (const float*)a->grid_histogram;
}

int proposed_analysis_getNbands
(
    proposed_analysis_handle const hAna
)
{
    return hAna == NULL ? 0 : ((proposed_analysis_data*)(hAna))->nBands;
}

int proposed_analysis_getNDirs
(
    proposed_analysis_handle const hAna
)
{
    return hAna == NULL ? 0 : ((proposed_analysis_data*)(hAna))->nDirs;
}

float* proposed_analysis_getCovarianceAvagingCoeffPtr
(
    proposed_analysis_handle const hAna
)
{
    proposed_analysis_data *a;
    if(hAna==NULL)
        return NULL;
    a = (proposed_analysis_data*)(hAna);
    return &(a->covAvgCoeff);
}

float* proposed_analysis_getMaxAnalysisFreqPtr
(
    proposed_analysis_handle const hAna
)
{
    proposed_analysis_data *a;
    if(hAna==NULL)
        return NULL;
    a = (proposed_analysis_data*)(hAna);
    return &(a->maximumAnalysisFreq);
}

int proposed_analysis_getProcDelay
(
    proposed_analysis_handle const hAna
)
{
    return hAna == NULL ? 0 : ((proposed_analysis_data*)(hAna))->filterbankDelay;
}


/* ========================================================================== */
/*                      Parameter and Signal Containers                       */
/* ========================================================================== */

void proposed_param_container_create
(
    proposed_param_container_handle* const phPCon,
    proposed_analysis_handle const hAna
)
{
    proposed_param_container_data* pcon = (proposed_param_container_data*)malloc1d(sizeof(proposed_param_container_data));
    *phPCon = (void*)pcon;
    proposed_analysis_data *a = (proposed_analysis_data*)(hAna);

    /* Copy data that is relevant to the container */
    pcon->nBands = a->nBands;

    /* Allocate parameter storage */
    pcon->nSrcs = malloc1d(pcon->nBands*sizeof(int));
    pcon->diffuseness = malloc1d(pcon->nBands*sizeof(float));
    pcon->doa_idx = (int**)malloc2d(pcon->nBands, PROPOSED_MAX_K, sizeof(int));
    pcon->gains_idx = (int**)malloc2d(pcon->nBands, PROPOSED_MAX_K, sizeof(int));

    /* Allocate the optional parameter storage */
    pcon->src_gains = (float**)calloc2d(pcon->nBands, PROPOSED_MAX_K, sizeof(float));
}

void proposed_param_container_destroy
(
    proposed_param_container_handle* const phPCon
)
{
    proposed_param_container_data *pcon = (proposed_param_container_data*)(*phPCon);

    if (pcon != NULL) {
        /* Free parameter storage */
        free(pcon->nSrcs);
        free(pcon->diffuseness);
        free(pcon->doa_idx);
        free(pcon->gains_idx);
        free(pcon->src_gains);

        free(pcon);
        pcon = NULL;
        (*phPCon) = NULL;
    }
}

void proposed_signal_container_create
(
    proposed_signal_container_handle* const phSCon,
    proposed_analysis_handle const hAna
)
{
    proposed_signal_container_data* scon = (proposed_signal_container_data*)malloc1d(sizeof(proposed_signal_container_data));
    *phSCon = (void*)scon;
    proposed_analysis_data *a = (proposed_analysis_data*)(hAna);

    /* Copy data that is relevant to the container */
    scon->nMics = a->nMics;
    scon->nBands = a->nBands;
    scon->timeSlots = a->timeSlots;

    /* Copy of the NON-time-averaged covariance matrix per band */
    scon->Cx = malloc1d(a->nBands*sizeof(CxMic));

    /* Time-frequency frame */
    scon->inTF = (float_complex***)malloc3d(scon->nBands, scon->nMics, scon->timeSlots, sizeof(float_complex));
}

void proposed_signal_container_destroy
(
    proposed_signal_container_handle* const phSCon
)
{
    proposed_signal_container_data *scon = (proposed_signal_container_data*)(*phSCon);

    if (scon != NULL) {
        /* Free time-frequency frame */
        free(scon->Cx);
        free(scon->inTF);

        free(scon);
        scon = NULL;
        (*phSCon) = NULL;
    }
}
