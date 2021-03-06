/**
 *
 *            L'Université Jean Monnet
 *
 *         Dimitrios Soupilas * Riad Lazli
 *                Aleksei Pashinin
 *
 *                  Projet SAI
 *
 *         made by Zavie (Julien Guertault)
 *
 */


#include <stdlib.h>
#include "eau.h"

#define		MOD	0xff

static float surface[6 * RESOLUTION * (RESOLUTION + 1)];
static float normal[6 * RESOLUTION * (RESOLUTION + 1)];
static GLuint texture;


static float zf(const float x, const float y, const float t) {
    const float x2 = x - 3;
    const float y2 = y + 1;
    const float xx = x2 * x2;
    const float yy = y2 * y2;
    return ((2 * sinf(20 * sqrtf(xx + yy) - 4 * t) +
             Noise(10 * x, 10 * y, t, 0)) / 200);
}


static int		permut[256];
static const char	gradient[32][4] =
  {
    { 1,  1,  1,  0}, { 1,  1,  0,  1}, { 1,  0,  1,  1}, { 0,  1,  1,  1},
    { 1,  1, -1,  0}, { 1,  1,  0, -1}, { 1,  0,  1, -1}, { 0,  1,  1, -1},
    { 1, -1,  1,  0}, { 1, -1,  0,  1}, { 1,  0, -1,  1}, { 0,  1, -1,  1},
    { 1, -1, -1,  0}, { 1, -1,  0, -1}, { 1,  0, -1, -1}, { 0,  1, -1, -1},
    {-1,  1,  1,  0}, {-1,  1,  0,  1}, {-1,  0,  1,  1}, { 0, -1,  1,  1},
    {-1,  1, -1,  0}, {-1,  1,  0, -1}, {-1,  0,  1, -1}, { 0, -1,  1, -1},
    {-1, -1,  1,  0}, {-1, -1,  0,  1}, {-1,  0, -1,  1}, { 0, -1, -1,  1},
    {-1, -1, -1,  0}, {-1, -1,  0, -1}, {-1,  0, -1, -1}, { 0, -1, -1, -1},
};


void		InitNoise (void)
{
  unsigned int i = 0;
  while (i < 256)
    permut[i++] = rand () & MOD;
}

/*
** Function finding out the gradient corresponding to the coordinates
*/
static int	Indice (const int i,
			const int j,
			const int k,
			const int l)
{
  return (permut[(l + permut[(k + permut[(j + permut[i & MOD])
					 & MOD])
			     & MOD])
		 & MOD]
	  & 0x1f);
}

/*
** Functions computing the dot product of the vector and the gradient
*/
static float	Prod (const float a, const char b)
{
  if (b > 0)
    return a;
  if (b < 0)
    return -a;
  return 0;
}

static float	Dot_prod (const float x1, const char x2,
			  const float y1, const char y2,
			  const float z1, const char z2,
			  const float t1, const char t2)
{
  return (Prod (x1, x2) + Prod (y1, y2) + Prod (z1, z2) + Prod (t1, t2));
}

/*
** Functions computing interpolations
*/
static float	Spline5 (const float state)
{
  /*
  ** Enhanced spline :
  ** (3x^2 + 2x^3) is not as good as (6x^5 - 15x^4 + 10x^3)
  */
  const float sqr = state * state;
  return state * sqr * (6 * sqr - 15 * state + 10);
}

static float	Linear (const float start,
			const float end,
			const float state)
{
  return start + (end - start) * state;
}

