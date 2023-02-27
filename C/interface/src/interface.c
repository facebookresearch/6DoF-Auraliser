/**
 * @file interface.c
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

void interface_create
(
    void ** const phInt
)
{
    interface_data* pData = (interface_data*)malloc1d(sizeof(interface_data));
    *phInt = (void*)pData;

    /* Default user parameters */
    pData->favour2Daccuracy = SAF_FALSE;
    pData->enableEPbeamformers = SAF_TRUE;
    pData->enableDiffEQ_HRTFs = SAF_TRUE;
    pData->enableDiffEQ_ATFs = SAF_FALSE;
    pData->renderingMode = CORE_6DOF;
    pData->sofa_filepath_MAIR = NULL;
    pData->useDefaultHRIRsFLAG = SAF_TRUE;
    pData->sofa_filepath_HRIR = NULL;
    pData->binConfig.lHRIR = pData->binConfig.nHRIR = pData->binConfig.hrir_fs = 0;
    pData->binConfig.hrirs = NULL;
    pData->binConfig.hrir_dirs_deg = NULL;
    pData->yaw = 0.0f;
    pData->pitch = 0.0f;
    pData->roll = 0.0f;
    pData->bFlipYaw = 0;
    pData->bFlipPitch = 0;
    pData->bFlipRoll = 0;
    pData->distMapOption = INTERFACE_DISTANCE_MAP_USE_PARAM;
    pData->sourceDistance = 1.85f;
    pData->enableSourceDirectivity = SAF_FALSE;
    pData->x = 0.0f;
    pData->y = 0.0f;
    pData->z = 0.0f;
    pData->bFlipX = 0;
    pData->bFlipY = 0;
    pData->bFlipZ = 0;

    /* internal parameters */
    pData->inputFrameTD =  (float**)malloc2d(INTERFACE_MAX_NUM_CHANNELS, FRAME_SIZE, sizeof(float));
    pData->outputFrameTD = (float**)malloc2d(NUM_EARS, FRAME_SIZE, sizeof(float));
    pData->fs = 48000.0f;
    pData->hAna = NULL;
    pData->hSyn = NULL;
    pData->hPCon = NULL;
    pData->hSCon = NULL;
    pData->head_orientation_xyz[0] = 1.0f; pData->head_orientation_xyz[1] = pData->head_orientation_xyz[2] = 0.0f;

    /* Local copy of internal parameter vectors (for optional thread-safe GUI plotting) */
    pData->nBands_local = 0;
    pData->nDirs_local = 0;
    pData->freqVector_local = NULL;
    pData->streamBalBands_local = NULL;
    pData->histogram_local = NULL;
    pData->grid_dirs_xyz_local = NULL;

    /* our core data */
    pData->progressBar0_1 = 0.0f;
    pData->progressBarText = malloc1d(INTERFACE_PROGRESSBARTEXT_CHAR_LENGTH*sizeof(char));
    strcpy(pData->progressBarText,"");

    /* Default IR data */
    pData->nMics = pData->nDirs = pData->IRlength = 0;
    pData->IR_fs = 0.0f;
    
    /* flags */
    pData->MAIR_SOFA_isLoadedFLAG = 0;
    pData->procStatus = PROC_STATUS_NOT_ONGOING;
    pData->coreStatus = CORE_STATUS_NOT_INITIALISED;

    /* Init core with defaults */
    interface_initCore(*phInt);
}

void interface_destroy
(
    void ** const phInt
)
{
    interface_data *pData = (interface_data*)(*phInt);

    if (pData != NULL) {
        /* not safe to free memory during intialisation/processing loop */
        while (pData->coreStatus == CORE_STATUS_INITIALISING ||
               pData->procStatus == PROC_STATUS_ONGOING){
            SAF_SLEEP(10);
        }
        free(pData->binConfig.hrirs);
        free(pData->binConfig.hrir_dirs_deg);
        proposed_analysis_destroy(&(pData->hAna));
        proposed_param_container_destroy(&(pData->hPCon));
        proposed_signal_container_destroy(&(pData->hSCon));
        proposed_synthesis_destroy(&(pData->hSyn));
        free(pData->progressBarText);
        free(pData->inputFrameTD);
        free(pData->outputFrameTD);
        free(pData->freqVector_local);
        free(pData->streamBalBands_local);
        free(pData->histogram_local);
        free(pData->grid_dirs_xyz_local);

        free(pData);
        pData = NULL;
        (*phInt) = NULL;
    }
}

