from PySide import QtGui
from PySide import QtCore

class XMLSyntaxHighlighter(QtGui.QSyntaxHighlighter):
    def __init__(self, parent=None):
        super(XMLSyntaxHighlighter, self).__init__(parent)
 
        self.highlightingRules = []
 
        # todo: some fail colour highlighting :D please someone change the colours
        keywordFormat = QtGui.QTextCharFormat()
        keywordFormat.setFontWeight(QtGui.QFont.Bold)
        keywordFormat.setForeground(QtCore.Qt.darkCyan)
        
        for pattern in ["\\b?xml\\b", "/>", ">", "</", "<"]:
            self.highlightingRules.append((QtCore.QRegExp(pattern), keywordFormat))
 
        elementNameFormat = QtGui.QTextCharFormat()
        elementNameFormat.setFontWeight(QtGui.QFont.Bold)
        elementNameFormat.setForeground(QtCore.Qt.red)
        
        self.highlightingRules.append((QtCore.QRegExp("\\b[A-Za-z0-9_]+(?=[\s/>])"), elementNameFormat))
 
        attributeKeyFormat = QtGui.QTextCharFormat()
        attributeKeyFormat.setFontItalic(True)
        attributeKeyFormat.setForeground(QtCore.Qt.blue)
        self.highlightingRules.append((QtCore.QRegExp("\\b[A-Za-z0-9_]+(?=\\=)"), attributeKeyFormat))
        
    def highlightBlock(self, text):
        for expression, format in self.highlightingRules:
            index = expression.indexIn(text)

            while index >= 0:
                length = expression.matchedLength()
                self.setFormat(index, length, format)
 
                index = expression.indexIn(text, index + length)
    
class XMLEditWidget(QtGui.QTextEdit):
    def __init__(self):
        super(XMLEditWidget, self).__init__()
        
        self.setAcceptRichText(False)
        self.zoomIn()
        
        # todo: we want dark background!! This doesn't entirely work the way I would want it to,
        #       only areas that have text are with black background :-(
        #self.setTextBackgroundColor(QtGui.QColor(0, 0, 0))
        self.highlighter = XMLSyntaxHighlighter(self.document())
        