/*
** Noise function, returning the Perlin Noise at a given point
*/
float		Noise (const float x,
		       const float y,
		       const float z,
		       const float t)
{
  /* The unit hypercube containing the point */
  const int x1 = (int) (x > 0 ? x : x - 1);
  const int y1 = (int) (y > 0 ? y : y - 1);
  const int z1 = (int) (z > 0 ? z : z - 1);
  const int t1 = (int) (t > 0 ? t : t - 1);
  const int x2 = x1 + 1;
  const int y2 = y1 + 1;
  const int z2 = z1 + 1;
  const int t2 = t1 + 1;

  /* The 16 corresponding gradients */
  const char * g0000 = gradient[Indice (x1, y1, z1, t1)];
  const char * g0001 = gradient[Indice (x1, y1, z1, t2)];
  const char * g0010 = gradient[Indice (x1, y1, z2, t1)];
  const char * g0011 = gradient[Indice (x1, y1, z2, t2)];
  const char * g0100 = gradient[Indice (x1, y2, z1, t1)];
  const char * g0101 = gradient[Indice (x1, y2, z1, t2)];
  const char * g0110 = gradient[Indice (x1, y2, z2, t1)];
  const char * g0111 = gradient[Indice (x1, y2, z2, t2)];
  const char * g1000 = gradient[Indice (x2, y1, z1, t1)];
  const char * g1001 = gradient[Indice (x2, y1, z1, t2)];
  const char * g1010 = gradient[Indice (x2, y1, z2, t1)];
  const char * g1011 = gradient[Indice (x2, y1, z2, t2)];
  const char * g1100 = gradient[Indice (x2, y2, z1, t1)];
  const char * g1101 = gradient[Indice (x2, y2, z1, t2)];
  const char * g1110 = gradient[Indice (x2, y2, z2, t1)];
  const char * g1111 = gradient[Indice (x2, y2, z2, t2)];

  /* The 16 vectors */
  const float dx1 = x - x1;
  const float dx2 = x - x2;
  const float dy1 = y - y1;
  const float dy2 = y - y2;
  const float dz1 = z - z1;
  const float dz2 = z - z2;
  const float dt1 = t - t1;
  const float dt2 = t - t2;

  /* The 16 dot products */
  const float b0000 = Dot_prod(dx1, g0000[0], dy1, g0000[1],
			       dz1, g0000[2], dt1, g0000[3]);
  const float b0001 = Dot_prod(dx1, g0001[0], dy1, g0001[1],
			       dz1, g0001[2], dt2, g0001[3]);
  const float b0010 = Dot_prod(dx1, g0010[0], dy1, g0010[1],
			       dz2, g0010[2], dt1, g0010[3]);
  const float b0011 = Dot_prod(dx1, g0011[0], dy1, g0011[1],
			       dz2, g0011[2], dt2, g0011[3]);
  const float b0100 = Dot_prod(dx1, g0100[0], dy2, g0100[1],
			       dz1, g0100[2], dt1, g0100[3]);
  const float b0101 = Dot_prod(dx1, g0101[0], dy2, g0101[1],
			       dz1, g0101[2], dt2, g0101[3]);
  const float b0110 = Dot_prod(dx1, g0110[0], dy2, g0110[1],
			       dz2, g0110[2], dt1, g0110[3]);
  const float b0111 = Dot_prod(dx1, g0111[0], dy2, g0111[1],
			       dz2, g0111[2], dt2, g0111[3]);
  const float b1000 = Dot_prod(dx2, g1000[0], dy1, g1000[1],
			       dz1, g1000[2], dt1, g1000[3]);
  const float b1001 = Dot_prod(dx2, g1001[0], dy1, g1001[1],
			       dz1, g1001[2], dt2, g1001[3]);
  const float b1010 = Dot_prod(dx2, g1010[0], dy1, g1010[1],
			       dz2, g1010[2], dt1, g1010[3]);
  const float b1011 = Dot_prod(dx2, g1011[0], dy1, g1011[1],
			       dz2, g1011[2], dt2, g1011[3]);
  const float b1100 = Dot_prod(dx2, g1100[0], dy2, g1100[1],
			       dz1, g1100[2], dt1, g1100[3]);
  const float b1101 = Dot_prod(dx2, g1101[0], dy2, g1101[1],
			       dz1, g1101[2], dt2, g1101[3]);
  const float b1110 = Dot_prod(dx2, g1110[0], dy2, g1110[1],
			       dz2, g1110[2], dt1, g1110[3]);
  const float b1111 = Dot_prod(dx2, g1111[0], dy2, g1111[1],
			       dz2, g1111[2], dt2, g1111[3]);

  /* Then the interpolations, down to the result */
  const float idx1 = Spline5 (dx1);
  const float idy1 = Spline5 (dy1);
  const float idz1 = Spline5 (dz1);
  const float idt1 = Spline5 (dt1);

  const float b111 = Linear (b1110, b1111, idt1);
  const float b110 = Linear (b1100, b1101, idt1);
  const float b101 = Linear (b1010, b1011, idt1);
  const float b100 = Linear (b1000, b1001, idt1);
  const float b011 = Linear (b0110, b0111, idt1);
  const float b010 = Linear (b0100, b0101, idt1);
  const float b001 = Linear (b0010, b0011, idt1);
  const float b000 = Linear (b0000, b0001, idt1);

  const float b11 = Linear (b110, b111, idz1);
  const float b10 = Linear (b100, b101, idz1);
  const float b01 = Linear (b010, b011, idz1);
  const float b00 = Linear (b000, b001, idz1);

  const float b1 = Linear (b10, b11, idy1);
  const float b0 = Linear (b00, b01, idy1);

  return Linear (b0, b1, idx1);
}