void interface_init
(
    void * const hInt,
    int          sampleRate
)
{
    interface_data *pData = (interface_data*)(hInt);

    if(sampleRate!=(int)pData->fs){
        pData->fs = (float)sampleRate;
        interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
    }

    /* reset (flush internal buffers with zeros etc.) */
    if(pData->coreStatus == CORE_STATUS_INITIALISED){
        proposed_analysis_reset(pData->hAna);
        proposed_synthesis_reset(pData->hSyn);
    }
}

void interface_initCore
(
    void* const hInt
)
{
    interface_data *pData = (interface_data*)(hInt);
    int load_prevFLAG, nBands;
    float* eq, *streamBalance, *tmp;
    SAF_SOFA_ERROR_CODES error;
    saf_sofa_container sofa;
    float maxMagLSFreq, maxBSMFreq, maxAnalysisFreq;
    float* grid_dirs_deg;

    if (pData->coreStatus != CORE_STATUS_NOT_INITIALISED)
        return; /* re-init not required, or already happening */
    while (pData->procStatus == PROC_STATUS_ONGOING){
        /* re-init required, but we need to wait for the current processing loop to end */
        pData->coreStatus = CORE_STATUS_INITIALISING; /* indicate that we want to init */
        SAF_SLEEP(10);
    }
    
    /* for progress bar */
    pData->coreStatus = CORE_STATUS_INITIALISING;
    strcpy(pData->progressBarText,"Intialising Core");
    pData->progressBar0_1 = 0.0f;

    /* Local copy of internal settings (since they are overriden) */
    maxBSMFreq = maxMagLSFreq = maxAnalysisFreq = 0.0f;
    if(pData->hAna!=NULL && pData->hSyn!=NULL){
        load_prevFLAG = 1;
        maxBSMFreq = *proposed_synthesis_getMaxBSMFreqPtr(pData->hSyn);
        maxMagLSFreq = *proposed_synthesis_getMaxMagLSFreqPtr(pData->hSyn);
        maxAnalysisFreq = *proposed_analysis_getMaxAnalysisFreqPtr(pData->hAna);
        tmp = proposed_synthesis_getEqPtr(pData->hSyn, &nBands);
        eq = malloc1d(nBands*sizeof(float));
        memcpy(eq, tmp, nBands*sizeof(float));
        tmp = proposed_synthesis_getStreamBalancePtr(pData->hSyn, NULL);
        streamBalance = malloc1d(nBands*sizeof(float));
        memcpy(streamBalance, tmp, nBands*sizeof(float));
    }
    else{
        load_prevFLAG = 0;
        eq = streamBalance = NULL;
        nBands = -1;
    }
    
    /* Load SOFA file */
    error = saf_sofa_open(&sofa, pData->sofa_filepath_MAIR, SAF_SOFA_READER_OPTION_DEFAULT);
    if(error==SAF_SOFA_OK){
        pData->nDirs = sofa.nSources;
        pData->nMics = sofa.nReceivers;
        pData->IR_fs = sofa.DataSamplingRate;
        pData->IRlength = sofa.DataLengthIR;
        grid_dirs_deg = malloc1d(pData->nDirs*2*sizeof(float));
        cblas_scopy(pData->nDirs, sofa.SourcePosition, 3, grid_dirs_deg, 2); /* azi */
        cblas_scopy(pData->nDirs, &sofa.SourcePosition[1], 3, &grid_dirs_deg[1], 2); /* elev */
   
        /* Analysis */
        strcpy(pData->progressBarText,"Intialising Analysis");
        pData->progressBar0_1 = 0.3f;
        proposed_analysis_destroy(&(pData->hAna));
        proposed_analysis_create(&(pData->hAna), pData->fs, HOP_SIZE, FRAME_SIZE, sofa.DataIR, grid_dirs_deg, pData->nDirs, pData->nMics, sofa.DataLengthIR);
        free(grid_dirs_deg);

        /* Parameter/signal containers */
        strcpy(pData->progressBarText,"Intialising Containers");
        pData->progressBar0_1 = 0.5f;
        proposed_param_container_destroy(&(pData->hPCon));
        proposed_param_container_create(&(pData->hPCon), pData->hAna);
        proposed_signal_container_destroy(&(pData->hSCon));
        proposed_signal_container_create(&(pData->hSCon), pData->hAna);

        /* Synthesis */
        strcpy(pData->progressBarText,"Intialising Synthesis");
        pData->progressBar0_1 = 0.8f;
        saf_sofa_close(&sofa); /* Close previous */
        error = saf_sofa_open(&sofa, pData->sofa_filepath_HRIR, SAF_SOFA_READER_OPTION_DEFAULT);
        if(error==SAF_SOFA_OK){
            pData->binConfig.nHRIR = sofa.nSources;
            pData->binConfig.hrir_fs = sofa.DataSamplingRate;
            pData->binConfig.lHRIR = sofa.DataLengthIR;
            pData->binConfig.hrir_dirs_deg = realloc1d(pData->binConfig.hrir_dirs_deg, pData->binConfig.nHRIR*2*sizeof(float));
            cblas_scopy(pData->binConfig.nHRIR, sofa.SourcePosition, 3, pData->binConfig.hrir_dirs_deg, 2); /* azi */
            cblas_scopy(pData->binConfig.nHRIR, &sofa.SourcePosition[1], 3, &pData->binConfig.hrir_dirs_deg[1], 2); /* elev */
            pData->binConfig.hrirs = realloc1d(pData->binConfig.hrirs, pData->binConfig.nHRIR*NUM_EARS*pData->binConfig.lHRIR*sizeof(float));
            cblas_scopy(pData->binConfig.nHRIR*NUM_EARS*pData->binConfig.lHRIR, sofa.DataIR, 1, pData->binConfig.hrirs, 1);
            pData->useDefaultHRIRsFLAG = 0;
        }
        else{ /* Load default HRIRs: */
            pData->binConfig.hrirs = realloc1d(pData->binConfig.hrirs, __default_N_hrir_dirs*NUM_EARS*__default_hrir_len*sizeof(float));
            cblas_scopy(__default_N_hrir_dirs*NUM_EARS*__default_hrir_len, (float*)__default_hrirs, 1, pData->binConfig.hrirs, 1);
            pData->binConfig.hrir_dirs_deg = realloc1d(pData->binConfig.hrir_dirs_deg, __default_N_hrir_dirs*2*sizeof(float));
            cblas_scopy(__default_N_hrir_dirs*2, (float*)__default_hrir_dirs_deg, 1, pData->binConfig.hrir_dirs_deg, 1);
            pData->binConfig.nHRIR = __default_N_hrir_dirs;
            pData->binConfig.lHRIR = __default_hrir_len;
            pData->binConfig.hrir_fs = __default_hrir_fs;
            pData->useDefaultHRIRsFLAG = 1;
        }
        proposed_synthesis_destroy(&(pData->hSyn));
        proposed_synthesis_create(&(pData->hSyn), pData->hAna, &pData->binConfig, PROPOSED_HRTF_INTERP_NEAREST, pData->favour2Daccuracy, pData->enableEPbeamformers, pData->enableDiffEQ_HRTFs, pData->enableDiffEQ_ATFs);
 
        /* All went OK */
        pData->MAIR_SOFA_isLoadedFLAG = 1;
    }
    else /* Bypass audio, try to load a valid SOFA file instead: */
        pData->MAIR_SOFA_isLoadedFLAG = 0;
    saf_sofa_close(&sofa);

    /* Load previous internal settings (if not first init, and nBands is the same) */
    if(load_prevFLAG && (nBands==proposed_analysis_getNbands(pData->hAna))){
        tmp = proposed_synthesis_getEqPtr(pData->hSyn, &nBands);
        memcpy(tmp, eq, nBands*sizeof(float));
        tmp = proposed_synthesis_getStreamBalancePtr(pData->hSyn, NULL);
        memcpy(tmp, streamBalance, nBands*sizeof(float));
        
        *proposed_synthesis_getMaxBSMFreqPtr(pData->hSyn) = maxBSMFreq;
        *proposed_synthesis_getMaxMagLSFreqPtr(pData->hSyn) = maxMagLSFreq;
        *proposed_analysis_getMaxAnalysisFreqPtr(pData->hAna) = maxAnalysisFreq;
    }
 
    /* Local copy of internal parameter vectors (for optional thread-safe GUI plotting) */
    if(!load_prevFLAG || (proposed_analysis_getNbands(pData->hAna)!=pData->nBands_local) || (proposed_analysis_getNDirs(pData->hAna)!=pData->nDirs_local) ){
        /* If first init... Or nBands has changed */
        pData->nBands_local = proposed_analysis_getNbands(pData->hAna);
        pData->nDirs_local = proposed_analysis_getNDirs(pData->hAna);
        pData->freqVector_local = realloc1d(pData->freqVector_local, pData->nBands_local*sizeof(float));
        pData->streamBalBands_local = realloc1d(pData->streamBalBands_local, pData->nBands_local*sizeof(float));
        pData->histogram_local = realloc1d(pData->histogram_local, pData->nDirs_local*sizeof(float));
        pData->grid_dirs_xyz_local = realloc1d(pData->grid_dirs_xyz_local, pData->nDirs_local*3*sizeof(float));
    }
    tmp = (float*)proposed_analysis_getFrequencyVectorPtr(pData->hAna, NULL);
    memcpy(pData->freqVector_local, tmp, pData->nBands_local*sizeof(float));
    tmp = proposed_synthesis_getStreamBalancePtr(pData->hSyn, NULL);
    memcpy(pData->streamBalBands_local, tmp, pData->nBands_local*sizeof(float));

    /* done! */
    strcpy(pData->progressBarText,"Done!");
    pData->progressBar0_1 = 1.0f;
    pData->coreStatus = CORE_STATUS_INITIALISED;
    free(eq);
    free(streamBalance);
}

