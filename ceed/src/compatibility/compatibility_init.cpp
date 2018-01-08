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

#include "compatibility/compatibility_init.h"

#include <QFileInfo>

namespace CEED {
namespace compatibility {

QStringList Manager::getKnownTypes()
{
    /**Retrieves types that we have detectors for.*/

    QStringList ret;
    for (TypeDetector* detector : m_detectors) {
        ret += detector->getType();
    }

    return ret;
}

QSet<QString> Manager::getAllPossibleExtensions()
{
    /**Retrieves all possible extensions of all types this manager knows of.*/

    QSet<QString> ret;
    for (TypeDetector* detector : m_detectors) {
        ret += detector->getPossibleExtensions();
    }
    return ret;
}

QStringList Manager::getCEGUIVersionsCompatibleWithType(const QString &type)
{
    QStringList ret;

    for (auto it = CEGUIVersionTypes.begin(); it != CEGUIVersionTypes.end(); it++) {
        QString version = it.key();
        QString otherType = it.value();
        if (type == otherType) {
            ret.append(version);
        }
    }

    return ret;
}

QString Manager::getSuitableDataTypeForCEGUIVersion(const QString &ceguiVersion)
{
    QStringList ret;

    for (auto it = CEGUIVersionTypes.begin(); it != CEGUIVersionTypes.end(); it++) {
        QString version = it.key();
        QString dataType = it.value();
        if (version == ceguiVersion) {
            ret.append(dataType);
        }
    }

    if (ret.length() > 1)
        throw RuntimeError(QString("More than one data type is suitable for given CEGUI version '%1', this must be a mistake in the compatibility code in CEED!").arg(ceguiVersion));

    if (ret.isEmpty())
        throw RuntimeError(QString("Can't find any suitable data type for given CEGUI version '%1'. It's possible that this version isn't supported for the editing you are about to do.").arg(ceguiVersion));

    return ret[0];
}

QString Manager::transform(const QString &sourceType, const QString &targetType, const QString& data, const QList<Layer *> visitedLayers)
{
    logging.debug(QString("Attempting to transform type '%1' into '%2' (compatibility path fragments are in reverse order)").arg(sourceType).arg(targetType));

    // special case:
    if (sourceType == targetType) {
        logging.debug("Returning data with no transformation applied, both types are the same!");
        return data;
    }

    for (Layer* layer : m_layers) {
        if (visitedLayers.contains(layer))
            continue;

        if (layer->getSourceType() == sourceType) {
            try {
                QString ret = transform(layer->getTargetType(), targetType, layer->transform(data), QList<Layer*>() << visitedLayers << layer);
                logging.debug(QString("Compatibility path fragment: '%1' <- '%2'").arg(layer->getTargetType()).arg(sourceType));
                return ret;

            } catch (LayerNotFoundError e) {
                // this path doesn't lead anywhere,
                // lets try to find another one
            }
        }
    }

    throw new LayerNotFoundError(sourceType, targetType);
}

QString Manager::guessType(const QString& code, const QString &extension_)
{
    logging.debug(QString("Attempting to guess type (code size: %1, extension: '%2')").arg(code.length()).arg(extension_));

#if 1
    QString extension = QFileInfo(extension_).suffix();
#else
    QStringList extSplit = extension.rsplit(".", 1);

    if (!extSplit.isEmpty())
        extension = (extSplit.length() == 2) ? extSplit[1] : extSplit[0];
    else
        extension = extension.lstrip(".", 1);
#endif

    QStringList ret;

    for (TypeDetector* detector : m_detectors) {
        if (detector->matches(code, extension)) {
            logging.debug(QString("Detector '%1' reported a positive match!").arg(detector->getType()));
            ret.append(detector->getType());
        }
    }

    if (ret.length() > 1)
        throw MultiplePossibleTypesError(ret);

    if (ret.isEmpty())
        throw NoPossibleTypesError();

    return ret[0];
}

QString Manager::transformTo(const QString &targetType, const QString& code, const QString &extension)
{
    QString sourceType = guessType(code, extension);
    return transform(sourceType, targetType, code);
}


} // namespace compatibility
} // namespace CEED
