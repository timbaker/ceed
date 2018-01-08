/*
Packer using a custom algorithm by Markus 'Cygon' Ewald
Copyright (C) 2002-2010 Nuclex Development Labs

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __CEED_metaimageset_rectanglpacking__
#define __CEED_metaimageset_rectanglpacking__

#include "CEEDBase.h"

// Thanks to Markus 'Cygon' Ewald (original CygonPacker author)
//       and Eniko (porting CygonPacker to python)
//
// currently at: http://projectdrake.net/blog/?p=129
// originally found at: http://enichan.darksiren.net/wordpress/?p=49
//
//       Cygon furthermore allowed this to be relicensed under
//       GPL3 and newer so I got around the fact that CPL is not
//       GPL-compatible
//
// We use this for imageset optimisation and meta-imageset
// generating code.

// Minor changes by Martin Preisler

//from bisect import bisect_left

namespace CEED {
namespace editors {
namespace metaimageset {
namespace rectanglepacking {

typedef Exception OutOfSpaceError;

class Point
{
public:
    int m_x;
    int m_y;

    Point(int x = 0, int y = 0)
    {
        m_x = x;
        m_y = y;
    }

    /**Compares the starting position of height slices*/
    bool operator<(const Point& other)
    {
        return m_x < other.m_x;
    }
};

/*!
\brief RectanglePacker

Base class for rectangle packing algorithms

    By uniting all rectangle packers under this common base class, you can
    easily switch between different algorithms to find the most efficient or
    performant one for a given job.

    An almost exhaustive list of packing algorithms can be found here:
    http://www.csc.liv.ac.uk/~epa/surveyhtml.html
*/
class RectanglePacker
{
public:
    int m_packingAreaWidth;
    int m_packingAreaHeight;

    /**Initializes a new rectangle packer

    packingAreaWidth: Maximum width of the packing area
    packingAreaHeight: Maximum height of the packing area*/
    RectanglePacker(int packingAreaWidth, int packingAreaHeight)
    {
        m_packingAreaWidth = packingAreaWidth;
        m_packingAreaHeight = packingAreaHeight;
    }

    /**Allocates space for a rectangle in the packing area

    rectangleWidth: Width of the rectangle to allocate
    rectangleHeight: Height of the rectangle to allocate

    Returns the location at which the rectangle has been placed*/
    Point pack(int rectangleWidth, int rectangleHeight)
    {
        optional<Point> point = tryPack(rectangleWidth, rectangleHeight);

        if (!point)
            throw OutOfSpaceError(QString("Rectangle does not fit in packing area"));

        return *point;
    }

    /**Tries to allocate space for a rectangle in the packing area

    rectangleWidth: Width of the rectangle to allocate
    rectangleHeight: Height of the rectangle to allocate

    Returns a Point instance if space for the rectangle could be allocated
    be found, otherwise returns None*/
    virtual optional<Point> tryPack(int rectangleWidth, int rectangleHeight) = 0;
};


/*!
\brief CygonRectanglePacker


    Packer using a custom algorithm by Markus 'Cygon' Ewald

    Algorithm conceived by Markus Ewald (cygon at nuclex dot org), though
    I'm quite sure I'm not the first one to come up with it :)

    The algorithm always places rectangles as low as possible in the packing
    area. So, for any new rectangle that is to be added, the packer has to
    determine the X coordinate at which the rectangle can have the lowest
    overall height without intersecting any other rectangles.

    To quickly discover these locations, the packer uses a sophisticated
    data structure that stores the upper silhouette of the packing area. When
    a new rectangle needs to be added, only the silouette edges need to be
    analyzed to find the position where the rectangle would achieve the lowest
*/
class CygonRectanglePacker : public RectanglePacker
{
public:
    QList<Point> m_heightSlices;