void interface_process
(
    void  *  const hInt,
    float ** const inputs,
    float ** const outputs,
    int            nInputs,
    int            nOutputs,
    int            nSamples
)
{
    interface_data *pData = (interface_data*)(hInt);
    int ch, nMics;
    float ypr_rad[3], xyz_m[3], Rzyx[3][3];
    const float forwards_xyz[3] = {1.0f, 0.0f, 0.0f};
    PROPOSED_DISTANCE_MAPS distMap;

    /* Local copies of parameters */
    nMics = pData->nMics;
    
    /* Process Frame if everything is ready */
    if ((nSamples == FRAME_SIZE) && (pData->coreStatus == CORE_STATUS_INITIALISED) && pData->MAIR_SOFA_isLoadedFLAG) {
        pData->procStatus = PROC_STATUS_ONGOING;

        /* Load time-domain data */
        for(ch=0; ch < SAF_MIN(nMics, nInputs); ch++)
            utility_svvcopy(inputs[ch], FRAME_SIZE, pData->inputFrameTD[ch]);
        for(; ch<nMics; ch++)
            memset(pData->inputFrameTD[ch], 0, FRAME_SIZE * sizeof(float)); /* fill remaining channels with zeros */

        /* Apply proposed analysis */
        proposed_analysis_apply(pData->hAna, pData->inputFrameTD, nMics, FRAME_SIZE, pData->hPCon, pData->hSCon);
 
        /* Listener head-orientation/rotation */
        switch (pData->renderingMode){
            default: /* fall through */
            case CORE_3DOF_TRANSLATIONS: /* fall through */
            case CORE_0DOF:              ypr_rad[0] = ypr_rad[1] = ypr_rad[2] = 0.0f; break;
            case CORE_1DOF_ROTATIONS:    ypr_rad[0] = pData->yaw; ypr_rad[1] = ypr_rad[2] = 0.0f; break;
            case CORE_6DOF:              /* fall through */
            case CORE_3DOF_ROTATIONS:    ypr_rad[0] = pData->yaw; ypr_rad[1] = pData->pitch;  ypr_rad[2] = pData->roll; break;
        }
        
        /* Head orientation based on selected degrees of freedom */
        euler2rotationMatrix(-ypr_rad[0], -ypr_rad[1], ypr_rad[2], 0, EULER_ROTATION_YAW_PITCH_ROLL, Rzyx);
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 1, 3, 3, 1.0f,
                    (const float*)forwards_xyz, 3,
                    (float*)Rzyx, 3, 0.0f,
                    (float*)pData->head_orientation_xyz, 3); 
                                         
        /* Listener position relative to origin */
        switch (pData->renderingMode){
            default: /* fall through */
            case CORE_0DOF:              /* fall through */
            case CORE_1DOF_ROTATIONS:    /* fall through */
            case CORE_3DOF_ROTATIONS:    xyz_m[0] = xyz_m[1] = xyz_m[2] = 0.0f; break;
            case CORE_3DOF_TRANSLATIONS: /* fall through */
            case CORE_6DOF:              xyz_m[0] = pData->x; xyz_m[1] = pData->y; xyz_m[2] = pData->z; break;
        }
        
        /* distance map option */
        switch(pData->distMapOption){
            case INTERFACE_DISTANCE_MAP_USE_PARAM: distMap = PROPOSED_DISTANCE_MAP_USE_PARAM; break;
            case INTERFACE_DISTANCE_MAP_1SRC:      distMap = PROPOSED_DISTANCE_MAP_1SRC; break;
            case INTERFACE_DISTANCE_MAP_2SRC:      distMap = PROPOSED_DISTANCE_MAP_2SRC; break;
            case INTERFACE_DISTANCE_MAP_3SRC:      distMap = PROPOSED_DISTANCE_MAP_3SRC; break; 
        }
         
        /* Apply proposed synthesis */
        proposed_synthesis_apply(pData->hSyn, pData->hPCon, pData->hSCon,
                                 (float*)ypr_rad, (float*)xyz_m, distMap, pData->sourceDistance, pData->enableSourceDirectivity,
                                 NUM_EARS, FRAME_SIZE, pData->outputFrameTD);

        /* Copy to output */
        for(ch=0; ch<SAF_MIN(NUM_EARS,nOutputs); ch++)
            memcpy(outputs[ch], pData->outputFrameTD[ch], FRAME_SIZE*sizeof(float));
        for(; ch<nOutputs; ch++)
            memset(outputs[ch], 0, FRAME_SIZE * sizeof(float)); /* fill remaining channels with zeros */
    }
    else{
        /* output zero if one of the pre-requrisite conditions are not met */
        for(ch=0; ch<nOutputs; ch++)
            memset(outputs[ch], 0, FRAME_SIZE*sizeof(float));
    }

    pData->procStatus = PROC_STATUS_NOT_ONGOING;
}
    
