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

#include "corecryptoSymmetricBridge.h"
#include "ccMemory.h"
#include <corecrypto/ccrc4.h>

static void *noMode(void) { return NULL; }

// RC4 as a mode trick ...

void rc4ModeInit(const struct ccmode_ofb *ofb, void *ctx,
             unsigned long key_len, const void *key,
             const void *iv)
{
    ccrc4_eay.init(ctx, key_len, key);
}

void rc4crypt(void *ctx, unsigned long nbytes, const void *in, void *out)
{
    ccrc4_eay.crypt(ctx, nbytes, in, out);
}

typedef struct eay_rc4_key_st
{
	uint32_t x,y;
	uint32_t data[256];
} eay_RC4_KEY;

struct ccmode_ofb rc4mode = {
    .size = sizeof(eay_RC4_KEY),
    .block_size = 1,
    .init = rc4ModeInit,
    .ofb = rc4crypt,
};



struct ccmode_ofb *cc_rc4_crypt_mode(void)
{
    return &rc4mode;
}


// 2 dimensional array of various mode/cipher contexts
// encrypt/decrypt x algorithm matching the list in CommonCryptor.h

modeList ccmodeList[7][2] = {
    { // AES
        { ccaes_ecb_encrypt_mode, ccaes_cbc_encrypt_mode, ccaes_cfb_encrypt_mode, ccaes_cfb8_encrypt_mode, ccaes_ctr_crypt_mode, ccaes_ofb_crypt_mode, ccaes_xts_encrypt_mode, ccaes_gcm_encrypt_mode },
        { ccaes_ecb_decrypt_mode, ccaes_cbc_decrypt_mode, ccaes_cfb_decrypt_mode, ccaes_cfb8_decrypt_mode, ccaes_ctr_crypt_mode, ccaes_ofb_crypt_mode, ccaes_xts_decrypt_mode, ccaes_gcm_decrypt_mode }
    },
    
    { // DES
        { ccdes_ecb_encrypt_mode, ccdes_cbc_encrypt_mode, ccdes_cfb_encrypt_mode, ccdes_cfb8_encrypt_mode, ccdes_ctr_crypt_mode, ccdes_ofb_crypt_mode, noMode, noMode },
        { ccdes_ecb_decrypt_mode, ccdes_cbc_decrypt_mode, ccdes_cfb_decrypt_mode, ccdes_cfb8_decrypt_mode, ccdes_ctr_crypt_mode, ccdes_ofb_crypt_mode, noMode, noMode }
    },
    
    { // DES3
        { ccdes3_ecb_encrypt_mode, ccdes3_cbc_encrypt_mode, ccdes3_cfb_encrypt_mode, ccdes3_cfb8_encrypt_mode, ccdes3_ctr_crypt_mode, ccdes3_ofb_crypt_mode, noMode, noMode },
        { ccdes3_ecb_decrypt_mode, ccdes3_cbc_decrypt_mode, ccdes3_cfb_decrypt_mode, ccdes3_cfb8_decrypt_mode, ccdes3_ctr_crypt_mode, ccdes3_ofb_crypt_mode, noMode, noMode }
    },
    
    { // CAST
        { cccast_ecb_encrypt_mode, cccast_cbc_encrypt_mode, cccast_cfb_encrypt_mode, cccast_cfb8_encrypt_mode, cccast_ctr_crypt_mode, cccast_ofb_crypt_mode, noMode, noMode },
        { cccast_ecb_decrypt_mode, cccast_cbc_decrypt_mode, cccast_cfb_decrypt_mode, cccast_cfb8_decrypt_mode, cccast_ctr_crypt_mode, cccast_ofb_crypt_mode, noMode, noMode }
    },
    
    { // RC4 - hijack OFB to put in streaming cipher descriptor
        { noMode, noMode, noMode, noMode, noMode, cc_rc4_crypt_mode, noMode, noMode },
        { noMode, noMode, noMode, noMode, noMode, cc_rc4_crypt_mode, noMode, noMode }
    },

    
    { // RC2
        { ccrc2_ecb_encrypt_mode, ccrc2_cbc_encrypt_mode, ccrc2_cfb_encrypt_mode, ccrc2_cfb8_encrypt_mode, ccrc2_ctr_crypt_mode, ccrc2_ofb_crypt_mode, noMode, noMode },
        { ccrc2_ecb_decrypt_mode, ccrc2_cbc_decrypt_mode, ccrc2_cfb_decrypt_mode, ccrc2_cfb8_decrypt_mode, ccrc2_ctr_crypt_mode, ccrc2_ofb_crypt_mode, noMode, noMode }
    },

    { // Blowfish
        { ccblowfish_ecb_encrypt_mode, ccblowfish_cbc_encrypt_mode, ccblowfish_cfb_encrypt_mode, ccblowfish_cfb8_encrypt_mode, ccblowfish_ctr_crypt_mode, ccblowfish_ofb_crypt_mode, noMode, noMode },
        { ccblowfish_ecb_decrypt_mode, ccblowfish_cbc_decrypt_mode, ccblowfish_cfb_decrypt_mode, ccblowfish_cfb8_decrypt_mode, ccblowfish_ctr_crypt_mode, ccblowfish_ofb_crypt_mode, noMode, noMode }
    },
};


