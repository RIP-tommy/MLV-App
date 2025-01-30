/*!
 * \file cube_lut.c
 * \author masc4ii
 * \copyright 2018
 * \brief this module allows loading 1d/3d luts (.cube) and applies them on a picture
 */

#include "cube_lut.h"
#include <stdlib.h>
#include <stdio.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define LIMIT16(X) MAX(MIN(X, 65535), 0)

FILE* openFileWithQFile(const char* filePath, const char* mode);

//Switch here between trilinear and tetrahedral interpolation
//#define USE_TRILIN_INT

// //////////////
//Interpolation functions
float lerp(float x, float x1, float x2, float q00, float q01) {
    if( ( x2 - x1 ) == 0 ) return q00;
    else return ( ( ( x2 - x ) / ( x2 - x1 ) ) * q00 ) + ( ( ( x - x1 ) / ( x2 - x1 ) ) * q01 );
}

float biLerp(float x, float y, float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2) {
  float r1 = lerp(x, x1, x2, q11, q21);
  float r2 = lerp(x, x1, x2, q12, q22);

  return lerp(y, y1, y2, r1, r2);
}

float triLerp(float x, float y, float z, float q000, float q001, float q010, float q011, float q100, float q101, float q110, float q111, float x1, float x2, float y1, float y2, float z1, float z2) {
  float x00 = lerp(x, x1, x2, q000, q100);
  float x10 = lerp(x, x1, x2, q010, q110);
  float x01 = lerp(x, x1, x2, q001, q101);
  float x11 = lerp(x, x1, x2, q011, q111);
  float r0 = lerp(y, y1, y2, x00, x10);
  float r1 = lerp(y, y1, y2, x01, x11);

  return lerp(z, z1, z2, r0, r1);
}
// //////////////

//Initialize LUT object
lut_t * init_lut( void )
{
    lut_t *lut = calloc( 1, sizeof( lut_t ) );
    lut->intensity = 100;
    return lut;
}

//Unload the whole lut object
void free_lut(lut_t *lut)
{
    if( lut )
    {
        unload_lut( lut );
        free( lut );
    }
}

//Load the LUT
int load_lut( lut_t *lut, char *filename, char *error_message )
{
    unsigned int i = 0;
    char line[250];
    uint32_t lut_size = 0;
    float r, g, b;
    float inMin, inMax;

    FILE *fp;
#ifdef __ANDROID__
    fp = openFileWithQFile(filename, "r");
#else
    fp = fopen( filename, "r" );
#endif

    //Settings standard parameters, which mustn't be in cube file
    for( int i = 0; i < 3; i++ )
    {
        lut->domain_min[i] = 0.0;
        lut->domain_max[i] = 1.0;
    }

    while( fgets (line, 250, fp) != NULL ) //No more than 250 characters on the line, cube specification
    {
        if(line[0] == '#' || line[0] == '\n' || line[0] == '\r') //Comment, just skip this line
        {
#ifndef STDOUT_SILENT
            printf("Comment line found and ignored\n");
#endif
            continue;
        }
        else if( sscanf(line, "TITLE%*[ \t]%[^\n]", lut->title ) == 1) //Title is set
        {
#ifndef STDOUT_SILENT
            printf("TITLE %s\n", lut->title);
#endif
            continue;
        }
        else if( sscanf(line, "LUT_1D_SIZE%*[ \t]%hu%*[^\n]", &lut->dimension) == 1) //LUT is 1D
        {
#ifndef STDOUT_SILENT
            printf("LUT_1D_SIZE %u\n", lut->dimension);
#endif
            lut_size = lut->dimension * 3;
            lut->is3d = 0;
            lut->cube = malloc( lut_size * sizeof( float ) );
            continue;
        }
        else if( sscanf(line, "LUT_3D_SIZE%*[ \t]%hu%*[^\n]", &lut->dimension) == 1) //LUT is 3D
        {
#ifndef STDOUT_SILENT
            printf("LUT_3D_SIZE %u\n", lut->dimension);
#endif
            lut_size = (uint32_t)lut->dimension * (uint32_t)lut->dimension * (uint32_t)lut->dimension * 3;
            lut->is3d = 1;
            lut->cube = malloc( lut_size * sizeof( float ) );
            continue;
        }
        else if( sscanf(line, "%f%*[ \t]%f%*[ \t]%f%*[^\n]", &r, &g, &b ) == 3) //Read data
        {
            if(!lut_size || i >= lut_size) //File with invalid header or file is too long
            {
                sprintf(error_message, "File with invalid header or file is too long.");
#ifndef STDOUT_SILENT
                printf("%s\n", error_message);
#endif
                unload_lut( lut );
                fclose( fp );
                return -1;
            }
#ifndef STDOUT_SILENT
            printf("Data line #%d values: r = %f, g = %f, b = %f\n", i/3, r, g, b);
#endif
            lut->cube[i+0] = r;
            lut->cube[i+1] = g;
            lut->cube[i+2] = b;
            i+=3;
        }
        else if( sscanf(line, "DOMAIN_MIN%*[ \t]%f%*[ \t]%f%*[ \t]%f%*[^\n]", &lut->domain_min[0], &lut->domain_min[1], &lut->domain_min[2]) == 3) //Read domain min values
        {
#ifndef STDOUT_SILENT
            printf("DOMAIN_MIN %f %f %f\n", lut->domain_min[0], lut->domain_min[1], lut->domain_min[2]);
#endif
            continue;
        }
        else if( sscanf(line, "DOMAIN_MAX%*[ \t]%f%*[ \t]%f%*[ \t]%f%*[^\n]", &lut->domain_max[0], &lut->domain_max[1], &lut->domain_max[2]) == 3) //Read domain max values
        {
#ifndef STDOUT_SILENT
            printf("DOMAIN_MAX %f %f %f\n", lut->domain_max[0], lut->domain_max[1], lut->domain_max[2]);
#endif
            continue;
        }
        else if( sscanf(line, "LUT_1D_INPUT_RANGE%*[ \t]%f%*[ \t]%f%*[^\n]", &inMin, &inMax) == 2) //Read input range values (Resolve created), do nothing with it, because it is not in the specs
        {
#ifndef STDOUT_SILENT
            printf("LUT_1D_INPUT_RANGE %f %f\n", &inMin, &inMax);
#endif
            continue;
        }
        else if( sscanf(line, "LUT_3D_INPUT_RANGE%*[ \t]%f%*[ \t]%f%*[^\n]", &inMin, &inMax) == 2) //Read input range values (Resolve created), do nothing with it, because it is not in the specs
        {
#ifndef STDOUT_SILENT
            printf("LUT_3D_INPUT_RANGE %f %f\n", &inMin, &inMax);
#endif
            continue;
        }
        else //Invalid file
        {
            sprintf(error_message, "Invalid file.");
#ifndef STDOUT_SILENT
            printf("%s\n", error_message);
#endif
            unload_lut( lut );
            fclose( fp );
            return -2;
        }
    }

    if(i < lut_size) //File is too short
    {
        sprintf(error_message, "File too short.");
#ifndef STDOUT_SILENT
        printf("%s\n", error_message);
#endif
        unload_lut( lut );
        fclose( fp );
        return -3;
    }

    fclose( fp );
    return 0;
}