/* Set Functions */
    
void interface_refreshSettings(void* const hInt)
{
    interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
}

void interface_setFavour2Daccuracy(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->favour2Daccuracy!=newState){
        pData->favour2Daccuracy = newState;
        interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
    }
}
    
void interface_setEnableEPbeamformers(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->enableEPbeamformers!=newState){
        pData->enableEPbeamformers = newState;
        interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
    }
}

void interface_setEnableDiffEQ_HRTFs(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->enableDiffEQ_HRTFs!=newState){
        pData->enableDiffEQ_HRTFs = newState;
        interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
    }
}

void interface_setEnableDiffEQ_ATFs(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->enableDiffEQ_ATFs!=newState){
        pData->enableDiffEQ_ATFs = newState;
        interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
    }
}

void interface_setDOFoption(void* const hInt, INTERFACE_DOF_OPTIONS newOption)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->renderingMode = newOption;
}

void interface_setAnalysisAveraging(void* const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->hAna==NULL)
        return;
    *proposed_analysis_getCovarianceAvagingCoeffPtr(pData->hAna) = newValue;
}

void interface_setSynthesisAveraging(void* const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->hSyn==NULL)
        return;
    *proposed_synthesis_getSynthesisAveragingCoeffPtr(pData->hSyn) = newValue;
}