// Thunks
//ECB

static size_t ccecb_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.ecb->size; }
static size_t ccecb_mode_get_block_size(corecryptoMode modeObject) { return modeObject.ecb->block_size; }
static void ccecb_mode_setup(corecryptoMode modeObj, const void *IV,
                             const void *key, size_t keylen, const void *tweak,
                             int tweaklen, int options, modeCtx ctx)
{
    modeObj.ecb->init(modeObj.ecb, ctx.ecb, keylen, key);
}

static void ccecb_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.ecb->ecb(ctx.ecb, len / ccecb_mode_get_block_size(modeObj), in, out);
}

cc2CCModeDescriptor ccecb_mode = {
    .mode_get_ctx_size = ccecb_mode_get_ctx_size,
    .mode_get_block_size = ccecb_mode_get_block_size,
    .mode_setup = ccecb_mode_setup,
    .mode_encrypt = ccecb_mode_crypt,
    .mode_decrypt = ccecb_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// CBC

static size_t cccbc_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.cbc->size + 16; }
static size_t cccbc_mode_get_block_size(corecryptoMode modeObject) { return modeObject.cbc->block_size; }
static void cccbc_mode_setup(corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void *tweak,
                             size_t tweaklen, int options, modeCtx ctx)
{
    CC_XMEMCPY(ctx.cbc->iv, iv, modeObj.cbc->block_size);
    modeObj.cbc->init(modeObj.cbc, &ctx.cbc->cbc, keylen, key);
}

static void cccbc_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.cbc->cbc(&ctx.cbc->cbc, ctx.cbc->iv, len / cccbc_mode_get_block_size(modeObj), in, out);
}

static int cccbc_getiv(corecryptoMode modeObj, void *iv, uint32_t *len, modeCtx ctx)
{
    if(*len < cccbc_mode_get_block_size(modeObj)) {
        *len = cccbc_mode_get_block_size(modeObj);
        return -1;
    }
    uint8_t tmp[cccbc_mode_get_block_size(modeObj)];
    CC_XMEMCPY(iv, ctx.cbc->iv, *len = cccbc_mode_get_block_size(modeObj));
    return 0;
}

static int cccbc_setiv(corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx)
{
    uint8_t tmp[cccbc_mode_get_block_size(modeObj)];
    if(len != cccbc_mode_get_block_size(modeObj)) return -1;
    CC_XMEMCPY(ctx.cbc->iv, iv, cccbc_mode_get_block_size(modeObj));
    return 0;
}

cc2CCModeDescriptor cccbc_mode = {
    .mode_get_ctx_size = cccbc_mode_get_ctx_size,
    .mode_get_block_size = cccbc_mode_get_block_size,
    .mode_setup = cccbc_mode_setup,
    .mode_encrypt = cccbc_mode_crypt,
    .mode_decrypt = cccbc_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = cccbc_setiv,
    .mode_getiv = cccbc_getiv
};

// CFB

