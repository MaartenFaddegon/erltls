#include "ssldh.h"
#include "macros.h"
#include <memory>

#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined LIBRESSL_VERSION_NUMBER
    #define DH_set0_pqg(A, B, C, D) do { (A)->p = B; (A)->g = D; } while (0)
#endif

#ifndef OPENSSL_NO_DH
/*
     1024-bit MODP Group with 160-bit prime order subgroup (RFC5114)
     -----BEGIN DH PARAMETERS-----
     MIIBDAKBgQCxC4+WoIDgHd6S3l6uXVTsUsmfvPsGo8aaap3KUtI7YWBz4oZ1oj0Y
     mDjvHi7mUsAT7LSuqQYRIySXXDzUm4O/rMvdfZDEvXCYSI6cIZpzck7/1vrlZEc4
     +qMaT/VbzMChUa9fDci0vUW/N982XBpl5oz9p21NpwjfH7K8LkpDcQKBgQCk0cvV
     w/00EmdlpELvuZkF+BBN0lisUH/WQGz/FCZtMSZv6h5cQVZLd35pD1UE8hMWAhe0
     sBuIal6RVH+eJ0n01/vX07mpLuGQnQ0iY/gKdqaiTAh6CR9THb8KAWm2oorWYqTR
     jnOvoy13nVkY0IvIhY9Nzvl8KiSFXm7rIrOy5QICAKA=
     -----END DH PARAMETERS-----
*/

static unsigned char dh1024_p[] = {
    0xB1, 0x0B, 0x8F, 0x96, 0xA0, 0x80, 0xE0, 0x1D, 0xDE, 0x92, 0xDE, 0x5E,
    0xAE, 0x5D, 0x54, 0xEC, 0x52, 0xC9, 0x9F, 0xBC, 0xFB, 0x06, 0xA3, 0xC6,
    0x9A, 0x6A, 0x9D, 0xCA, 0x52, 0xD2, 0x3B, 0x61, 0x60, 0x73, 0xE2, 0x86,
    0x75, 0xA2, 0x3D, 0x18, 0x98, 0x38, 0xEF, 0x1E, 0x2E, 0xE6, 0x52, 0xC0,
    0x13, 0xEC, 0xB4, 0xAE, 0xA9, 0x06, 0x11, 0x23, 0x24, 0x97, 0x5C, 0x3C,
    0xD4, 0x9B, 0x83, 0xBF, 0xAC, 0xCB, 0xDD, 0x7D, 0x90, 0xC4, 0xBD, 0x70,
    0x98, 0x48, 0x8E, 0x9C, 0x21, 0x9A, 0x73, 0x72, 0x4E, 0xFF, 0xD6, 0xFA,
    0xE5, 0x64, 0x47, 0x38, 0xFA, 0xA3, 0x1A, 0x4F, 0xF5, 0x5B, 0xCC, 0xC0,
    0xA1, 0x51, 0xAF, 0x5F, 0x0D, 0xC8, 0xB4, 0xBD, 0x45, 0xBF, 0x37, 0xDF,
    0x36, 0x5C, 0x1A, 0x65, 0xE6, 0x8C, 0xFD, 0xA7, 0x6D, 0x4D, 0xA7, 0x08,
    0xDF, 0x1F, 0xB2, 0xBC, 0x2E, 0x4A, 0x43, 0x71,
};

static unsigned char dh1024_g[] = {
    0xA4, 0xD1, 0xCB, 0xD5, 0xC3, 0xFD, 0x34, 0x12, 0x67, 0x65, 0xA4, 0x42,
    0xEF, 0xB9, 0x99, 0x05, 0xF8, 0x10, 0x4D, 0xD2, 0x58, 0xAC, 0x50, 0x7F,
    0xD6, 0x40, 0x6C, 0xFF, 0x14, 0x26, 0x6D, 0x31, 0x26, 0x6F, 0xEA, 0x1E,
    0x5C, 0x41, 0x56, 0x4B, 0x77, 0x7E, 0x69, 0x0F, 0x55, 0x04, 0xF2, 0x13,
    0x16, 0x02, 0x17, 0xB4, 0xB0, 0x1B, 0x88, 0x6A, 0x5E, 0x91, 0x54, 0x7F,
    0x9E, 0x27, 0x49, 0xF4, 0xD7, 0xFB, 0xD7, 0xD3, 0xB9, 0xA9, 0x2E, 0xE1,
    0x90, 0x9D, 0x0D, 0x22, 0x63, 0xF8, 0x0A, 0x76, 0xA6, 0xA2, 0x4C, 0x08,
    0x7A, 0x09, 0x1F, 0x53, 0x1D, 0xBF, 0x0A, 0x01, 0x69, 0xB6, 0xA2, 0x8A,
    0xD6, 0x62, 0xA4, 0xD1, 0x8E, 0x73, 0xAF, 0xA3, 0x2D, 0x77, 0x9D, 0x59,
    0x18, 0xD0, 0x8B, 0xC8, 0x85, 0x8F, 0x4D, 0xCE, 0xF9, 0x7C, 0x2A, 0x24,
    0x85, 0x5E, 0x6E, 0xEB, 0x22, 0xB3, 0xB2, 0xE5,
};

int SetupDH(SSL_CTX* ctx, const std::string& dh_file)
{
    scoped_ptr(dh, DH, NULL, DH_free);

    if (!dh_file.empty())
    {
        scoped_ptr(bio, BIO, BIO_new_file(dh_file.c_str(), "r"), BIO_free);

        if (bio.get())
            dh.reset(PEM_read_bio_DHparams(bio.get(), NULL, NULL, NULL));
    }
    else
    {
        dh.reset(DH_new());

        if (dh.get())
        {
            BIGNUM *dh_p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), NULL);
            BIGNUM *dh_g = BN_bin2bn(dh1024_g, sizeof(dh1024_g), NULL);

            if (dh_p == NULL || dh_g == NULL)
            {
                BN_free(dh_p);
                BN_free(dh_g);
                return 0;
            }

            DH_set0_pqg(dh.get(), dh_p, NULL, dh_g);
        }
    }

    if (dh.get() == NULL)
        return 0;

    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE);
    return static_cast<int>(SSL_CTX_set_tmp_dh(ctx, dh.get()));
}

#endif

#ifndef OPENSSL_NO_ECDH
void SetupECDH(SSL_CTX* ctx)
{
    //ECDHE is enabled only on OpenSSL 1.0.0e and later
    if (SSLeay() < 0x1000005fL)
        return;

    EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE);
    SSL_CTX_set_tmp_ecdh(ctx, ecdh);
    EC_KEY_free(ecdh);
}
#endif