void interface_setMaximumAnalysisFreq(void* const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->hAna==NULL)
        return;
    *proposed_analysis_getMaxAnalysisFreqPtr(pData->hAna) = newValue;
}
 
void interface_setMaximumBSMFreq(void* const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->hSyn==NULL)
        return;
    *proposed_synthesis_getMaxBSMFreqPtr(pData->hSyn) = newValue;
}
    
void interface_setMaximumMagLSFreq(void* const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->hSyn==NULL)
        return;
    *proposed_synthesis_getMaxMagLSFreqPtr(pData->hSyn) = newValue;
}

void interface_setLinear2ParametricBalance(void* const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->hSyn==NULL)
        return;
    *proposed_synthesis_getLinear2ParametricBalancePtr(pData->hSyn) = newValue;
}

void interface_setStreamBalanceFromLocal(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    int nBands;
    float* streamBalance;
    streamBalance = proposed_synthesis_getStreamBalancePtr(pData->hSyn, &nBands);
    if(nBands==pData->nBands_local && streamBalance != NULL && pData->streamBalBands_local != NULL)
        memcpy(streamBalance, pData->streamBalBands_local, nBands*sizeof(float));
}

void interface_setStreamBalance(void * const hInt, float newValue, int bandIdx)
{
    interface_data *pData = (interface_data*)(hInt);
    int nBands;
    float* streamBalance;
    streamBalance = proposed_synthesis_getStreamBalancePtr(pData->hSyn, &nBands);
    if(bandIdx>=nBands-1 || streamBalance == NULL)
        return;
    streamBalance[bandIdx] = newValue;
    pData->streamBalBands_local[bandIdx] = newValue;
}

