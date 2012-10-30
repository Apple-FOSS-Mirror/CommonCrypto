/*
 * Copyright (c) 2012 Apple Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

// #define COMMON_KEYDERIVATION_FUNCTIONS

#include "CommonKeyDerivation.h"
#include <corecrypto/ccpbkdf2.h>
#include "CommonDigestPriv.h"
#include "CommonDigestSPI.h"
#include "ccdebug.h"


int 
CCKeyDerivationPBKDF( CCPBKDFAlgorithm algorithm, const char *password, size_t passwordLen,
					 const uint8_t *salt, size_t saltLen,
					 CCPseudoRandomAlgorithm prf, uint rounds, 
					 uint8_t *derivedKey, size_t derivedKeyLen)
{
    const struct ccdigest_info *di;

    CC_DEBUG_LOG(ASL_LEVEL_ERR, "PasswordLen %lu SaltLen %lU PRF %d Rounds %u DKLen %lu\n", passwordLen, saltLen, prf, rounds, derivedKeyLen);
    if(algorithm != kCCPBKDF2) return -1;
    switch(prf) {
        case kCCPRFHmacAlgSHA1: di = CCDigestGetDigestInfo(kCCDigestSHA1); break;
        case kCCPRFHmacAlgSHA224: di = CCDigestGetDigestInfo(kCCDigestSHA224); break;
        case kCCPRFHmacAlgSHA256: di = CCDigestGetDigestInfo(kCCDigestSHA256); break;
        case kCCPRFHmacAlgSHA384: di = CCDigestGetDigestInfo(kCCDigestSHA384); break;
        case kCCPRFHmacAlgSHA512: di = CCDigestGetDigestInfo(kCCDigestSHA512); break;
        default: return -1;
    }
    if(!password || !salt || !derivedKey || (derivedKeyLen == 0) || (rounds == 0)) return -1;
    
    ccpbkdf2_hmac(di, passwordLen, password, saltLen, salt, rounds, derivedKeyLen, derivedKey);
    return 0;
}

#include <mach/mach.h>
#include <mach/mach_time.h>
#define ROUNDMEASURE 100000
// This is for the scratchspace - it's twice the size of the max PRF buffer + 4 to work within the pbkdf2 code we currently
// have.

#define CC_MAX_PRF_WORKSPACE 128+4


static uint64_t
timer()
{
	static mach_timebase_info_data_t    sTimebaseInfo;
    uint64_t        timeNano;
    
	if ( sTimebaseInfo.denom == 0 ) {
        (void) mach_timebase_info(&sTimebaseInfo);
    }
	
	timeNano = mach_absolute_time();
	return (uint64_t) (timeNano * sTimebaseInfo.numer) / (sTimebaseInfo.denom * 1000000);
}

uint
CCCalibratePBKDF(CCPBKDFAlgorithm algorithm, size_t passwordLen, size_t saltLen,
				 CCPseudoRandomAlgorithm prf, size_t derivedKeyLen, uint32_t msec)
{
	char        *password;
	uint8_t     *salt;
	uint64_t	startTime, endTime, elapsedTime;
	uint8_t     *derivedKey;
	int         i;
    
    CC_DEBUG_LOG(ASL_LEVEL_ERR, "Entering\n");
	if (derivedKeyLen == 0) return -1; // bad parameters
	if (saltLen == 0 || saltLen > CC_MAX_PRF_WORKSPACE) return -1; // out of bounds parameters
	if (passwordLen == 0 ) passwordLen = 1;
	if(algorithm != kCCPBKDF2) return -1;
    
	if((password = malloc(passwordLen)) == NULL) return -1;
	for(i=0; i<passwordLen; i++) password[i] = 'a';
	if((salt = malloc(saltLen)) == NULL) return -1;
	for(i=0; i<saltLen; i++) salt[i] = i%256;
	if((derivedKey = malloc(derivedKeyLen)) == NULL) return -1;
    
    for(elapsedTime = 0, i=0; i < 5 && elapsedTime == 0; i++) {
        startTime = timer();
        if(CCKeyDerivationPBKDF(algorithm, password, passwordLen, salt, saltLen, prf, ROUNDMEASURE, derivedKey, derivedKeyLen)) return -2;
        endTime = timer();
        
        elapsedTime = endTime - startTime;
	}
    
    if(elapsedTime == 0) return 123456; // arbitrary, but something is seriously wrong
    
	free(password);
	free(salt);
	free(derivedKey);
    
	return (msec * ROUNDMEASURE)/elapsedTime;
}

