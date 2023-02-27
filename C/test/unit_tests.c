/**
 * @file unit_tests.c
 * @brief Unit tests for the proposed method
 *
 * This unit testing program adapted from the unit testing program found here:
 * https://github.com/leomccormack/Spatial_Audio_Framework/tree/master/test
 * (ISC license)
 *
 * @author Leo McCormack
 * @date 9th August 2022
 *
 * Copyright (c) Meta Platforms, Inc. All Rights Reserved
 *
 */

#include "unit_tests.h"

static tick_t start;      /**< Start time for whole test program */
static tick_t start_test; /**< Start time for the current unit test */
/** Called before each unit test is executed */
void setUp(void) { start_test = timer_current(); }
/** Called after each unit test is executed */
void tearDown(void) { }
/** Displays the time taken to run the current unit test */
static void timerResult(void) {
    printf("    (Time elapsed: %lfs) \n", (double)timer_elapsed(start_test));
}

#undef RUN_TEST
/** A custom Unity RUN_TEST, which calls timerResult() upon exiting each test */
#define RUN_TEST(testfunc)  UNITY_NEW_TEST(#testfunc) \
    if (TEST_PROTECT()) {  setUp();  testfunc();  } \
    if (TEST_PROTECT() && (!TEST_IS_IGNORED))  {tearDown(); } \
    UnityConcludeTest(); timerResult();

/** Main test program */
int main_test(void) {
    /* Display the current SAF version and configuration */
    printf("%s\n", SAF_VERSION_BANNER);
    printf("%s\n", SAF_EXTERNALS_CONFIGURATION_STRING);
    printf("Executing the unit testing program");
#ifdef NDEBUG
    printf(" (Release):\n");
#else
    printf(" (Debug):\n");
#endif

    /* initialise */
    timer_lib_initialize();
    start = timer_current();
    UNITY_BEGIN();

    /* SAF utilities modules unit tests */
    RUN_TEST(test__proposed_method);
    
    /* close */
    timer_lib_shutdown();
    printf("\nTotal time elapsed: %lfs", (double)timer_elapsed(start));
    return UNITY_END();
}

/**
 * Quick test to analyse and render some audio. This unit test is mainly to
 * check for segfaults and memory leaks, and to do some optimisations using the
 * timer.
 *
 * The unit test was adapted from the unit test found here (GPLv2 License):
 * https://github.com/leomccormack/Spatial_Audio_Framework/blob/master/test/src/test__hades_module.c
 */
void test__proposed_method(void){
    proposed_analysis_handle hAna = NULL;          /* Analysis handle */
    proposed_synthesis_handle hSyn = NULL;         /* Synthesis handle */
    proposed_param_container_handle hPCon = NULL;  /* Parameter container handle */
    proposed_signal_container_handle hSCon = NULL; /* Signal container handle */
    proposed_binaural_config binConfig;
    SAF_SOFA_ERROR_CODES error;
    saf_sofa_container sofa;
    int i, ch, nDirs, nMics;
    float* array_dirs_deg;
    float ypr_rad[3] = {0.0f};
    float xyz_m[3] = {0.0f};

    /* Config */
    const int fs = 48000;
    const int sigLen = fs*2;
    const int hopsize = 64;
    const int blocksize = 256;

    /* Analysis */
    error = saf_sofa_open(&sofa, "/Users/leomccormack/Documents/git/resources/HADES/sofa/h_array.sofa", SAF_SOFA_READER_OPTION_DEFAULT);
    if(error!=SAF_SOFA_OK)
        return; /* SOFA File does not exist, so skip this unit test. */
    nDirs = sofa.nSources;
    nMics = sofa.nReceivers;
    array_dirs_deg = malloc1d(nDirs*2*sizeof(float));
    cblas_scopy(nDirs, sofa.SourcePosition, 3, array_dirs_deg, 2);         /* azi */
    cblas_scopy(nDirs, &sofa.SourcePosition[1], 3, &array_dirs_deg[1], 2); /* elev */
    proposed_analysis_create(&hAna, (float)fs, hopsize, blocksize, sofa.DataIR, array_dirs_deg, nDirs, nMics, sofa.DataLengthIR);
    saf_sofa_close(&sofa);

    /* Synthesis */
    error = saf_sofa_open(&sofa, "/Users/leomccormack/Documents/git/resources/HADES/sofa/hrirs.sofa", SAF_SOFA_READER_OPTION_DEFAULT);
    if(error!=SAF_SOFA_OK){
        /* SOFA File does not exist, so skip this unit test. */
        free(array_dirs_deg);
        proposed_analysis_destroy(&hAna);
        return;
    }
    binConfig.hrir_fs = sofa.DataSamplingRate;
    binConfig.lHRIR = sofa.DataLengthIR;
    binConfig.nHRIR = sofa.nSources;
    binConfig.hrirs = sofa.DataIR;
    binConfig.hrir_dirs_deg = malloc1d(binConfig.nHRIR*2*sizeof(float));
    cblas_scopy(binConfig.nHRIR, sofa.SourcePosition, 3, binConfig.hrir_dirs_deg, 2);         /* azi */
    cblas_scopy(binConfig.nHRIR, &sofa.SourcePosition[1], 3, &binConfig.hrir_dirs_deg[1], 2); /* elev */
    proposed_synthesis_create(&hSyn, hAna, &binConfig, PROPOSED_HRTF_INTERP_NEAREST, 0, 1, 1, 0);
    saf_sofa_close(&sofa);
     
    /* Parameter/signal containers */
    proposed_param_container_create(&hPCon, hAna);
    proposed_signal_container_create(&hSCon, hAna);
    
    /* Define input audio */
    float** inSigMIC;
    inSigMIC = (float**)malloc2d(nMics, sigLen, sizeof(float));
    rand_m1_1(FLATTEN2D(inSigMIC), nMics*sigLen);

    /* Main loop */
    float **inSigMIC_block, **outSigBIN_block, **outSigBIN;
    inSigMIC_block = (float**)malloc2d(nMics, blocksize, sizeof(float));
    outSigBIN_block = (float**)malloc2d(NUM_EARS, blocksize, sizeof(float));
    outSigBIN = (float**)malloc2d(NUM_EARS, sigLen, sizeof(float));
    for(i=0; i < (int)((float)sigLen/(float)blocksize); i++){
        /* Copy input to buffer */
        for(ch=0; ch<nMics; ch++)
            memcpy(inSigMIC_block[ch], &inSigMIC[ch][i*blocksize], blocksize*sizeof(float));

        /* Analysis/synthesis */
        proposed_analysis_apply(hAna, inSigMIC_block, nMics, blocksize, hPCon, hSCon);
        proposed_synthesis_apply(hSyn, hPCon, hSCon, (float*)ypr_rad, (float*)xyz_m, PROPOSED_DISTANCE_MAP_USE_PARAM, 2.0f, SAF_TRUE, NUM_EARS, blocksize, outSigBIN_block);
        
        /* Copy buffer to output */
        for(ch=0; ch<NUM_EARS; ch++)
            memcpy(&outSigBIN[ch][i*blocksize], outSigBIN_block[ch], blocksize*sizeof(float));
    }

    /* Clean-up */
    proposed_analysis_destroy(&hAna);
    proposed_param_container_destroy(&hPCon);
    proposed_signal_container_destroy(&hSCon);
    proposed_synthesis_destroy(&hSyn);
    free(array_dirs_deg);
    free(binConfig.hrir_dirs_deg);
    free(inSigMIC);
    free(inSigMIC_block);
    free(outSigBIN_block);
    free(outSigBIN);
}