void interface_setStreamBalanceAllBands(void * const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    int nBands, band;
    float* streamBalance;
    streamBalance = proposed_synthesis_getStreamBalancePtr(pData->hSyn, &nBands);
    for(band=0; band<nBands; band++){ /* nBands==0 when streamBalance==NULL */
        streamBalance[band] = newValue;
        pData->streamBalBands_local[band] = newValue;
    }
}

void interface_setSofaFilePathMAIR(void* const hInt, const char* path)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->sofa_filepath_MAIR = realloc1d(pData->sofa_filepath_MAIR, strlen(path) + 1);
    strcpy(pData->sofa_filepath_MAIR, path);
    interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
}

void interface_setUseDefaultHRIRsflag(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if((!pData->useDefaultHRIRsFLAG) && (newState)){
        pData->useDefaultHRIRsFLAG = newState;
        interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
    }
}

void interface_setSofaFilePathHRIR(void* const hInt, const char* path)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->sofa_filepath_HRIR = realloc1d(pData->sofa_filepath_HRIR, strlen(path) + 1);
    strcpy(pData->sofa_filepath_HRIR, path);
    interface_setCoreStatus(hInt, CORE_STATUS_NOT_INITIALISED);
}

void interface_setYaw(void  * const hInt, float newYaw)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->yaw = pData->bFlipYaw == 1 ? -DEG2RAD(newYaw) : DEG2RAD(newYaw);
}

void interface_setPitch(void* const hInt, float newPitch)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->pitch = pData->bFlipPitch == 1 ? -DEG2RAD(newPitch) : DEG2RAD(newPitch);
}

void interface_setRoll(void* const hInt, float newRoll)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->roll = pData->bFlipRoll == 1 ? -DEG2RAD(newRoll) : DEG2RAD(newRoll);
}

void interface_setFlipYaw(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(newState !=pData->bFlipYaw ){
        pData->bFlipYaw = newState;
        interface_setYaw(hInt, -interface_getYaw(hInt));
    }
}

void interface_setFlipPitch(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(newState !=pData->bFlipPitch ){
        pData->bFlipPitch = newState;
        interface_setPitch(hInt, -interface_getPitch(hInt));
    }
}

void interface_setFlipRoll(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(newState !=pData->bFlipRoll ){
        pData->bFlipRoll = newState;
        interface_setRoll(hInt, -interface_getRoll(hInt));
    }
}

void interface_setDistanceMapMode(void* const hInt, INTERFACE_DISTANCE_MAPS newMap)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->distMapOption = newMap;
}

void interface_setSourceDistance(void  * const hInt, float newValue)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->sourceDistance = SAF_MIN(newValue, INTERFACE_PERIMETER_DISTANCE_M);
}
 
void interface_setEnableSourceDirectivity(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->enableSourceDirectivity = newState;
}

void interface_setX(void  * const hInt, float newX)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->x = pData->bFlipX == 1 ? -(newX) : (newX);
}

