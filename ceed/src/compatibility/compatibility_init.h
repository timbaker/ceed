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

#ifndef CEED_compatibility___init___
#define CEED_compatibility___init___

#include "CEEDBase.h"

/**This module is the root of all compatibility support and layers in the editor
*/

//import logging

#include <QMap>
#include <QStringList>

// NOTE: It should be importable with as few dependencies as possible because
//       this is used in the command line migration tool!

namespace CEED {
namespace compatibility {

/*!
\brief Layer

Compatibility layer can transform given code from source type to target type.

    If you want transparent loading and saving you need to implement 2 layers!
    From your type to editor's supported type (or type for which there already are
    compatibility layers) and back!

*/
class Layer
{
public:
    virtual QString getSourceType() = 0;

    virtual QString getTargetType() = 0;

    /**Transforms given data from sourceType (== self.getSourceType())
    to targetType (== self.getTargetType())
    */
    virtual QString transform(const QString& data) = 0;
};

class TypeDetector
{
public:
    /**Gets the type this detector detects*/
    virtual QString getType() = 0;

    /**Retrieves all possible extensions the type this detector detects can have as a set.*/
    virtual QSet<QString> getPossibleExtensions() = 0;

    /**Checks whether given source code and extension match this detector's type.

    The detector should be as strict as possible, if it returns that the type matches, the editor will
    assume it can safely open it as that type. If unsure, return false!

    User will be prompted to choose which type the file is if the editor can't make a 100% match.
    */
    virtual bool matches(const QString& data, const QString& extension) = 0;
};

/*!
\brief LayerNotFoundError

Exception thrown when no compatibility layer or path can be found between 2 types
*/
class LayerNotFoundError : public RuntimeError
{
public:
    LayerNotFoundError(const QString& sourceType, const QString& targetType)
        : RuntimeError(QString("Can't find any compatibility path from sourceType '%1' to targetType '%2'").arg(sourceType, targetType))
    {
    }
};

/*!
\brief MultiplePossibleTypesError

Exception thrown when multiple types match given data (from the guessType method), user should be
    asked to choose the right type in this case

*/
class MultiplePossibleTypesError : public RuntimeError
{
public:
    QStringList m_possibleTypes;

    MultiplePossibleTypesError(const QStringList& possibleTypes)
        : RuntimeError(QString("Given data matches multiple types (%1 possible types)").arg(possibleTypes.length()))
    {
        // we store possible types so that developers can catch this and offer a choice to the user
        m_possibleTypes = possibleTypes;
    }
};

/*!
\brief NoPossibleTypesError

Exception thrown when no types match given data (from the guessType method), user should be
    asked to choose the right type in this case

*/
class NoPossibleTypesError : public RuntimeError
{
public:
    NoPossibleTypesError()
        : RuntimeError("Can't decide type of given code and extension, no positives turned up!")
    {

    }
};

/*!
\brief Manager

Manager holds type detectors and compatibility layers and is able to perform transformation between data.

    It is usually used as a singleton and this is just the base class! See compatibility.imageset.Manager for
    example of use of this class

*/
class Manager
{
public:
    QMap<QString, QString> CEGUIVersionTypes;
    QString EditorNativeType;
    QList<Layer*> m_layers;
    QList<TypeDetector*> m_detectors;

    Manager()
    {
        // derived Managers should override this and provide the info
 //       CEGUIVersionTypes = {};
        // as well as this
        EditorNativeType = "";
    }

    QStringList getKnownTypes();

    QSet<QString> getAllPossibleExtensions();

    QStringList getCEGUIVersionsCompatibleWithType(const QString& type);

    QString getSuitableDataTypeForCEGUIVersion(const QString& ceguiVersion);

      /**Performs transformation of given source code from sourceType to targetType.

      TODO: This method doesn't even bother to try to find the shortest path possible or such,
            I leave this as an exercise for future generations :-D
      */
    QString transform(const QString& sourceType, const QString& targetType, const QString &data, const QList<Layer*> visitedLayers = QList<Layer*>());

    /**Attempts to make an informed guess based on given data and extension. If the guess is positive, the
        data *should be* of returned type. It depends on type detectors however.

        If you pass full file path instead of the extension, the extension will be extracted from it.
        */
    QString guessType(const QString &code, const QString& extension = "");

    /**Transforms given code to given target type.

    extension is optional and used as a hint for the type guessing, you can pass the full file path,
    extension will be extracted.

    This method tries to guess type of given code and extension, in the case this fails, exception is thrown.
    */
    QString transformTo(const QString& targetType, const QString &code, const QString& extension = "");
};

const QStringList CEGUIVersions = { "0.6", "0.7", "0.8" };
const QString EditorEmbeddedCEGUIVersion = "0.8";

} // namespace compatibility
} // namespace CEED

#endif
