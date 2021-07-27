#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gd.h"
#include "gd_intern.h"
#include "gdhelpers.h"
#include "gd_errors.h"

#include "gd_surface.h"
#include "gd_array.h"
#include "gd_span_rle.h"
#include "gd_path_matrix.h"
#include "gd_path.h"
#include "gd_path_dash.h"
#include "ftraster/gd_ft_raster.h"
#include "ftraster/gd_ft_stroker.h"
#include "ftraster/gd_ft_types.h"
#include "ftraster/gd_ft_math.h"

#define SQRT2 1.41421356237309504880

gdSpanRlePtr gdSpanRleCreate()
{
    gdSpanRlePtr rle = gdMalloc(sizeof(gdSpanRle));
    if (!rle)
        return NULL;
    _rle_spans_init(rle->spans);
    rle->x = 0;
    rle->y = 0;
    rle->w = 0;
    rle->h = 0;
    return rle;
}

void gdSpanRleDestroy(gdSpanRlePtr rle)
{
    if (rle == NULL)
        return;
    gdFree(rle->spans.data);
    gdFree(rle);
}

void gdSpanRleClear(gdSpanRlePtr rle)
{
    rle->spans.size = 0;
    rle->x = 0;
    rle->y = 0;
    rle->w = 0;
    rle->h = 0;
}

gdSpanRlePtr gdSpanRleClone(gdSpanRlePtr rle)
{
    if (rle == NULL) return NULL;

    gdSpanRlePtr clone = gdMalloc(sizeof(gdSpanRle));
    if (!clone) return NULL;

    _rle_spans_init(clone->spans);
    _rle_spans_allocate(clone->spans, rle->spans.size);
    memcpy(clone->spans.data, rle->spans.data, (size_t)rle->spans.size * sizeof(gdSpan));
    clone->spans.size = rle->spans.size;
    clone->x = rle->x;
    clone->y = rle->y;
    clone->w = rle->w;
    clone->h = rle->h;
    return clone;
}

#define DIV255(x) (((x) + ((x) >> 8) + 0x80) >> 8)
gdSpanRlePtr gdSpanHorizontalClip(const gdSpanRlePtr a, const gdSpanRlePtr b)
{
    gdSpanRlePtr result = gdMalloc(sizeof(gdSpanRle));
    if (!result)
    {
        return NULL;
    }

    _rle_spans_init(result->spans);
    _rle_spans_allocate(result->spans, MAX(a->spans.size, b->spans.size));

    gdSpanPtr a_spans = a->spans.data;
    gdSpanPtr a_end = a_spans + a->spans.size;

    gdSpanPtr b_spans = b->spans.data;
    gdSpanPtr b_end = b_spans + b->spans.size;

    while (a_spans < a_end && b_spans < b_end)
    {
        if (b_spans->y > a_spans->y)
        {
            ++a_spans;
            continue;
        }

        if (a_spans->y != b_spans->y)
        {
            ++b_spans;
            continue;
        }

        int ax1 = a_spans->x;
        int ax2 = ax1 + a_spans->len;
        int bx1 = b_spans->x;
        int bx2 = bx1 + b_spans->len;

        if (bx1 < ax1 && bx2 < ax1)
        {
            ++b_spans;
            continue;
        }

        if (ax1 < bx1 && ax2 < bx1)
        {
            ++a_spans;
            continue;
        }

        int x = MAX(ax1, bx1);
        int len = MIN(ax2, bx2) - x;
        if (len)
        {   
            gdSpanPtr span = result->spans.data + result->spans.size;
            span->x = (short)x;
            span->len = (unsigned short)len;
            span->y = a_spans->y;
            span->coverage = DIV255(a_spans->coverage * b_spans->coverage);
            result->spans.size += 1;
        }

        if (ax2 < bx2)
        {
            ++a_spans;
        }
        else
        {
            ++b_spans;
        }
    }

    if (result->spans.size == 0)
    {
        result->x = 0;
        result->y = 0;
        result->w = 0;
        result->h = 0;
        return result;
    }

    gdSpanPtr spans = (result->spans.data);
    int x1 = INT_MAX;
    int y1 = spans[0].y;
    int x2 = 0;
    int y2 = spans[result->spans.size - 1].y;
    for (int i = 0; i < result->spans.size; i++)
    {
        if (spans[i].x < x1)
            x1 = spans[i].x;
        if (spans[i].x + spans[i].len > x2)
            x2 = spans[i].x + spans[i].len;
    }

    result->x = x1;
    result->y = y1;
    result->w = x2 - x1;
    result->h = y2 - y1 + 1;
    return result;
}