void interface_setY(void* const hInt, float newY)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->y = pData->bFlipY == 1 ? -(newY) : (newY);
}

void interface_setZ(void* const hInt, float newZ)
{
    interface_data *pData = (interface_data*)(hInt);
    pData->z = pData->bFlipZ == 1 ? -(newZ) : (newZ);
}

void interface_setFlipX(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(newState !=pData->bFlipX){
        pData->bFlipX = newState;
        interface_setX(hInt, -interface_getX(hInt));
    }
}

void interface_setFlipY(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(newState !=pData->bFlipY){
        pData->bFlipY = newState;
        interface_setY(hInt, -interface_getY(hInt));
    }
}

void interface_setFlipZ(void* const hInt, int newState)
{
    interface_data *pData = (interface_data*)(hInt);
    if(newState !=pData->bFlipZ){
        pData->bFlipZ = newState;
        interface_setZ(hInt, -interface_getZ(hInt));
    }
}

/* Get Functions */

int interface_getFrameSize(void)
{
    return FRAME_SIZE;
}

INTERFACE_CORE_STATUS interface_getCoreStatus(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->coreStatus;
}

float interface_getProgressBar0_1(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->progressBar0_1;
}

void interface_getProgressBarText(void* const hInt, char* text)
{
    interface_data *pData = (interface_data*)(hInt);
    memcpy(text, pData->progressBarText, INTERFACE_PROGRESSBARTEXT_CHAR_LENGTH*sizeof(char));
}

int interface_getFavour2Daccuracy(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->favour2Daccuracy;
}
    
int interface_getEnableEPbeamformers(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->enableEPbeamformers;
}

int interface_getEnableDiffEQ_HRTFs(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->enableDiffEQ_HRTFs;
}

int interface_getEnableDiffEQ_ATFs(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->enableDiffEQ_ATFs;
}
    
INTERFACE_DOF_OPTIONS interface_getDOFoption(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->renderingMode;
}

float interface_getAnalysisAveraging(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->hAna==NULL ? 0.5f : *proposed_analysis_getCovarianceAvagingCoeffPtr(pData->hAna);
}

float interface_getSynthesisAveraging(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->hSyn==NULL ? 0.5f : *proposed_synthesis_getSynthesisAveragingCoeffPtr(pData->hSyn);
}

float interface_getMaximumAnalysisFreq(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->hAna==NULL ? 5000.0f : *proposed_analysis_getMaxAnalysisFreqPtr(pData->hAna);
}
 
float interface_getMaximumBSMFreq(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->hSyn==NULL ? 5000.0f : *proposed_synthesis_getMaxBSMFreqPtr(pData->hSyn);
}
    
float interface_getMaximumMagLSFreq(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->hSyn==NULL ? 1500.0f : *proposed_synthesis_getMaxMagLSFreqPtr(pData->hSyn);
}

float interface_getLinear2ParametricBalance(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->hSyn==NULL ? 1.0f : *proposed_synthesis_getLinear2ParametricBalancePtr(pData->hSyn);
}

float interface_getStreamBalance(void* const hInt, int bandIdx)
{
    interface_data *pData = (interface_data*)(hInt);
    int nBands;
    float* streamBalance;
    streamBalance = proposed_synthesis_getStreamBalancePtr(pData->hSyn, &nBands);
    if(bandIdx>=nBands-1)
        return 0.0f;
    else
        return streamBalance == NULL ? 0.0f : streamBalance[bandIdx];
}

float interface_getStreamBalanceAllBands(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    float* streamBalance;
    streamBalance = proposed_synthesis_getStreamBalancePtr(pData->hSyn, NULL);
    return streamBalance == NULL ? 0.0f : streamBalance[0];
}
    
void interface_getStreamBalanceLocalPtrs
(
    void* const hInt,
    float** pX_vector,
    float** pY_values,
    int* pNpoints
)
{
    interface_data *pData = (interface_data*)(hInt);
    float* tmp;

    (*pNpoints) = pData->nBands_local;
    (*pX_vector) = pData->freqVector_local;
    if(pData->hSyn!=NULL){
        tmp = proposed_synthesis_getStreamBalancePtr(pData->hSyn, NULL);
        memcpy(pData->streamBalBands_local, tmp, pData->nBands_local*sizeof(float));
    }
    (*pY_values) = pData->streamBalBands_local;
}

