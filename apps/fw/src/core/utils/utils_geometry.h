/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Public header for geometry utils.
 */

#ifndef UTILS_GEOMETRY_H_
#define UTILS_GEOMETRY_H_

#include "hydra/hydra_types.h"

/**
 * Type definition for a point with 32 bit coordinates.
 */
typedef struct utils_point_32
{
    int32 x;
    int32 y;
} utils_point_32;

/**
 * Type definition for a line represented by points with 32 bit coordinates.
 */
typedef struct utils_line_32
{
    utils_point_32 a;
    utils_point_32 b;
} utils_line_32;


/**
 * Checks if two lines are parallel.
 * \param l1 First line defined by two points.
 * \param l2 Second line defined by two points.
 * \return TRUE if lines are parallel, FALSE otherwise.
 */
bool lines_are_parallel(utils_line_32 l1, utils_line_32 l2);

/**
 * Calculates the X coordinate of the intersection point between two lines.
 * \param l1 First line defined by two points.
 * \param l2 Second line defined by two points.
 * \return X coordinate of the intersection point.
 */
int32 lines_intersect_x(utils_line_32 l1, utils_line_32 l2);

/**
 * Calculates the Y coordinate of the intersection point between two lines.
 * \param l1 First line defined by two points.
 * \param l2 Second line defined by two points.
 * \return Y coordinate of the intersection point.
 */
int32 lines_intersect_y(utils_line_32 l1, utils_line_32 l2);

/**
 * Calculates the X coordinate of the triangle centroid (center of mass).
 * \param l1 First line defined by two points.
 * \param l2 Second line defined by two points.
 * \param l3 Third line defined by two points.
 * \return X coordinate of the centroid.
 */
int32 triangle_centroid_x(utils_line_32 l1, utils_line_32 l2, utils_line_32 l3);

/**
 * Calculates the Y coordinate of the triangle centroid (center of mass).
 * \param l1 First line defined by two points.
 * \param l2 Second line defined by two points.
 * \param l3 Third line defined by two points.
 * \return Y coordinate of the centroid.
 */
int32 triangle_centroid_y(utils_line_32 l1, utils_line_32 l2, utils_line_32 l3);


#define LINEAR_CONV(OLDSCALE_VAL, \
                    OLDSCALE_MIN, OLDSCALE_MAX, \
                    NEWSCALE_MIN, NEWSCALE_MAX) \
    (((int32)(OLDSCALE_VAL) - (int32)(OLDSCALE_MIN)) * \
     ((int32)(NEWSCALE_MAX) - (int32)(NEWSCALE_MIN)) / \
     ((int32)(OLDSCALE_MAX) - (int32)(OLDSCALE_MIN)) + \
     ((int32)(NEWSCALE_MIN)))


#define RULE_OF_THREE(RATIO_NOM, RATIO_DENOM, VALUE) \
    ((int32)(VALUE) * (int32)(RATIO_NOM) / (int32)(RATIO_DENOM))


#endif /*UTILS_GEOMETRY_H_*/