//Unload the LUT
void unload_lut( lut_t *lut )
{
    if( !lut ) return;
    if( lut->dimension == 0 ) return;
    if( lut->cube ) free( lut->cube );
    lut->dimension = 0;
}

//Read out a point from the 1dlut (input: 0..dimension-1)
float getLut1dPoint( lut_t *lut, uint16_t inPos, uint8_t outChannel )
{
    if( inPos >= lut->dimension ) inPos = lut->dimension - 1;
    return lut->cube[ inPos * 3 + outChannel ];
}

//Read out a point from the 3dlut (input: 0..dimension-1)
float getLut3dPoint( lut_t *lut, uint16_t rIn, uint16_t gIn, uint16_t bIn, uint8_t outChannel )
{
    if( rIn >= lut->dimension ) rIn = lut->dimension - 1;
    if( gIn >= lut->dimension ) gIn = lut->dimension - 1;
    if( bIn >= lut->dimension ) bIn = lut->dimension - 1;
    uint32_t red0_offs = rIn;
    uint32_t green0_offs = gIn * lut->dimension;
    uint32_t blue0_offs = bIn * lut->dimension * lut->dimension;
    uint32_t offset0 = ( red0_offs + green0_offs + blue0_offs ) * 3;

    return lut->cube[ offset0 + outChannel ];
}