void interface_getHistogramLocalPtrs
(
    void* const hInt,
    float** pY_values,
    int* pNpoints
)
{
    interface_data *pData = (interface_data*)(hInt);
    float* tmp;

    (*pNpoints) = pData->nDirs_local;
    if(pData->hAna!=NULL){
        tmp = (float*)proposed_analysis_getHistogramPtr(pData->hAna, NULL);
        memcpy(pData->histogram_local, tmp, pData->nDirs_local*sizeof(float));
    }
    (*pY_values) = pData->histogram_local;
}

void interface_getGridDirectionsXYZLocalPtrs
(
    void* const hInt,
    float** pY_values,
    int* pNpoints
)
{
    interface_data *pData = (interface_data*)(hInt);
    float* tmp;
    int nDirs_local;

    (*pNpoints) = pData->nDirs_local;
    if(pData->hAna!=NULL){
        tmp = (float*)proposed_analysis_getGridDirsXYZPtr(pData->hAna, &nDirs_local);
        //pData->grid_dirs_xyz_local = realloc(pData->grid_dirs_xyz_local, 3000*sizeof(float));
        if (pData->nDirs_local == nDirs_local && tmp!=NULL){
        memcpy(pData->grid_dirs_xyz_local, tmp, pData->nDirs_local*3*sizeof(float));
        }
        else
            (*pNpoints) = 0;
    }
    (*pY_values) = pData->grid_dirs_xyz_local;
}

float* interface_getHeadOrientationPtr(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return &pData->head_orientation_xyz[0];
}

int interface_getNumberOfBands(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->hAna == NULL ? 0 : proposed_analysis_getNbands(pData->hAna);
}

int interface_getNmicsArray(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->nMics;
}

int interface_getNDirsArray(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->nDirs;
}

int interface_getIRlengthArray(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->IRlength;
}

int interface_getIRsamplerateArray(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->IR_fs;
}

int interface_getNDirsBin(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->binConfig.nHRIR;
}

int interface_getIRlengthBin(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->binConfig.lHRIR;
}

int interface_getIRsamplerateBin(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->binConfig.hrir_fs;
}

int interface_getDAWsamplerate(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return (int)pData->fs;
}

char* interface_getSofaFilePathMAIR(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->sofa_filepath_MAIR!=NULL)
        return pData->sofa_filepath_MAIR;
    else
        return "no_file";
}

int interface_getUseDefaultHRIRsflag(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->useDefaultHRIRsFLAG;
}

char* interface_getSofaFilePathHRIR(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    if(pData->sofa_filepath_HRIR!=NULL)
        return pData->sofa_filepath_HRIR;
    else
        return "no_file";
}

float interface_getSourceDistance(void  * const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->sourceDistance;
}

INTERFACE_DISTANCE_MAPS interface_getDistanceMapMode(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->distMapOption;
}

int interface_getEnableSourceDirectivity(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->enableSourceDirectivity;
}

float interface_getYaw(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipYaw == 1 ? -RAD2DEG(pData->yaw) : RAD2DEG(pData->yaw);
}

float interface_getPitch(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipPitch == 1 ? -RAD2DEG(pData->pitch) : RAD2DEG(pData->pitch);
}

float interface_getRoll(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipRoll == 1 ? -RAD2DEG(pData->roll) : RAD2DEG(pData->roll);
}

int interface_getFlipYaw(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipYaw;
}

int interface_getFlipPitch(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipPitch;
}

int interface_getFlipRoll(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipRoll;
}

float interface_getX(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipX == 1 ? -(pData->x) : (pData->x);
}

float interface_getY(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipY == 1 ? -(pData->y) : (pData->y);
}

float interface_getZ(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipZ == 1 ? -(pData->z) : (pData->z);
}

int interface_getFlipX(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipX;
}

int interface_getFlipY(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipY;
}

int interface_getFlipZ(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return pData->bFlipZ;
}

int interface_getProcessingDelay(void* const hInt)
{
    interface_data *pData = (interface_data*)(hInt);
    return proposed_analysis_getProcDelay(pData->hAna)+proposed_synthesis_getProcDelay(pData->hSyn);
} 