static size_t cccfb_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.cfb->size; }
static size_t cccfb_mode_get_block_size(corecryptoMode modeObject) { return modeObject.cfb->block_size; }
static void cccfb_mode_setup(corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void *tweak,
                             size_t tweaklen, int options, modeCtx ctx)
{
    modeObj.cfb->init(modeObj.cfb, ctx.cfb, keylen, key, iv);
}

static void cccfb_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.cfb->cfb(ctx.cfb, len / cccfb_mode_get_block_size(modeObj), in, out);
}

cc2CCModeDescriptor cccfb_mode = {
    .mode_get_ctx_size = cccfb_mode_get_ctx_size,
    .mode_get_block_size = cccfb_mode_get_block_size,
    .mode_setup = cccfb_mode_setup,
    .mode_encrypt = cccfb_mode_crypt,
    .mode_decrypt = cccfb_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};


// CFB8

static size_t cccfb8_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.cfb8->size; }
static size_t cccfb8_mode_get_block_size(corecryptoMode modeObject) { return modeObject.cfb8->block_size; }
static void cccfb8_mode_setup(corecryptoMode modeObj, const void *iv,
                              const void *key, size_t keylen, const void *tweak,
                              size_t tweaklen, int options, modeCtx ctx)
{
    modeObj.cfb8->init(modeObj.cfb8, ctx.cfb8, keylen, key, iv);
}

static void cccfb8_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.cfb8->cfb8(ctx.cfb8, len / cccfb8_mode_get_block_size(modeObj), in, out);
}

cc2CCModeDescriptor cccfb8_mode = {
    .mode_get_ctx_size = cccfb8_mode_get_ctx_size,
    .mode_get_block_size = cccfb8_mode_get_block_size,
    .mode_setup = cccfb8_mode_setup,
    .mode_encrypt = cccfb8_mode_crypt,
    .mode_decrypt = cccfb8_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// CTR

static size_t ccctr_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.ctr->size; }
static size_t ccctr_mode_get_block_size(corecryptoMode modeObject) { return modeObject.ctr->block_size; }
static void ccctr_mode_setup(corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void *tweak,
                             size_t tweaklen, int options, modeCtx ctx)
{
    modeObj.ctr->init(modeObj.ctr, ctx.ctr, keylen, key, iv);
}

static void ccctr_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.ctr->ctr(ctx.ctr, len / ccctr_mode_get_block_size(modeObj), in, out);
}

cc2CCModeDescriptor ccctr_mode = {
    .mode_get_ctx_size = ccctr_mode_get_ctx_size,
    .mode_get_block_size = ccctr_mode_get_block_size,
    .mode_setup = ccctr_mode_setup,
    .mode_encrypt = ccctr_mode_crypt,
    .mode_decrypt = ccctr_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// OFB

static size_t ccofb_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.ofb->size; }
static size_t ccofb_mode_get_block_size(corecryptoMode modeObject) { return modeObject.ofb->block_size; }
static void ccofb_mode_setup(corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void *tweak,
                             size_t tweaklen, int options, modeCtx ctx)
{
    modeObj.ofb->init(modeObj.ofb, ctx.ofb, keylen, key, iv);
}

static void ccofb_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.ofb->ofb(ctx.ofb, len / ccofb_mode_get_block_size(modeObj), in, out);
}

cc2CCModeDescriptor ccofb_mode = {
    .mode_get_ctx_size = ccofb_mode_get_ctx_size,
    .mode_get_block_size = ccofb_mode_get_block_size,
    .mode_setup = ccofb_mode_setup,
    .mode_encrypt = ccofb_mode_crypt,
    .mode_decrypt = ccofb_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// XTS
/* For now we always schedule both encrypt and decrypt contexts for AES-XTS.  Original CommonCrypto support
 * allowed a "both" (kCCEncrypt and kCCDecrypt) capability used for AES-XTS block I/O.  The initialization 
 * and correct mode objext and context passing are done at the CommonCryptor layer.
 */


static size_t ccxts_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.xts->size; }
static size_t ccxts_mode_get_block_size(corecryptoMode modeObject) { return modeObject.xts->block_size; }
static void ccxts_mode_setup(corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void *tweak,
                             size_t tweaklen, int options, modeCtx ctx)
{
    modeObj.xts->init(modeObj.xts, ctx.xts, keylen, key, tweak);
}

