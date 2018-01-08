/*
   CEED - Unified CEGUI asset editor

   Copyright (C) 2011-2017   Martin Preisler <martin@preisler.me>
                             and contributing authors (see AUTHORS file)

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

#ifndef CEED_metaimageset_compiler_
#define CEED_metaimageset_compiler_

#include "CEEDBase.h"


namespace CEED {
namespace editors {
namespace metaimageset {
namespace compiler {

class ImageInstance
{
public:
    int m_x;
    int m_y;
    inputs::Image* m_image;

    ImageInstance(int x, int y, inputs::Image* image)
    {
        m_x = x;
        m_y = y;

        m_image = image;
    }
};


class CompilerInstance
{
public:
    int m_jobs;
    int m_sizeIncrement;
    bool m_padding;
    MetaImageset* m_metaImageset;

    CompilerInstance(MetaImageset* metaImageset)
    {
        m_jobs = 1;
        m_sizeIncrement = 5;
        // if true, the images will be padded on all sizes to prevent UV
        // rounding/interpolation artefacts
        m_padding = true;

        m_metaImageset = metaImageset;
    }

    /**Returns the next power of two that is greater than given number*/
    static
    int getNextPOT(int number);

    /**Tries to estimate minimal side of the underlying image of the output imageset.

    This is used merely as a starting point in the packing process.
    */
    int estimateMinimalSize(const QList<inputs::Image*>& images);

    int findSideSize(int startingSideSize, const QList<inputs::Image*>& images, QList<ImageInstance *> &imageInstances);

    QList<inputs::Image*> buildAllImages(const QList<inputs::Input*>& inputs, int parallelJobs);

    void compile();
};

} // namespace compiler
} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
