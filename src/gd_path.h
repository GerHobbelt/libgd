#ifndef GD_PATH_H
#define GD_PATH_H

gdPathPtr gdPathCreate();
gdPathPtr gdPathAddRef(gdPathPtr path);
void gdPathDestroy(gdPathPtr path);

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI2
#define M_PI2 M_PI * 2
#endif

#define EPSILON_DOUBLE 0.000000000001f
#define MAX_FULL_CIRCLES 65536
#define PATH_KAPPA 0.5522847498
#define DEFAULT_TOLERANCE 0.1
static inline int _doubleEqualsEpsilon(double p1, double p2)
{
    return (fabs(p1 - p2) < EPSILON_DOUBLE);
}

static inline int _doubleIsZero(double f)
{
    return (fabs(f) <= EPSILON_DOUBLE);
}

#define ARRAY_LENGTH(__array) ((int) (sizeof (__array) / sizeof (__array[0])))

gdPaintPtr gdPaintCreateRgba(double r, double g, double b, double a);
void gdPaintDestroy(gdPaintPtr paint);
void gdContextSetSource(gdContextPtr context, gdPaintPtr source);

gdStatePtr gdStateCreate();
void gdStateDestroy(gdStatePtr state);

gdPathPtr gdPathCreate();
gdPathPtr gdPathDuplicate(const gdPathPtr path);
gdPathPtr gdPathDuplicateFlattened(const gdPathPtr path);
void gdPathClear(gdPathPtr path);
void gdPathDestroy(gdPathPtr path);

gdPaintPtr gdPaintCreateForSurface(gdSurfacePtr surface);
gdPathPatternPtr gdPaintGetPattern(const gdPaintPtr paint);
void gdPathPatternSetMatrix(gdPathPatternPtr pattern, gdPathMatrixPtr matrix);
void gdPaintSetSourceSurface(gdContextPtr context, gdSurfacePtr surface, double x, double y);
void gdPathPatternDestroy(gdPathPatternPtr pattern);

void gdPathCurveTo(gdPathPtr path, double x1, double y1, double x2, double y2, double x3, double y3);
void gdPathQuadTo(gdPathPtr path, double x1, double y1, double x2, double y2);
void gdPathLineTo(gdPathPtr path, double x, double y);
void gdPathRelLineTo(gdPathPtr path, double dx, double dy);
void gdPathMoveTo(gdPathPtr path, double x, double y);
void gdPathArcTo(gdPathPtr path, double x1, double y1, double x2, double y2, double radius);
void gdPathAddRectangle(gdPathPtr path, double x, double y, double w, double h);
void gdPathClose(gdPathPtr path);

void gdPathAddArc(gdPathPtr path, double cx, double cy, double radius, double angle1, double angle2, int ccw);
void gdPathDumpPathTransform(const gdPathPtr path, const gdPathMatrixPtr matrix);
#endif // GD_PATH_H