    /**Initializes a new rectangle packer

    packingAreaWidth: Maximum width of the packing area
    packingAreaHeight: Maximum height of the packing area*/
    CygonRectanglePacker(int packingAreaWidth, int packingAreaHeight)
        : RectanglePacker(packingAreaWidth, packingAreaHeight)
    {
        // Stores the height silhouette of the rectangles
//        m_heightSlices = [];

        // At the beginning, the packing area is a single slice of height 0
        m_heightSlices.append(Point(0, 0));
    }

    /**Tries to allocate space for a rectangle in the packing area

    rectangleWidth: Width of the rectangle to allocate
    rectangleHeight: Height of the rectangle to allocate

    Returns a Point instance if space for the rectangle could be allocated
    be found, otherwise returns None*/
    optional<Point> tryPack(int rectangleWidth, int rectangleHeight)
    {
        // If the rectangle is larger than the packing area in any dimension,
        // it will never fit!
        if (rectangleWidth > m_packingAreaWidth || rectangleHeight > m_packingAreaHeight)
            return nonstd::nullopt;

        // Determine the placement for the new rectangle
        optional<Point> placement = tryFindBestPlacement(rectangleWidth, rectangleHeight);

        // If a place for the rectangle could be found, update the height slice
        // table to mark the region of the rectangle as being taken.
        if (placement)
            integrateRectangle(placement->m_x, rectangleWidth, placement->m_y + rectangleHeight);

        return placement;
    }

    /**Finds the best position for a rectangle of the given dimensions

    rectangleWidth: Width of the rectangle to find a position for
    rectangleHeight: Height of the rectangle to find a position for

    Returns a Point instance if a valid placement for the rectangle could
    be found, otherwise returns None*/
    optional<Point> tryFindBestPlacement(int rectangleWidth, int rectangleHeight)
    {
        // Slice index, vertical position and score of the best placement we
        // could find
        int bestSliceIndex = -1; // Slice index where the best placement was found
        int bestSliceY = 0; // Y position of the best placement found
        // lower == better!
        int bestScore = m_packingAreaWidth * m_packingAreaHeight;

        // This is the counter for the currently checked position. The search
        // works by skipping from slice to slice, determining the suitability
        // of the location for the placement of the rectangle.
        int leftSliceIndex = 0;

        // Determine the slice in which the right end of the rectangle is located
        int rightSliceIndex = std::lower_bound(m_heightSlices.begin(), m_heightSlices.end(), Point(rectangleWidth, 0)) - m_heightSlices.begin(); // bisect_left(m_heightSlices, Point(rectangleWidth, 0));
        if (rightSliceIndex < 0)
            rightSliceIndex = ~rightSliceIndex;

        while (rightSliceIndex <= m_heightSlices.length()) {
            // Determine the highest slice within the slices covered by the
            // rectangle at its current placement. We cannot put the rectangle
            // any lower than this without overlapping the other rectangles.
            int highest = m_heightSlices[leftSliceIndex].m_y;
            for (int index = leftSliceIndex + 1; index < rightSliceIndex; index++) {
                if (m_heightSlices[index].m_y > highest)
                    highest = m_heightSlices[index].m_y;
            }

            // Only process this position if it doesn't leave the packing area
            if (highest + rectangleHeight < m_packingAreaHeight) {
                int score = highest;

                if (score < bestScore) {
                    bestSliceIndex = leftSliceIndex;
                    bestSliceY = highest;
                    bestScore = score;
                }
            }

            // Advance the starting slice to the next slice start
            leftSliceIndex += 1;
            if (leftSliceIndex >= m_heightSlices.length())
                break;

            // Advance the ending slice until we're on the proper slice again,
            // given the new starting position of the rectangle.
            int rightRectangleEnd = m_heightSlices[leftSliceIndex].m_x + rectangleWidth;
            while (rightSliceIndex <= m_heightSlices.length()) {
                int rightSliceStart;
                if (rightSliceIndex == m_heightSlices.length())
                    rightSliceStart = m_packingAreaWidth;
                else
                    rightSliceStart = m_heightSlices[rightSliceIndex].m_x;

                // Is this the slice we're looking for?
                if (rightSliceStart > rightRectangleEnd)
                    break;

                rightSliceIndex += 1;
            }

            // If we crossed the end of the slice array, the rectangle's right
            // end has left the packing area, and thus, our search ends.
            if (rightSliceIndex > m_heightSlices.length())
                break;
        }

        // Return the best placement we found for this rectangle. If the
        // rectangle didn't fit anywhere, the slice index will still have its
        // initialization value of -1 and we can report that no placement
        // could be found.
        if (bestSliceIndex == -1)
            return nonstd::nullopt;
        else
            return Point(m_heightSlices[bestSliceIndex].m_x, bestSliceY);
    }

