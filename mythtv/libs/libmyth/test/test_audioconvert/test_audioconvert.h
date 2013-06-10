/*
 *  Class TestAudioConvert
 *
 *  Copyright (C) Bubblestuff Pty Ltd 2013
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <QtTest/QtTest>

#include "mythcorecontext.h"
#include "audioconvert.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define MSKIP(MSG) QSKIP(MSG, SkipSingle)
#else
#define MSKIP(MSG) QSKIP(MSG)
#endif

#define AOALIGN(x) (((long)&x + 15) & ~0xf);

#define SSEALIGN 16     // for 16 bytes memory alignment

#define ISIZEOF(type) ((int)sizeof(type))

class TestAudioConvert: public QObject
{
    Q_OBJECT

  private slots:
    // called at the beginning of these sets of tests
    void initTestCase(void)
    {
        gCoreContext = new MythCoreContext("bin_version", NULL);
    }

    // test s16 -> float -> s16
    void Identical(void)
    {
        int    SIZEARRAY    = (INT16_MAX - INT16_MIN);
        short* arrays1      = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));
        short* arrays2      = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));

        short j = INT16_MIN;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j;
        }

        AudioConvert ac = AudioConvert(FORMAT_S16, FORMAT_S16);

        int val1 = ac.Process(arrays2, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
        }
        av_free(arrays1);
        av_free(arrays2);
    }

    // test s16 -> float -> s16 is lossless
    void S16ToFloat(void)
    {
        int    SIZEARRAY    = (INT16_MAX - INT16_MIN);
        short* arrays1      = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));
        short* arrays2      = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));
        float* arrayf       = (float*)av_malloc(SIZEARRAY * ISIZEOF(float));

        short j = INT16_MIN;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j;
        }

        AudioConvert acf = AudioConvert(FORMAT_S16, FORMAT_FLT);
        AudioConvert acs = AudioConvert(FORMAT_FLT, FORMAT_S16);

        int val1 = acf.Process(arrayf, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrayf[0]));
        int val2 = acs.Process(arrays2, arrayf, SIZEARRAY * ISIZEOF(arrayf[0]));
        QCOMPARE(val2, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
        }

        av_free(arrays1);
        av_free(arrays2);
        av_free(arrayf);
    }

    // test S16 -> S24LSB -> S16 is lossless
    void S16ToS24LSB(void)
    {
        int SIZEARRAY       = (INT16_MAX - INT16_MIN);

        short*   arrays1    = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));
        short*   arrays2    = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));
        int32_t* arrays24   = (int32_t*)av_malloc(SIZEARRAY * ISIZEOF(int32_t));

        short j = INT16_MIN;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j;
        }

        AudioConvert ac24   = AudioConvert(FORMAT_S16, FORMAT_S24LSB);
        AudioConvert acs    = AudioConvert(FORMAT_S24LSB, FORMAT_S16);

        int val1 = ac24.Process(arrays24, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrays24[0]));
        int val2 = acs.Process(arrays2, arrays24, SIZEARRAY * ISIZEOF(arrays24[0]));
        QCOMPARE(val2, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
            // Check we are indeed getting a 24 bits int
            QVERIFY(arrays24[i] >= -(2<<23 + 1));
            QVERIFY(arrays24[i] <= (2<<23));
        }

        av_free(arrays1);
        av_free(arrays2);
        av_free(arrays24);
    }

    void S24LSBToS32(void)
    {
        int SIZEARRAY       = (INT16_MAX - INT16_MIN);

        int32_t*   arrays1  = (int32_t*)av_malloc((SIZEARRAY+1) * ISIZEOF(int32_t));
        int32_t*   arrays2  = (int32_t*)av_malloc((SIZEARRAY+1) * ISIZEOF(int32_t));
        int32_t*   arrays32 = (int32_t*)av_malloc((SIZEARRAY+1) * ISIZEOF(int32_t));

        short j = INT16_MIN;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j;
        }

        AudioConvert ac24   = AudioConvert(FORMAT_S24LSB, FORMAT_S32);
        AudioConvert acs    = AudioConvert(FORMAT_S32, FORMAT_S24LSB);

        int val1 = ac24.Process(arrays32, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrays32[0]));
        int val2 = acs.Process(arrays2, arrays32, SIZEARRAY * ISIZEOF(arrays32[0]));
        QCOMPARE(val2, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
        }

        av_free(arrays1);
        av_free(arrays2);
        av_free(arrays32);
    }

    // test S16 -> S24 -> S16 is lossless
    void S16ToS24(void)
    {
        int SIZEARRAY       = (INT16_MAX - INT16_MIN);

        short*   arrays1    = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));
        short*   arrays2    = (short*)av_malloc(SIZEARRAY * ISIZEOF(short));
        int32_t* arrays24   = (int32_t*)av_malloc(SIZEARRAY * ISIZEOF(int32_t));

        short j = INT16_MIN;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j << 8;
        }

        AudioConvert ac24   = AudioConvert(FORMAT_S16, FORMAT_S24);
        AudioConvert acs    = AudioConvert(FORMAT_S24, FORMAT_S16);

        int val1 = ac24.Process(arrays24, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrays24[0]));
        int val2 = acs.Process(arrays2, arrays24, SIZEARRAY * ISIZEOF(arrays24[0]));
        QCOMPARE(val2, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
            // Check we are indeed getting a 24 bits int
            QCOMPARE(arrays24[i] & ~0xffff, arrays24[i]);
        }

        av_free(arrays1);
        av_free(arrays2);
        av_free(arrays24);
    }

    void S24ToS32(void)
    {
        int SIZEARRAY       = (INT16_MAX - INT16_MIN);

        int32_t*   arrays1  = (int32_t*)av_malloc((SIZEARRAY+1) * ISIZEOF(int32_t));
        int32_t*   arrays2  = (int32_t*)av_malloc((SIZEARRAY+1) * ISIZEOF(int32_t));
        int32_t*   arrays32 = (int32_t*)av_malloc((SIZEARRAY+1) * ISIZEOF(int32_t));

        short j = INT16_MIN;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j << 8;
        }

        AudioConvert ac32   = AudioConvert(FORMAT_S24, FORMAT_S32);
        AudioConvert acs    = AudioConvert(FORMAT_S32, FORMAT_S24);

        int val1 = ac32.Process(arrays32, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrays32[0]));
        int val2 = acs.Process(arrays2, arrays32, SIZEARRAY * ISIZEOF(arrays32[0]));
        QCOMPARE(val2, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
        }

        av_free(arrays1);
        av_free(arrays2);
        av_free(arrays32);
    }

    // test S16 -> S24 -> S16 is lossless
    void S16ToS32(void)
    {
        int SIZEARRAY       = (INT16_MAX - INT16_MIN);

        short*   arrays1    = (short*)av_malloc((SIZEARRAY+1) * ISIZEOF(short));
        short*   arrays2    = (short*)av_malloc((SIZEARRAY+1) * ISIZEOF(short));
        int32_t* arrays32   = (int32_t*)av_malloc((SIZEARRAY+1) * ISIZEOF(int32_t));

        short j = INT16_MIN;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j;
        }

        AudioConvert ac32   = AudioConvert(FORMAT_S16, FORMAT_S32);
        AudioConvert acs    = AudioConvert(FORMAT_S32, FORMAT_S16);

        int val1 = ac32.Process(arrays32, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrays32[0]));
        int val2 = acs.Process(arrays2, arrays32, SIZEARRAY * ISIZEOF(arrays32[0]));
        QCOMPARE(val2, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
        }

        av_free(arrays1);
        av_free(arrays2);
        av_free(arrays32);
    }

    // test U8 -> S16 -> U8 is lossless
    void U8ToS16(void)
    {
        int SIZEARRAY       = 256;

        uchar*   arrays1    = (uchar*)av_malloc((SIZEARRAY+1) * ISIZEOF(uchar));
        uchar*   arrays2    = (uchar*)av_malloc((SIZEARRAY+1) * ISIZEOF(uchar));
        short*  arrays32    = (short*)av_malloc((SIZEARRAY+1) * ISIZEOF(short));

        uchar j = 0;
        for (int i = 0; i < SIZEARRAY; i++, j++)
        {
            arrays1[i] = j;
        }

        AudioConvert ac32   = AudioConvert(FORMAT_U8, FORMAT_S16);
        AudioConvert acs    = AudioConvert(FORMAT_S16, FORMAT_U8);

        int val1 = ac32.Process(arrays32, arrays1, SIZEARRAY * ISIZEOF(arrays1[0]));
        QCOMPARE(val1, SIZEARRAY * ISIZEOF(arrays32[0]));
        int val2 = acs.Process(arrays2, arrays32, SIZEARRAY * ISIZEOF(arrays32[0]));
        QCOMPARE(val2, SIZEARRAY * ISIZEOF(arrays2[0]));
        for (int i = 0; i < SIZEARRAY; i++)
        {
            QCOMPARE(arrays1[i], arrays2[i]);
        }

        av_free(arrays1);
        av_free(arrays2);
        av_free(arrays32);
    }
};
