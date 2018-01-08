#include "elementtree.h"

#include "qtorderedmap.h"

#include <QBuffer>
#include <QDebug>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>

namespace ElementTree {

class ElementPrivate
{
public:
    QString name;
    OrderedMap<QString, QString> attributes;
    QList<Element*> children;
};

/////

Element::Element(const QString &name)
    : d(new ElementPrivate())
{
    d->name = name;
}

Element::~Element()
{
    delete d;
}

void Element::set(const QString &attrib, const QString &value)
{
    d->attributes[attrib] = value;
}

void Element::set(const QString &attrib, const char *value)
{
    d->attributes[attrib] = QLatin1String(value);
}

void Element::set(const QString &attrib, bool value)
{
    d->attributes[attrib] = value ? QStringLiteral("true") : QStringLiteral("false");
}

void Element::set(const QString &attrib, int value)
{
    d->attributes[attrib] = QString::number(value);
}

QString Element::get(const QString &attrib, const QString &defaultValue)
{
    return d->attributes.value(attrib, defaultValue);
}

int Element::getInt(const QString &attrib, int defaultValue)
{
    auto it = d->attributes.find(attrib);
    if (it == d->attributes.end()) {
        return defaultValue;
    }
    bool ok;
    int value = it.value().toInt(&ok);
    return ok ? value : defaultValue;
}

void Element::append(Element *child)
{
    d->children << child;
}

Element *Element::find(const QString &name)
{
    for (auto child : d->children) {
        if (child->d->name == name) {
            return child;
        }
    }
    return nullptr;
}

QList<Element *> Element::findall(const QString &name)
{
    QList<Element*> ret;
    for (auto child : d->children) {
        if (child->d->name == name) {
            ret << child;
        }
    }
    return ret;
}

bool Element::has(const QString &attrib)
{
    return d->attributes.find(attrib) != d->attributes.end();
}

void Element::remove(const QString &attrib)
{
    auto it = d->attributes.find(attrib);
    if (it == d->attributes.end())
        return;
    d->attributes.erase(it);
}

void Element::toStream(QXmlStreamWriter &xml)
{
    xml.writeStartElement(d->name);
    for (auto it = d->attributes.begin(); it != d->attributes.end(); it++) {
        xml.writeAttribute(it.key(), it.value());
    }
//    xml.writeTextElement("title", "Qt Project");
    for (auto child : d->children) {
        child->toStream(xml);
    }
    xml.writeEndElement();

}

/////

SubElement::SubElement(Element *parent, const QString &name)
    : Element(name)
{
    parent->append(this);
}

/////

class ElementTreePrivate
{
public:
    ElementTreePrivate()
        : root(nullptr)
    {

    }

    ~ElementTreePrivate()
    {
        delete root;
    }

    Element* readElement()
    {
        Element *element = new Element(reader.name().toString());
        for (QXmlStreamAttribute& attribute : reader.attributes()) {
            element->set(attribute.name().toString(), attribute.value().toString());
        }

        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.tokenType() == QXmlStreamReader::StartElement) {
                if (Element *child = readElement()) {
                    element->append(child);
                }
            }
            else if (reader.tokenType() == QXmlStreamReader::EndElement) {
                return element;
            }
            else {
                qDebug() << QStringLiteral("unknown tokenType") << reader.tokenString();
            }
        }
        if (reader.hasError()) {
            delete element;
            return nullptr;
        }

        reader.skipCurrentElement();
        return element;
    }

    Element* root;
    QXmlStreamReader reader;
};


/////

ElementTree::ElementTree()
    : d(new ElementTreePrivate())
{

}

ElementTree::ElementTree(Element *root)
    : d(new ElementTreePrivate())
{
    d->root = root;
}

ElementTree::ElementTree(const QString &file)
{

}

ElementTree::~ElementTree()
{
    delete d;
}

QList<Element *> ElementTree::findall(const QString &name)
{
    return {};
}

Element *ElementTree::fromString(const QString &string)
{
    QByteArray byteArray = string.toUtf8();
    QBuffer buffer(&byteArray);
    buffer.open(QBuffer::ReadWrite);
    d->reader.setDevice(&buffer);
    if (d->reader.readNextStartElement()) {
        d->root = d->readElement();
    } else {
        return nullptr;
    }
    if (d->reader.hasError()) {
        return nullptr;
    }
    Element* root = d->root;
    d->root = nullptr; // owned by caller
    return root;
}

void ElementTree::write(QIODevice &file, const QString &encoding, bool xml_declaration)
{

}

/////

Element *fromstring(const QString &string)
{
    ElementTree tree;
    return tree.fromString(string);
}

QString tostring(Element *root, const QString &encoding)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QBuffer::ReadWrite | QBuffer::Text);
    QXmlStreamWriter writer;
    writer.setDevice(&buffer);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    root->toStream(writer);
    writer.writeEndDocument();
    return QString::fromUtf8(byteArray);
}

} // namespace ElementTree