    /**Integrates a new rectangle into the height slice table

    left: Position of the rectangle's left side
    width: Width of the rectangle
    bottom: Position of the rectangle's lower side*/
    void integrateRectangle(int left, int width, int bottom)
    {
        // Find the first slice that is touched by the rectangle
        int startSlice = std::lower_bound(m_heightSlices.begin(), m_heightSlices.end(), Point(left, 0)) - m_heightSlices.begin(); //bisect_left(m_heightSlices, Point(left, 0));

        // Did we score a direct hit on an existing slice start?
        int firstSliceOriginalHeight;
        if (startSlice >= 0) {
            // We scored a direct hit, so we can replace the slice we have hit
            firstSliceOriginalHeight = m_heightSlices[startSlice].m_y;
            m_heightSlices[startSlice] = Point(left, bottom);
        } else { // No direct hit, slice starts inside another slice
            // Add a new slice after the slice in which we start
            startSlice = ~startSlice;
            firstSliceOriginalHeight = m_heightSlices[startSlice - 1].m_y;
            m_heightSlices.insert(startSlice, Point(left, bottom));
        }

        int right = left + width;
        startSlice += 1;

        // Special case, the rectangle started on the last slice, so we cannot
        // use the start slice + 1 for the binary search and the possibly
        // already modified start slice height now only remains in our temporary
        // firstSliceOriginalHeight variable
        if (startSlice >= m_heightSlices.length()) {
            // If the slice ends within the last slice (usual case, unless it
            // has the exact same width the packing area has), add another slice
            // to return to the original height at the end of the rectangle.
            if (right < m_packingAreaWidth) {
                m_heightSlices.append(Point(right, firstSliceOriginalHeight));
            }
        } else { // The rectangle doesn't start on the last slice
            int endSlice = std::lower_bound(m_heightSlices.begin() + startSlice, m_heightSlices.end(), Point(right, 0)) - m_heightSlices.begin(); // bisect_left(m_heightSlices, Point(right, 0), startSlice, m_heightSlices.length());

            // Another direct hit on the final slice's end?
            if (endSlice > 0)
                m_heightSlices.erase(m_heightSlices.begin() + startSlice, m_heightSlices.begin() + endSlice);
            else { // No direct hit, rectangle ends inside another slice
                // Make index from negative bisect_left() result
                endSlice = ~endSlice;

                // Find out to which height we need to return at the right end of
                // the rectangle
                int returnHeight;
                if (endSlice == startSlice)
                    returnHeight = firstSliceOriginalHeight;
                else
                    returnHeight = m_heightSlices[endSlice - 1].m_y;

                // Remove all slices covered by the rectangle and begin a new
                // slice at its end to return back to the height of the slice on
                // which the rectangle ends.
                m_heightSlices.erase(m_heightSlices.begin() + startSlice, m_heightSlices.begin() + endSlice);
                if (right < m_packingAreaWidth)
                    m_heightSlices.insert(startSlice, Point(right, returnHeight));
            }
        }
    }
};

} // namespace rectanglepacking
} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
