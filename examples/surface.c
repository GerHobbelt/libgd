#include "gd.h"
#include <stdio.h>
#include <stdlib.h>

void save_png(gdSurfacePtr surface, const char *filename)
{
    FILE *fp;
    fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "Can't save png image %s\n", filename);
        return;
    }
    gdSurfacePng(surface, fp);
    fclose(fp);
}

int main()
{
    gdSurface *surface;
    unsigned char *data;
    surface = gdSurfaceCreate(500, 500, GD_SURFACE_ARGB32);

    // If this data is somehow passed around and used in other places
    // caller is responsible to call gdSurfaceAddRef to keep it from
    // being destroyed. gdSurfaceDestroy decreases the refcount, when reach
    // 0 it is actually freed
    data = gdSurfaceGetData(surface);
    int x = 100;
    for (int y = 100; y < 150; y++)
    {
        unsigned int *img = (unsigned int *)(data + surface->stride * y);
        for (int x = 100; x < 150; x++)
        {
            img[x] = 0x80FF0000;
        }
    }
    for (int y = 200; y < 350; y++)
    {
        unsigned int *img = (unsigned int *)(data + surface->stride * y);
        for (int x = 200; x < 350; x++)
        {
            img[x] = 0xFF00FF00;
        }
    }
    for (int y = 400; y < 450; y++)
    {
        unsigned int *img = (unsigned int *)(data + surface->stride * y);
        for (int x = 400; x < 450; x++)
        {
            img[x] = 0x300000FF;
        }
    }
    if (!surface)
    {
        fprintf(stderr, "Can't create 400x400 surface\n");
        return 1;
    }

    save_png(surface, "surface.png");

    gdSurfaceDestroy(surface);
    return 0;
}