void gdSpanRlePathClip(gdSpanRlePtr rle, const gdSpanRlePtr clip)
{
    if (rle == NULL || clip == NULL)
        return;

    gdSpanRlePtr result = gdSpanHorizontalClip(rle, clip);
    if (!result) {
        return;
    }
    _rle_spans_allocate(rle->spans, result->spans.size);
    memcpy(rle->spans.data, result->spans.data, (size_t)result->spans.size * sizeof(gdSpan));
    rle->spans.size = result->spans.size;
    rle->x = result->x;
    rle->y = result->y;
    rle->w = result->w;
    rle->h = result->h;
    gdSpanRleDestroy(result);
}

static GD_FT_Outline *gd_ft_outline_create(int points, int contours)
{
    GD_FT_Outline *ft = gdMalloc(sizeof(GD_FT_Outline));
    if (!ft)
        return NULL;
    ft->points = malloc((size_t)(points + contours) * sizeof(GD_FT_Vector));
    ft->tags = malloc((size_t)(points + contours) * sizeof(char));
    ft->contours = malloc((size_t)contours * sizeof(short));
    ft->contours_flag = malloc((size_t)contours * sizeof(char));
    ft->n_points = ft->n_contours = 0;
    ft->flags = 0x0;
    return ft;
}

static void gd_ft_outline_close(GD_FT_Outline *ft)
{
    ft->contours_flag[ft->n_contours] = 0;
    int index = ft->n_contours ? ft->contours[ft->n_contours - 1] + 1 : 0;
    if (index == ft->n_points)
        return;

    ft->points[ft->n_points].x = ft->points[index].x;
    ft->points[ft->n_points].y = ft->points[index].y;
    ft->tags[ft->n_points] = GD_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void gd_ft_outline_end(GD_FT_Outline *ft)
{
    if (ft->n_points)
    {
        ft->contours[ft->n_contours] = ft->n_points - 1;
        ft->n_contours++;
    }
}

#define FT_COORD(x) (GD_FT_Pos)((x)*64)
static void gd_ft_outline_move_to(GD_FT_Outline *ft, double x, double y)
{
    ft->points[ft->n_points].x = FT_COORD(x);
    ft->points[ft->n_points].y = FT_COORD(y);
    ft->tags[ft->n_points] = GD_FT_CURVE_TAG_ON;
    if (ft->n_points)
    {
        ft->contours[ft->n_contours] = ft->n_points - 1;
        ft->n_contours++;
    }

    ft->contours_flag[ft->n_contours] = 1;
    ft->n_points++;
}

static void gd_ft_outline_line_to(GD_FT_Outline *ft, double x, double y)
{
    ft->points[ft->n_points].x = FT_COORD(x);
    ft->points[ft->n_points].y = FT_COORD(y);
    ft->tags[ft->n_points] = GD_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void gd_ft_outline_cubic_to(GD_FT_Outline *ft, double x1, double y1, double x2, double y2, double x3, double y3)
{
    ft->points[ft->n_points].x = FT_COORD(x1);
    ft->points[ft->n_points].y = FT_COORD(y1);
    ft->tags[ft->n_points] = GD_FT_CURVE_TAG_CUBIC;
    ft->n_points++;

    ft->points[ft->n_points].x = FT_COORD(x2);
    ft->points[ft->n_points].y = FT_COORD(y2);
    ft->tags[ft->n_points] = GD_FT_CURVE_TAG_CUBIC;
    ft->n_points++;

    ft->points[ft->n_points].x = FT_COORD(x3);
    ft->points[ft->n_points].y = FT_COORD(y3);
    ft->tags[ft->n_points] = GD_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static GD_FT_Outline *gd_ft_outline_convert(const gdPathPtr path, const gdPathMatrixPtr matrix)
{
    GD_FT_Outline *outline = gd_ft_outline_create(gdArrayNumElements(&path->points), path->contours);
    gdPointF p[3];
    unsigned int numElements = gdArrayNumElements(&path->elements);
    unsigned int pointsIndex = 0;
    unsigned int i = 0;

    memset(p, 0, sizeof(gdPointF) * 3);
    if (!outline)
        return NULL;
    for (i = 0; i < numElements; i++)
    {
        gdPathOpsPtr element = (gdPathOpsPtr)gdArrayIndex(&path->elements, i);
        gdPointFPtr point = gdArrayIndex(&path->points, pointsIndex);
        switch (*element)
        {
        case gdPathOpsMoveTo:
            gdPathMatrixMapPoint(matrix, point, &p[0]);
            gd_ft_outline_move_to(outline, p[0].x, p[0].y);
            pointsIndex += 1;
            break;
        case gdPathOpsLineTo:
            gdPathMatrixMapPoint(matrix, point, &p[0]);
            gd_ft_outline_line_to(outline, p[0].x, p[0].y);
            pointsIndex += 1;
            break;
        case gdPathOpsCubicTo:
            gdPathMatrixMapPoint(matrix, point, &p[0]);
            point = gdArrayIndex(&path->points, pointsIndex + 1);
            gdPathMatrixMapPoint(matrix, point, &p[1]);
            point = gdArrayIndex(&path->points, pointsIndex + 2);
            gdPathMatrixMapPoint(matrix, point, &p[2]);
            gd_ft_outline_cubic_to(outline, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
            pointsIndex += 3;
            break;
        case gdPathOpsClose:
            gd_ft_outline_close(outline);
            pointsIndex += 1;
            break;
         case gdPathOpsQuadTo:
                break;
        }
    }

    gd_ft_outline_end(outline);
    return outline;
}

static GD_FT_Outline* gd_ft_outline_convert_dash(const gdPathPtr path, const gdPathMatrixPtr matrix, const gdPathDashPtr dash)
{
    gdPathPtr dashed = gdPathApplyDash(dash, path);
    GD_FT_Outline* outline = gd_ft_outline_convert(dashed, matrix);
    gdPathDestroy(dashed);
    return outline;
}

static void gd_ft_outline_destroy(GD_FT_Outline *ft)
{
    gdFree(ft->points);
    gdFree(ft->tags);
    gdFree(ft->contours);
    gdFree(ft->contours_flag);
    gdFree(ft);
}

static void generation_callback(int count, const GD_FT_Span *spans, void *user)
{
    gdSpanRlePtr rle = user;
    _rle_spans_allocate(rle->spans, count);
    gdSpanPtr data = rle->spans.data + rle->spans.size;
    memcpy(data, spans, (size_t)count * sizeof(gdSpan));
    rle->spans.size += count;
}

static void bbox_callback(int x, int y, int w, int h, void *user)
{
    gdSpanRlePtr rle = user;
    rle->x = x;
    rle->y = y;
    rle->w = w;
    rle->h = h;
}

void gdSpanRleRasterize(gdSpanRlePtr rle, const gdPathPtr path, const gdPathMatrixPtr matrix, const gdRectFPtr clip, const gdStrokePtr stroke, gdFillRule winding)
{
    GD_FT_Raster_Params params;
    params.flags = GD_FT_RASTER_FLAG_DIRECT | GD_FT_RASTER_FLAG_AA;
    params.gray_spans = generation_callback;
    params.bbox_cb = bbox_callback;
    params.user = rle;

    if (clip)
    {
        params.flags |= GD_FT_RASTER_FLAG_CLIP;
        params.clip_box.xMin = (GD_FT_Pos)clip->x;
        params.clip_box.yMin = (GD_FT_Pos)clip->y;
        params.clip_box.xMax = (GD_FT_Pos)(clip->x + clip->w);
        params.clip_box.yMax = (GD_FT_Pos)(clip->y + clip->h);
    }

    if (stroke)
    {
        GD_FT_Stroker_LineCap ftCap;
        GD_FT_Stroker_LineJoin ftJoin;
        GD_FT_Fixed ftWidth;
        GD_FT_Fixed ftMiterLimit;

        gdPointF p1 = {0, 0};
        gdPointF p2 = {SQRT2, SQRT2};

        gdPathMatrixMapPoint(matrix, &p1, &p1);
        gdPathMatrixMapPoint(matrix, &p2, &p2);

        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;

        double scale = sqrt(dx * dx + dy * dy) / 2.0;
        double radius = stroke->width / 2.0;

        ftWidth = (GD_FT_Fixed)(radius * scale * (1 << 6));
        ftMiterLimit = (GD_FT_Fixed)(stroke->miterlimit * (1 << 16));

        switch (stroke->cap)
        {
        case gdLineCapSquare:
            ftCap = GD_FT_STROKER_LINECAP_SQUARE;
            break;
        case gdLineCapRound:
            ftCap = GD_FT_STROKER_LINECAP_ROUND;
            break;
        case gdLineCapButt:
        default:
            ftCap = GD_FT_STROKER_LINECAP_BUTT;
            break;
        }

        switch (stroke->join)
        {
        case gdLineJoinBevel:
            ftJoin = GD_FT_STROKER_LINEJOIN_BEVEL;
            break;
        case gdLineJoinRound:
            ftJoin = GD_FT_STROKER_LINEJOIN_ROUND;
            break;
        case gdLineJoinMiter:
        default:
            ftJoin = GD_FT_STROKER_LINEJOIN_MITER_FIXED;
            break;
        }

        GD_FT_Outline *outline = stroke->dash ? gd_ft_outline_convert_dash(path, matrix, stroke->dash) : gd_ft_outline_convert(path, matrix);
        if (!outline)
            return;
        GD_FT_Stroker stroker;
        GD_FT_Stroker_New(&stroker);
        GD_FT_Stroker_Set(stroker, ftWidth, ftCap, ftJoin, ftMiterLimit);
        GD_FT_Stroker_ParseOutline(stroker, outline);

        GD_FT_UInt points;
        GD_FT_UInt contours;
        GD_FT_Stroker_GetCounts(stroker, &points, &contours);

        GD_FT_Outline *strokeOutline = gd_ft_outline_create((int)points, (int)contours);
        if (!strokeOutline)
            return;
        GD_FT_Stroker_Export(stroker, strokeOutline);
        GD_FT_Stroker_Done(stroker);

        strokeOutline->flags = GD_FT_OUTLINE_NONE;
        params.source = strokeOutline;
        gd_ft_grays_raster.raster_render(NULL, &params);
        gd_ft_outline_destroy(outline);
        gd_ft_outline_destroy(strokeOutline);
    }
    else
    {
        GD_FT_Outline *outline = gd_ft_outline_convert(path, matrix);
        outline->flags = winding == gdFillRulEvenOdd ? GD_FT_OUTLINE_EVEN_ODD_FILL : GD_FT_OUTLINE_NONE;
        params.source = outline;
        gd_ft_grays_raster.raster_render(NULL, &params);
        gd_ft_outline_destroy(outline);
    }
}