#ifdef UNUSED_INTERFACE
static void ccxts_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.xts->xts(ctx.xts, len / ccxts_mode_get_block_size(modeObj), in, out);
}

static int ccxts_setiv(corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx)
{
    if(len != modeObj.xts->block_size) return -1;
    modeObj.xts->set_tweak(ctx.xts, iv);
    return 0;
}

static int ccxts_getiv(corecryptoMode modeObj, void *iv, uint32_t *len, modeCtx ctx)
{
    if(*len < modeObj.xts->block_size) {
        *len = modeObj.xts->block_size;
        return -1;
    }
    CC_XMEMCPY(iv, modeObj.xts->xts(ctx.xts, 0, NULL, NULL), *len = modeObj.xts->block_size);
    return 0;
}
#endif

/*
 * These match what we had in libtomcrypt - they really are "this is a logical block" routines, so need
 * to handle partial blocks - so we use corecrypto's xts pad routines in every case.
 */

static void ccxts_mode_encrypt_tweak(corecryptoMode modeObj, const void *in, void *out, size_t len, const void *iv, modeCtx ctx)
{
    ccxts_tweak_decl(ccxts_context_size(modeObj.xts), tweak);
    modeObj.xts->set_tweak(ctx.xts, tweak, iv);
    ccpad_xts_encrypt(modeObj.xts, ctx.xts, tweak, len, in, out);
}

static void ccxts_mode_decrypt_tweak(corecryptoMode modeObj, const void *in, void *out, size_t len, const void *iv, modeCtx ctx)
{
    ccxts_tweak_decl(ccxts_context_size(modeObj.xts), tweak);
    modeObj.xts->set_tweak(ctx.xts, tweak, iv);
    ccpad_xts_decrypt(modeObj.xts, ctx.xts, tweak, len, in, out);
}


cc2CCModeDescriptor ccxts_mode = {
    .mode_get_ctx_size = ccxts_mode_get_ctx_size,
    .mode_get_block_size = ccxts_mode_get_block_size,
    .mode_setup = ccxts_mode_setup,
    .mode_encrypt = NULL,
    .mode_decrypt = NULL,
    .mode_encrypt_tweaked = ccxts_mode_encrypt_tweak,
    .mode_decrypt_tweaked = ccxts_mode_decrypt_tweak,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// GCM

static size_t ccgcm_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.gcm->size; }
static size_t ccgcm_mode_get_block_size(corecryptoMode modeObject) { return modeObject.gcm->block_size; }
static void ccgcm_mode_setup(corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void *tweak,
                             size_t tweaklen, int options, modeCtx ctx)
{
    modeObj.gcm->init(modeObj.gcm, ctx.gcm, keylen, key);
}

static void ccgcm_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.gcm->gcm(ctx.gcm, len, in, out);
}

static int ccgcm_setiv(corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx)
{
    modeObj.gcm->set_iv(ctx.gcm, len, iv);
    return 0;
}


cc2CCModeDescriptor ccgcm_mode = {
    .mode_get_ctx_size = ccgcm_mode_get_ctx_size,
    .mode_get_block_size = ccgcm_mode_get_block_size,
    .mode_setup = ccgcm_mode_setup,
    .mode_encrypt = ccgcm_mode_crypt,
    .mode_decrypt = ccgcm_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = ccgcm_setiv,
    .mode_getiv = NULL
};

// Padding

static int ccpkcs7_encrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_pkcs7_encrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, cipherText);
    *moved = modeptr->mode_get_block_size(modeObj);
    return 0;
}
static int ccpkcs7_decrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    *moved = ccpad_pkcs7_decrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, plainText);
    return 0;
}


static int ccpkcs7_encrypt_ecb_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_pkcs7_ecb_encrypt(modeObj.ecb, ctx.ecb, len, buff, cipherText);
    *moved = modeptr->mode_get_block_size(modeObj);
    return 0;
}
static int ccpkcs7_decrypt_ecb_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    *moved = ccpad_pkcs7_ecb_decrypt(modeObj.ecb, ctx.ecb, len, buff, plainText);
    return 0;
}


/*
 * Maximum space needed for padding.
 */

#define MAXBLOCKSIZE_PKCS7 128