/* ========================================================================= */

int load_texture(const char *filename,
                 unsigned char *dest,
                 const int format,
                 const unsigned int size) {
    FILE *fd;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *line;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress (&cinfo);

    if (0 == (fd = fopen(filename, "rb")))
        return 1;

    jpeg_stdio_src(&cinfo, fd);
    jpeg_read_header(&cinfo, TRUE);
    if ((cinfo.image_width != size) || (cinfo.image_height != size))
        return 1;

    if (GL_RGB == format) {
        if (cinfo.out_color_space == JCS_GRAYSCALE)
            return 1;
    } else if (cinfo.out_color_space != JCS_GRAYSCALE)
        return 1;

    jpeg_start_decompress(&cinfo);

    while (cinfo.output_scanline < cinfo.output_height) {
        line = dest +
               (GL_RGB == format ? 3 * size : size) * cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, &line, 1);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return 0;
}

void DessinerOcean(void) {
    unsigned char total_texture[4 * 256 * 256];
    unsigned char alpha_texture[256 * 256];
    unsigned char caustic_texture[3 * 256 * 256];
    const float t = glutGet(GLUT_ELAPSED_TIME) / 1000.;
    const float delta = 2. / RESOLUTION * 19;
    const unsigned int length = 2 * (RESOLUTION + 1);
    const float xn = (RESOLUTION + 1) * delta + 1;
    unsigned int i;
    unsigned int j;
    float x;
    float y;
    unsigned int indice;
    unsigned int preindice;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glGenTextures(1, &texture);
    if (load_texture("textures/alpha.jpg", alpha_texture, GL_ALPHA, 256) != 0 ||
        load_texture("textures/reflection.jpg", caustic_texture, GL_RGB, 256) != 0)
        //printf("Erreur");
    for (i = 0; i < 256 * 256; i++) {
        total_texture[4 * i] = caustic_texture[3 * i];
        total_texture[4 * i + 1] = caustic_texture[3 * i + 1];
        total_texture[4 * i + 2] = caustic_texture[3 * i + 2];
        total_texture[4 * i + 3] = alpha_texture[i];
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 256, 256, GL_RGBA,
                      GL_UNSIGNED_BYTE, total_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    /* Yes, I know, this is quite ugly... */
    float v1x;
    float v1y;
    float v1z;

    float v2x;
    float v2y;
    float v2z;

    float v3x;
    float v3y;
    float v3z;

    float vax;
    float vay;
    float vaz;

    float vbx;
    float vby;
    float vbz;

    float nx;
    float ny;
    float nz;

    float l;


    /* Vertices */
    for (j = 0; j < RESOLUTION; j++) {
        y = (j + 1) * delta - 1;
        for (i = 0; i <= RESOLUTION; i++) {
            indice = 6 * (i + j * (RESOLUTION + 1));

            x = i * delta - 1;
            surface[indice + 3] = x;
            surface[indice + 4] = zf(x, y, t);
            surface[indice + 5] = y;
            if (j != 0) {
                /* Values were computed during the previous loop */
                preindice = 6 * (i + (j - 1) * (RESOLUTION + 1));
                surface[indice] = surface[preindice + 3];
                surface[indice + 1] = surface[preindice + 4];
                surface[indice + 2] = surface[preindice + 5];
            } else {
                surface[indice] = x;
                surface[indice + 1] = zf(x, -1, t);
                surface[indice + 2] = -1;
            }
        }
    }

    /* Normals */
    for (j = 0; j < RESOLUTION; j++)
        for (i = 0; i <= RESOLUTION; i++) {
            indice = 6 * (i + j * (RESOLUTION + 1));

            v1x = surface[indice + 3];
            v1y = surface[indice + 4];
            v1z = surface[indice + 5];

            v2x = v1x;
            v2y = surface[indice + 1];
            v2z = surface[indice + 2];

            if (i < RESOLUTION) {
                v3x = surface[indice + 9];
                v3y = surface[indice + 10];
                v3z = v1z;
            } else {
                v3x = xn;
                v3y = zf(xn, v1z, t);
                v3z = v1z;
            }

            vax = v2x - v1x;
            vay = v2y - v1y;
            vaz = v2z - v1z;

            vbx = v3x - v1x;
            vby = v3y - v1y;
            vbz = v3z - v1z;

            nx = (vby * vaz) - (vbz * vay);
            ny = (vbz * vax) - (vbx * vaz);
            nz = (vbx * vay) - (vby * vax);

            l = sqrtf(nx * nx + ny * ny + nz * nz);
            if (l != 0) {
                l = 1 / l;
                normal[indice + 3] = nx * l;
                normal[indice + 4] = ny * l;
                normal[indice + 5] = nz * l;
            } else {
                normal[indice + 3] = 0;
                normal[indice + 4] = 1;
                normal[indice + 5] = 0;
            }


            if (j != 0) {
                /* Values were computed during the previous loop */
                preindice = 6 * (i + (j - 1) * (RESOLUTION + 1));
                normal[indice] = normal[preindice + 3];
                normal[indice + 1] = normal[preindice + 4];
                normal[indice + 2] = normal[preindice + 5];
            } else {
                v1y = zf(v1x, (j - 1) * delta - 1, t);
                v1z = (j - 1) * delta - 1;

                v3y = zf(v3x, v2z, t);
                v3z = v2z;

                vax = v1x - v2x;
                vay = v1y - v2y;
                vaz = v1z - v2z;

                vbx = v3x - v2x;
                vby = v3y - v2y;
                vbz = v3z - v2z;

                nx = (vby * vaz) - (vbz * vay);
                ny = (vbz * vax) - (vbx * vaz);
                nz = (vbx * vay) - (vby * vax);

                l = sqrtf(nx * nx + ny * ny + nz * nz);
                if (l != 0) {
                    l = 1 / l;
                    normal[indice] = nx * l;
                    normal[indice + 1] = ny * l;
                    normal[indice + 2] = nz * l;
                } else {
                    normal[indice] = 0;
                    normal[indice + 1] = 1;
                    normal[indice + 2] = 0;
                }
            }
        }

    /* The water */
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normal);
    glVertexPointer(3, GL_FLOAT, 0, surface);
    glTranslatef(-74.0, 0, -76.0);
    int k;
    for (k = 0; k < 2; k++) {
        for (j = 0; j < 10; j++) {
            for (i = 0; i < RESOLUTION; i++) {
                glDrawArrays(GL_TRIANGLE_STRIP, i * length, length);
            }
            glTranslatef(37.95, 0, 0.0);
        }
        glTranslatef(-379.5, 0, 37.95);
    }
    glTranslatef(-2.0, 0, 0);
    for (k = 0; k < 7; k++) {
        for (j = 0; j < 2; j++) {
            for (i = 0; i < RESOLUTION; i++) {
                glDrawArrays(GL_TRIANGLE_STRIP, i * length, length);
            }
            glTranslatef(37.95, 0, 0.0);
        }
        glTranslatef(-37.95 * 2, 0, 37.95);
    }
    glTranslatef(379.5 - 37.95 * 2 - 26, 0, -7 * 37.95);
    for (k = 0; k < 8; k++) {
        for (j = 0; j < 2; j++) {
            for (i = 0; i < RESOLUTION; i++) {
                glDrawArrays(GL_TRIANGLE_STRIP, i * length, length);
            }
            glTranslatef(37.95, 0, 0.0);
        }
        glTranslatef(-37.95 * 2, 0, 37.95);
    }
    glTranslatef(-379.5 / 2 - 12/*+37.95*2+26*/, 0, -37.95 * 2 - 25);
    for (k = 0; k < 3; k++) {
        for (j = 0; j < 6; j++) {
            for (i = 0; i < RESOLUTION; i++) {
                glDrawArrays(GL_TRIANGLE_STRIP, i * length, length);
            }
            glTranslatef(37.95, 0, 0.0);
        }
        glTranslatef(-37.95 * 6, 0, 37.95);
    }
    glPopMatrix();
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
}