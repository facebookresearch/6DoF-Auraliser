/**
 * @file unit_tests.h
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

#ifndef __UNIT_TESTS_H_INCLUDED__
#define __UNIT_TESTS_H_INCLUDED__

#include "resources/unity.h" /* unit testing suite (MIT license) */
#include "resources/timer.h" /* for timing the individual tests (PublicDomain)*/
#include "saf.h"             /* master framework include header */
#include "saf_externals.h"   /* to also include saf dependencies (cblas etc.) */
#include "proposed.h"        /* main header for the proposed method */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Main unit testing program */
int main_test(void);

/** Proposed method */
void test__proposed_method(void);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __UNIT_TESTS_H_INCLUDED__ */
