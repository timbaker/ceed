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

#include "metaimageset_compiler.h"

#include "metaimageset/metaimageset_init.h"
#include "metaimageset/inputs/metaimageset_inputs_init.h"
#include "metaimageset/rectanglepacking.h"

#include "compatibility/imageset/compat_imageset_init.h"

#include "ceed_paths.h"

#include <QFileInfo>
#include <QPainter>
#include <QtMath>

namespace CEED {
namespace editors {
namespace metaimageset {
namespace compiler {

#define print(s) (qDebug() << (s))

int CompilerInstance::getNextPOT(int number)
{
#if 1
    return qNextPowerOfTwo(number);
#else
    return int(2 ** math.ceil(math.log(number + 1, 2)));
#endif
}

int CompilerInstance::estimateMinimalSize(const QList<inputs::Image *> &images)
{
    int area = 0;
    for (auto image : images) {
        if (m_padding)
            area += (image->m_qimage.width() + 2) * (image->m_qimage.height() + 2);
        else
            area += (image->m_qimage.width()) * (image->m_qimage.height());
    }

    int ret = qSqrt(area);

    if (ret < 1)
        ret = 1;

    if (m_metaImageset->m_onlyPOT)
        ret = CompilerInstance::getNextPOT(ret);

    return ret;
}

int CompilerInstance::findSideSize(int startingSideSize, const QList<inputs::Image *> &images, QList<ImageInstance*>& imageInstances)
{
    int sideSize = startingSideSize;

    int i = 0;

    // This could be way sped up if we used some sort of a "binary search" approach
    while (true) {
        rectanglepacking::CygonRectanglePacker packer(sideSize, sideSize);
        try {
            imageInstances.clear();

            for (auto image : images) {
                rectanglepacking::Point point;
                if (m_padding)
                    point = packer.pack(image->m_qimage.width() + 2, image->m_qimage.height() + 2);
                else
                    point = packer.pack(image->m_qimage.width(), image->m_qimage.height());

                imageInstances.append(new ImageInstance(point.m_x, point.m_y, image));
            }

            // everything seems to have gone smoothly, lets use this configuration then
            break;
        }

        catch (rectanglepacking::OutOfSpaceError e) {
            sideSize = m_metaImageset->m_onlyPOT ? CompilerInstance::getNextPOT(sideSize) : (sideSize + m_sizeIncrement);

            i += 1;
            if (i % 5 == 0)
                print(QString("%1 candidate sizes checked").arg(i));

            continue;
        }
    }

    print(QString("Correct texture side size found after %1 iterations").arg(i));
    print("");

    return sideSize;
}

QList<inputs::Image*> CompilerInstance::buildAllImages(const QList<inputs::Input *> &inputs_, int parallelJobs)
{
    Q_ASSERT(parallelJobs >= 1);

    QList<inputs::Image*> images;
#if 0 // TODO

    Queue::Queue queue;
    for (auto input_ : inputs_)
        queue.put_nowait(input_);

    // we use a nasty trick of adding None elements to a list
    // because Python's int type is immutable
    QList<> doneTasks;

    errorsEncountered = threading::Event();

    imageBuilder = [=]()
    {
        while (true) {
            try {
                inputs::Input* input_ = queue.get(false);

                // In case an error occurs in any of the builders, we short circuit
                // everything.
                if (!errorsEncountered.is_set()) {
                    try {
                        // We do not have to do anything extra thanks to GIL
                        images.extend(input_->buildImages());
                    } catch (Exception e) {
#if 0
                        print("Error building input '%s'. %s" % (input_->getDescription(), e));
#endif
                        errorsEncountered.set();
                    }
                }

                // If an exception was caught above, fake the task as done to allow the builders to end
                queue.task_done();
                doneTasks.append(None);
#if 0
                percent = "{0:6.2f}%".format(float(len(doneTasks) * 100) / len(inputs));

                // same as above
                sys.stdout.write("[%s] Images from %s\n" % (percent, input_.getDescription()));
#endif
            }

            catch (Queue::Empty e) {
                break;
            }
        }
    };

    QList<> workers;
    for (int workerId = 0; workerId < parallelJobs; workerId++) {
        worker = threading::Thread(name = "MetaImageset compiler image builder worker #%i" % (workerId), target = imageBuilder);
        workers.append(worker);
        worker.start();
    }

    queue.join();

    if (errorsEncountered.is_set())
        throw RuntimeError("Errors encountered when building images!");
#endif
    return images;
}

void CompilerInstance::compile()
{
    print(QString("Gathering and rendering all images in %1 parallel jobs...").arg(m_jobs));
    print("");

    auto images = buildAllImages(m_metaImageset->m_inputs, m_jobs);
    print("");

    int theoreticalMinSize = estimateMinimalSize(images);

    // the image packer performs better if images are inserted by width, thinnest come first
#if 1
    std::sort(images.begin(), images.end(), [](inputs::Image* a, inputs::Image* b) {
        return a->m_qimage.width() < b->m_qimage.width();
    });
#else
    images = sorted(images, key = lambda image: image.qimage.width());
#endif

    print("Performing texture side size determination...");
    QList<ImageInstance*> imageInstances;
    int sideSize = findSideSize(theoreticalMinSize, images, imageInstances);

    print("Rendering the underlying image...");
    QImage underlyingImage(sideSize, sideSize, QImage::Format_ARGB32);
    underlyingImage.fill(0);

    QPainter painter;
    painter.begin(&underlyingImage);

    // Sort image instances by name to give us nicer diffs of the resulting imageset
#if 1
    std::sort(imageInstances.begin(), imageInstances.end(), [](ImageInstance* a, ImageInstance* b) {
        return a->m_image->m_name < b->m_image->m_name;
    });
#else
    imageInstances = sorted(imageInstances, key = lambda instance: instance.image.name);
#endif

    for (auto imageInstance : imageInstances) {
        QImage qimage = imageInstance->m_image->m_qimage;

        if (m_padding) {
            // FIXME: This could use a review!

            // top without corners
            painter.drawImage(QPointF(imageInstance->m_x + 1, imageInstance->m_y), qimage.copy(0, 0, qimage.width(), 1));
            // bottom without corners
            painter.drawImage(QPointF(imageInstance->m_x + 1, imageInstance->m_y + 1 + qimage.height()), qimage.copy(0, qimage.height() - 1, qimage.width(), 1));
            // left without corners
            painter.drawImage(QPointF(imageInstance->m_x, imageInstance->m_y + 1), qimage.copy(0, 0, 1, qimage.height()));
            // right without corners
            painter.drawImage(QPointF(imageInstance->m_x + 1 + qimage.width(), imageInstance->m_y + 1), qimage.copy(qimage.width() - 1, 0, 1, qimage.height()));

            // top left corner
            painter.drawImage(QPointF(imageInstance->m_x, imageInstance->m_y), qimage.copy(0, 0, 1, 1));
            // top right corner
            painter.drawImage(QPointF(imageInstance->m_x + 1 + qimage.width(), imageInstance->m_y), qimage.copy(qimage.width() - 1, 0, 1, 1));
            // bottom left corner
            painter.drawImage(QPointF(imageInstance->m_x, imageInstance->m_y + 1 + qimage.height()), qimage.copy(0, qimage.height() - 1, 1, 1));
            // bottom right corner
            painter.drawImage(QPointF(imageInstance->m_x + 1 + qimage.width(), imageInstance->m_y + 1 + qimage.height()), qimage.copy(qimage.width() - 1, qimage.height() - 1, 1, 1));

            // and then draw the real image on top
            painter.drawImage(QPointF(imageInstance->m_x + 1, imageInstance->m_y + 1),
                              qimage);
        } else {
            // padding disabled, just draw the real image
            painter.drawImage(QPointF(imageInstance->m_x, imageInstance->m_y),
                              qimage);
        }
    }

    painter.end();

    print("Saving underlying image...");

#if 1
    QString underlyingImageFileName = QFileInfo(m_metaImageset->m_output).completeBaseName() + ".png";
#else
    outputSplit = m_metaImageset->m_output.rsplit(".", 1);
    underlyingImageFileName = "%s.png" % (outputSplit[0]);
#endif
    underlyingImage.save(os.path.join(m_metaImageset->getOutputDirectory(), underlyingImageFileName));

    // CEGUI imageset format is very simple and easy to work with, using serialisation in the editor for this
    // seemed like a wasted effort :-)

    QString nativeData = QString("<Imageset name=\"%1\" imagefile=\"%2\" nativeHorzRes=\"%3\" nativeVertRes=\"%4\" autoScaled=\"%5\" version=\"2\">\n")
                .arg(m_metaImageset->m_name)
                .arg(underlyingImageFileName)
                .arg(m_metaImageset->m_nativeHorzRes)
                .arg(m_metaImageset->m_nativeVertRes)
                .arg(m_metaImageset->m_autoScaled);
    for (auto imageInstance : imageInstances) {
        int paddingOffset = m_padding ? 1 : 0;

        nativeData += QString("    <Image name=\"%1\" xPos=\"%2\" yPos=\"%3\" width=\"%4\" height=\"%5\" xOffset=\"%6\" yOffset=\"%7\" />\n")
                .arg(imageInstance->m_image->m_name)
                .arg(imageInstance->m_x + paddingOffset, imageInstance->m_y + paddingOffset)
                .arg(imageInstance->m_image->m_qimage.width())
                .arg(imageInstance->m_image->m_qimage.height())
                .arg(imageInstance->m_image->m_xOffset)
                .arg(imageInstance->m_image->m_yOffset);
    }

    nativeData += "</Imageset>\n";

    QString outputData = compatibility::imageset::manager->transform(compatibility::imageset::manager->EditorNativeType, m_metaImageset->m_outputTargetType, nativeData);
    QFile file(os.path.join(m_metaImageset->getOutputDirectory(), m_metaImageset->m_output));
    if (file.open(QFile::WriteOnly)) {
        file.write(outputData.toUtf8());
        file.close();
    }

    print(QString("Saved to directory '%1', imageset: '%2', underlying image: '%3'.")
          .arg(m_metaImageset->getOutputDirectory())
          .arg(m_metaImageset->m_output)
          .arg(underlyingImageFileName));

    print("All done and saved!");
    print("");

#if 0 // TODO
    int rjustChars = 40;
    print("Amount of inputs: ".rjust(rjustChars) + "%i" % (len(m_metaImageset.inputs)));
    print("Amount of images on the atlas: ".rjust(rjustChars) + "%i" % (len(imageInstances)));
    print("");
    print("Theoretical minimum texture size: ".rjust(rjustChars) + "%i x %i" % (theoreticalMinSize, theoreticalMinSize));
    print("Actual texture size: ".rjust(rjustChars) + "%i x %i" % (sideSize, sideSize));
    print("");
    print("Side size overhead: ".rjust(rjustChars) + "%f%%" % ((sideSize - theoreticalMinSize) / (theoreticalMinSize) * 100));
    print("Area (squared) overhead: ".rjust(rjustChars) + "%f%%" % ((sideSize * sideSize - theoreticalMinSize * theoreticalMinSize) / (theoreticalMinSize * theoreticalMinSize) * 100));
#endif
}



} // namespace compiler
} // namespace metaimageset
} // namespace editors
} // namespace CEED
