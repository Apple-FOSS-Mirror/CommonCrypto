/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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

/*
 *  ltc_rmd128.h
 *  MacTomCrypt
 *
 *  InfoSec Standard Configuration
 *  Copyright 2010 Apple Inc. All rights reserved.
 *
 */

#include <stdint.h>

#ifndef _LTC_RMD128_H_
#define _LTC_RMD128_H_

#define	LTC_RMD128_HASHSIZE	16
#define	LTC_RMD128_BLOCKSIZE	64

typedef struct ltc_rmd128_state {
    uint64_t length;
    uint32_t state[4];
    uint32_t curlen;
    unsigned char buf[LTC_RMD128_BLOCKSIZE];
} ltc_rmd128_ctx;

int ltc_rmd128_init(ltc_rmd128_ctx *ctx);
int ltc_rmd128_process(ltc_rmd128_ctx *ctx, const unsigned char *in,
    unsigned long inlen);
int ltc_rmd128_done(ltc_rmd128_ctx *ctx, unsigned char *hash);

#endif /* _LTC_RMD128_H_ */
