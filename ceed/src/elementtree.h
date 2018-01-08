#ifndef CEED_XML_H
#define CEED_XML_H

#include <QList>

class QIODevice;
class QXmlStreamWriter;

namespace ElementTree
{
    class ElementPrivate;

    class Element
    {
    public:
        QString text; // ???
        QString tail; // ???

        Element(const QString& name);
        ~Element();
        void set(const QString& attrib, const QString& value);
        void set(const QString& attrib, const char* value);
        void set(const QString& attrib, bool value);
        void set(const QString& attrib, int value);
        QString get(const QString& attrib, const QString& defaultValue = "");
        int getInt(const QString& attrib, int defaultValue = 0);
        void append(Element* child);
        Element* find(const QString& name);
        QList<Element*> findall(const QString& name);

         // added in C++ rewrite
        bool has(const QString& attrib);
        void remove(const QString& attrib);
        void toStream(QXmlStreamWriter& xml);

    private:
        friend class ElementTreePrivate;
        ElementPrivate* d;
    };

    class SubElement : public Element
    {
    public:
        SubElement(Element* parent, const QString &name);
    };

    class ElementTreePrivate;

    class ElementTree
    {
    public:
        ElementTree();
        ElementTree(Element* root);
        ElementTree(const QString& file);
        ~ElementTree();


        QList<Element*> findall(const QString& name);

        Element* fromString(const QString& string);

        void write(QIODevice& file, const QString& encoding, bool xml_declaration = true);

        void write(const QString& file, const QString& encoding, bool xml_declaration = true);

    private:
        ElementTreePrivate* d;
    };

    Element* fromstring(const QString& string);
    QString tostring(Element* root, const QString& encoding);
}

#endif // CEED_XML_H