//Apply LUT on picture
void apply_lut(lut_t *lut, int width, int height, uint16_t *image)
{
    if( lut->dimension <= 1 || !lut->cube ) return;
    if( lut->intensity > 100 ) lut->intensity = 100;

    uint16_t * end = image + (width * height * 3);
    float factor1 = (float)lut->intensity / 100.0f;
    float factor2 = 1.0 - factor1;

    for (uint16_t * pix = image; pix < end; pix += 3)
    {
        //x
        float red   = ( pix[0] * ( lut->dimension - 1 ) / 65536.0 / ( lut->domain_max[0] - lut->domain_min[0] ) ) - lut->domain_min[0];
        float green = ( pix[1] * ( lut->dimension - 1 ) / 65536.0 / ( lut->domain_max[1] - lut->domain_min[1] ) ) - lut->domain_min[1];
        float blue  = ( pix[2] * ( lut->dimension - 1 ) / 65536.0 / ( lut->domain_max[2] - lut->domain_min[2] ) ) - lut->domain_min[2];

        //x0
        uint16_t r0 = (uint16_t)red;
        uint16_t g0 = (uint16_t)green;
        uint16_t b0 = (uint16_t)blue;
        //x1
        uint16_t r1 = r0 + 1;
        uint16_t g1 = g0 + 1;
        uint16_t b1 = b0 + 1;

        if( lut->is3d == 0 )
        {
            //Linear Interpolation
            //y0 & //y1
            float pix00 = getLut1dPoint( lut, r0, 0 );
            float pix01 = getLut1dPoint( lut, r1, 0 );
            float pix10 = getLut1dPoint( lut, g0, 1 );
            float pix11 = getLut1dPoint( lut, g1, 1 );
            float pix20 = getLut1dPoint( lut, b0, 2 );
            float pix21 = getLut1dPoint( lut, b1, 2 );
            //3x Linear Interpolation
            float a = lerp( red,   r0, r1, pix00, pix01 );
            float b = lerp( green, g0, g1, pix10, pix11 );
            float c = lerp( blue,  b0, b1, pix20, pix21 );
            //Output
            pix[0] = pix[0] * factor2 + LIMIT16( a * 65535.0 ) * factor1;
            pix[1] = pix[1] * factor2 + LIMIT16( b * 65535.0 ) * factor1;
            pix[2] = pix[2] * factor2 + LIMIT16( c * 65535.0 ) * factor1;
        }
        else
        {
#ifdef USE_TRILIN_INT
            //Trilinear Interpolation
            for( int i = 0; i < 3; i++ )
            {
                float q000 = getLut3dPoint( lut, r0, g0, b0, i );
                float q001 = getLut3dPoint( lut, r0, g0, b1, i );
                float q010 = getLut3dPoint( lut, r0, g1, b0, i );
                float q011 = getLut3dPoint( lut, r0, g1, b1, i );
                float q100 = getLut3dPoint( lut, r1, g0, b0, i );
                float q101 = getLut3dPoint( lut, r1, g0, b1, i );
                float q110 = getLut3dPoint( lut, r1, g1, b0, i );
                float q111 = getLut3dPoint( lut, r1, g1, b1, i );

                float out = triLerp(red, green, blue, q000, q001, q010, q011, q100, q101, q110, q111, r0, r1, g0, g1, b0, b1);

                //Output
                pix[i] = LIMIT16( out * 65535.0 );
            }
#else
            //Tetrahedral Interpolation
            red = red - r0;
            blue = blue - b0;
            green = green - g0;

            for( uint8_t i = 0; i < 3; i++ )
            {
                float q000 = getLut3dPoint( lut, r0, g0, b0, i );
                float q001 = getLut3dPoint( lut, r0, g0, b1, i );
                float q010 = getLut3dPoint( lut, r0, g1, b0, i );
                float q011 = getLut3dPoint( lut, r0, g1, b1, i );
                float q100 = getLut3dPoint( lut, r1, g0, b0, i );
                float q101 = getLut3dPoint( lut, r1, g0, b1, i );
                float q110 = getLut3dPoint( lut, r1, g1, b0, i );
                float q111 = getLut3dPoint( lut, r1, g1, b1, i );

                float out;
                if( green >= blue && blue >= red ) //T1
                {
                    out = ( (1.0 - green) * q000 ) + ( ( green - blue ) * q010 )
                                + ( ( blue - red ) * q011 ) + ( red * q111 );
                }
                else if( blue > red && red > green ) //T2
                {
                    out = ( (1.0 - blue) * q000 ) + ( ( blue - red ) * q001 )
                                + ( ( red - green ) * q101 ) + ( green * q111 );
                }
                else if( blue > green && green >= red ) //T3
                {
                    out = ( (1.0 - blue) * q000 ) + ( ( blue - green ) * q001 )
                                + ( ( green - red ) * q011 ) + ( red * q111 );
                }
                else if( red >= green && green > blue ) //T4
                {
                    out = ( (1.0 - red) * q000 ) + ( ( red - green ) * q100 )
                                + ( ( green - blue ) * q110 ) + ( blue * q111 );
                }
                else if( green > red && red >= blue ) //T5
                {
                    out = ( (1.0 - green) * q000 ) + ( ( green - red ) * q010 )
                                + ( ( red - blue ) * q110 ) + ( blue * q111 );
                }
                else //T6
                {
                    out = ( (1.0 - red) * q000 ) + ( ( red - blue ) * q100 )
                                + ( ( blue - green ) * q101 ) + ( green * q111 );
                }
                //Output
                pix[i] = pix[i] * factor2 + LIMIT16( out * 65535.0 ) * factor1;
            }
#endif
        }
    }
}