static size_t ccpkcs7_padlen(int encrypt, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, char *buffp)
{
    int retval = 0;
    size_t blocksize = modeptr->mode_get_block_size(modeObj);
    
    /* We're going to return blocksize for unpad as a "maximum needed".  Otherwise we're going to have to decrypt the last block to get the number */
    return blocksize;
}

/*
 * How many bytes to reserve to enable padding - this is pre-encrypt/decrypt bytes.
 */

static size_t ccpkcs7_reserve(int encrypt, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj)
{
    if(encrypt) {
		return 0;
    } else {
    	return modeptr->mode_get_block_size(modeObj);
    }
}

cc2CCPaddingDescriptor ccpkcs7_pad = {
    .encrypt_pad = ccpkcs7_encrypt_pad,
    .decrypt_pad = ccpkcs7_decrypt_pad,
    .padlen = ccpkcs7_padlen,
    .padreserve = ccpkcs7_reserve,
};

cc2CCPaddingDescriptor ccpkcs7_ecb_pad = {
    .encrypt_pad = ccpkcs7_encrypt_ecb_pad,
    .decrypt_pad = ccpkcs7_decrypt_ecb_pad,
    .padlen = ccpkcs7_padlen,
    .padreserve = ccpkcs7_reserve,
};


static int cccts1_encrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_cts1_encrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, cipherText);
    *moved = len;
    return 0;
}
static int cccts1_decrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    ccpad_cts1_decrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, plainText);
    *moved = len;
    return 0;
}

static int cccts2_encrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_cts2_encrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, cipherText);
    *moved = len;
    return 0;
}
static int cccts2_decrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    ccpad_cts2_decrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, plainText);
    *moved = len;
    return 0;
}


static int cccts3_encrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_cts3_encrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, cipherText);
    *moved = len;
    return 0;
}
static int cccts3_decrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    ccpad_cts3_decrypt(modeObj.cbc, &ctx.cbc->cbc, ctx.cbc->iv, len, buff, plainText);
    *moved = len;
    return 0;
}



/*
 * Maximum space needed for padding.
 */

#define MAXBLOCKSIZE_PKCS7 128

static size_t ccctsX_padlen(int encrypt, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, char *buffp)
{
	return 0;
}

/*
 * How many bytes to reserve to enable padding - this is pre-encrypt/decrypt bytes.
 */

static size_t ccctsX_reserve(int encrypt, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj)
{
    return modeptr->mode_get_block_size(modeObj) * 2;
}

cc2CCPaddingDescriptor cccts1_pad = {
    .encrypt_pad = cccts1_encrypt_pad,
    .decrypt_pad = cccts1_decrypt_pad,
    .padlen = ccctsX_padlen,
    .padreserve = ccctsX_reserve,
};

cc2CCPaddingDescriptor cccts2_pad = {
    .encrypt_pad = cccts2_encrypt_pad,
    .decrypt_pad = cccts2_decrypt_pad,
    .padlen = ccctsX_padlen,
    .padreserve = ccctsX_reserve,
};


cc2CCPaddingDescriptor cccts3_pad = {
    .encrypt_pad = cccts3_encrypt_pad,
    .decrypt_pad = cccts3_decrypt_pad,
    .padlen = ccctsX_padlen,
    .padreserve = ccctsX_reserve,
};


static int ccnopad_encrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    *moved = 0;
    return 0;
}
static int ccnopad_decrypt_pad(modeCtx ctx, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    *moved = 0;
    return 0;
}

/*
 * Maximum space needed for padding.
 */

static size_t ccnopad_padlen(int encrypt, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj, char *buffp)
{
	return 0;
}

/*
 * How many bytes to reserve to enable padding - this is pre-encrypt/decrypt bytes.
 */

static size_t ccnopad_reserve(int encrypt, cc2CCModeDescriptorPtr modeptr, corecryptoMode modeObj)
{
    return 0;
}

cc2CCPaddingDescriptor ccnopad_pad = {
    .encrypt_pad = ccnopad_encrypt_pad,
    .decrypt_pad = ccnopad_decrypt_pad,
    .padlen = ccnopad_padlen,
    .padreserve = ccnopad_reserve,
};


