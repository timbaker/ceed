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

#include "compatibility_ceguihelpers.h"

#include <QBuffer>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>

namespace  CEED {
namespace compatibility {
namespace ceguihelpers {

/**Checks that tag of the root element in data is as given
and checks that version recorded in the root element is given
(can be None if no version information should be there)

Returns true if everything went well and all matches,
false otherwise.

NOTE: Implemented using SAX for speed
*/
bool checkDataVersion(const QString& rootElement, const QString &version, const QString &data)
{
#if 1
    QByteArray byteArray = data.toUtf8();
    QBuffer buffer(&byteArray);
    buffer.open(QBuffer::ReadWrite);
    QXmlStreamReader xml;
    xml.setDevice(&buffer);
    if (xml.readNextStartElement()) {
        if (rootElement != xml.name()) {
            return false;
        }
        if (!version.isEmpty()) {
            if (xml.attributes().hasAttribute("version")) {
                return xml.attributes().value("version") == version;
            }
        }
    }
#else
    class RootElement : public Exception
    {
    public:
        QString m_tag;
        QString m_version;

        RootElement(const QString& tag, const QString& version):
            Exception("")
        {
            m_tag = tag;
            m_version = version;
        }
    };

    class REHandler : public handler::ContentHandler
    {
    public:
        REHandler()
            : handler::ContentHandler()
        {

        }

        void startElement(const QString& name, attrs)
        {
            QString version = "";
            if (attrs.has_key("version"))
                version = attrs["version"];

            throw RootElement(name, version);
        }
    };

    try {
        parseString(data, REHandler());

    } catch (RootElement re) {
        if (re.m_tag == rootElement && re.m_version == version) {
            return true;
        }
    }
    catch (...) {
    }
#endif
    return false;
}

QString prettyPrintXMLElement(ElementTree::Element *rootElement)
{
    /**Takes an ElementTree.Element and returns a pretty printed UTF-8 XML file as string
    based on it. This functions adds newlines and indents and adds the XML declaration
    on top, which would be otherwise missing.

    Returns a string containing the pretty printed XML file.
    */
    xmledit::indent(rootElement);

    QBuffer tempFile;
    tempFile.open(QBuffer::ReadWrite);
    ElementTree::ElementTree elementTree(rootElement);
    elementTree.write(tempFile, "utf-8", true);
    return QString::fromUtf8( tempFile.buffer().data(), tempFile.buffer().length() );
}

} // namespace ceguihelpers
} // namespace compatibility
} // namespace CEED
