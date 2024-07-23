/*
    SPDX-FileCopyrightText: 2004-2008 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2004 Enrico Ros <eros.kde@email.it>

    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _OKULAR_GENERATOR_PDF_H_
#define _OKULAR_GENERATOR_PDF_H_

//#include "synctex/synctex_parser.h"

#include <poppler-qt5.h>
#include <poppler-version.h>

#define POPPLER_VERSION_MACRO ((POPPLER_VERSION_MAJOR << 16) | (POPPLER_VERSION_MINOR << 8) | (POPPLER_VERSION_MICRO))

#include <QBitArray>
#include <QPointer>

#include <core/annotations.h>
#include <core/document.h>
#include <core/generator.h>
#include <core/printoptionswidget.h>
#include <interfaces/configinterface.h>
#include <interfaces/printinterface.h>
#include <interfaces/saveinterface.h>

#include <QAbstractScrollArea>

#include <unordered_map>

#include <QEvent>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <KAboutData>
#include <KConfigDialog>
#include <KLocalizedString>
#include <core/page.h>
#include <core/utils.h>
#include <vector>
#include <fstream>
#include <cstring>

#include <core/textdocumentgenerator.h>
#include <core/textdocumentgenerator_p.h>
#include <QApplication>
#include <QtWidgets>
#include <QPushButton>

#include "../Okular-v3d-Plugin-Code/src/Rendering/renderheadless.h"
#include "../Okular-v3d-Plugin-Code/src/V3dFile/V3dFile.h"
#include "../Okular-v3d-Plugin-Code/src/Utility/Arcball.h"
#include "../Okular-v3d-Plugin-Code/src/Utility/ProtectedFunctionCaller.h"

#include "glm/gtx/string_cast.hpp"

class PDFOptionsPage;
class PopplerAnnotationProxy;

/**
 * @short A generator that builds contents from a PDF document.
 *
 * All Generator features are supported and implemented by this one.
 * Internally this holds a reference to xpdf's core objects and provides
 * contents generation using the PDFDoc object and a couple of OutputDevices
 * called Okular::OutputDev and Okular::TextDev (both defined in gp_outputdev.h).
 *
 * For generating page contents we tell PDFDoc to render a page and grab
 * contents from out OutputDevs when rendering finishes.
 *
 */
class PDFGenerator : public Okular::Generator, public Okular::ConfigInterface, public Okular::PrintInterface, public Okular::SaveInterface
{
    Q_OBJECT
    Q_INTERFACES(Okular::Generator)
    Q_INTERFACES(Okular::ConfigInterface)
    Q_INTERFACES(Okular::PrintInterface)
    Q_INTERFACES(Okular::SaveInterface)

// ==================================== Custom Addition ====================================
public:
    class EventFilter : public QObject {
    public:
        EventFilter(QObject* parent, PDFGenerator* generator)
            : QObject(parent), generator(generator) { }
        ~EventFilter() override = default;

        bool eventFilter(QObject *object, QEvent *event) {
            if (generator == nullptr) {
                return false;
            }

            if (event->type() == QEvent::MouseMove) {
                QMouseEvent* mouseMove = dynamic_cast<QMouseEvent*>(event);

                if (mouseMove != nullptr) {
                    return generator->mouseMoveEvent(mouseMove);
                }

            } else if (event->type() == QEvent::MouseButtonPress) {
                QMouseEvent* mousePress = dynamic_cast<QMouseEvent*>(event);

                if (mousePress != nullptr) {
                    return generator->mouseButtonPressEvent(mousePress);
                }

            } else if (event->type() == QEvent::MouseButtonRelease) {
                QMouseEvent* mouseRelease = dynamic_cast<QMouseEvent*>(event);

                if (mouseRelease != nullptr) {
                    return generator->mouseButtonReleaseEvent(mouseRelease);
                }
            }

            return false;
        }

        PDFGenerator* generator;
    };

    bool mouseMoveEvent(QMouseEvent* event);
    bool mouseButtonPressEvent(QMouseEvent* event);
    bool mouseButtonReleaseEvent(QMouseEvent* event);

private:
    void dragModeShift  (const glm::vec2& normalizedMousePosition, const glm::vec2& lastNormalizedMousePosition);
    void dragModeZoom   (const glm::vec2& normalizedMousePosition, const glm::vec2& lastNormalizedMousePosition);
    void dragModePan    (const glm::vec2& normalizedMousePosition, const glm::vec2& lastNormalizedMousePosition);
    void dragModeRotate (const glm::vec2& normalizedMousePosition, const glm::vec2& lastNormalizedMousePosition);

    void initProjection();
    void setProjection();

    void setDimensions(float width, float height, float X, float Y);
    void updateViewMatrix();

    void requestPixmapRefresh();
    void refreshPixmap();

    std::chrono::duration<double> m_MinTimeBetweenRefreshes{ 1.0 / 100.0 }; // In Seconds
    std::chrono::time_point<std::chrono::system_clock> m_LastPixmapRefreshTime;

    bool m_MouseDown{ false };

    enum class DragMode {
        SHIFT,
        ZOOM,
        PAN,
        ROTATE
    };
    DragMode m_DragMode{ DragMode::ROTATE };

    glm::ivec2 m_MousePosition;
    glm::ivec2 m_LastMousePosition;

    float m_Zoom = 1.0f;
    float m_LastZoom{ };

    float xShift;
    float yShift;

    glm::vec2 m_PageViewDimensions;

    glm::mat4 m_RotationMatrix{ 1.0f };
    glm::mat4 m_ViewMatrix{ 1.0f };
    glm::mat4 m_ProjectionMatrix{ 1.0f };

    float m_H{ };
    glm::vec3 m_Center{ };
    glm::vec2 m_Shift{ };

    struct ViewParam {
        glm::vec3 minValues{ };
        glm::vec3 maxValues{ };
    };

    ViewParam m_ViewParam;

    std::unique_ptr<V3dFile> m_File{ nullptr };

private:
    HeadlessRenderer* m_HeadlessRenderer;

    void CustomConstructor();
    void CustomDestructor();

    QAbstractScrollArea* m_PageView{ nullptr };

    EventFilter* m_EventFilter{ nullptr };

// ================================= End of Custom Addition =================================

public:
    PDFGenerator(QObject *parent, const QVariantList &args);
    ~PDFGenerator() override;

    // [INHERITED] load a document and fill up the pagesVector
    Okular::Document::OpenResult loadDocumentWithPassword(const QString &filePath, QVector<Okular::Page *> &pagesVector, const QString &password) override;
    Okular::Document::OpenResult loadDocumentFromDataWithPassword(const QByteArray &fileData, QVector<Okular::Page *> &pagesVector, const QString &password) override;
    void loadPages(QVector<Okular::Page *> &pagesVector, int rotation = -1, bool clear = false);
    // [INHERITED] document information
    Okular::DocumentInfo generateDocumentInfo(const QSet<Okular::DocumentInfo::Key> &keys) const override;
    const Okular::DocumentSynopsis *generateDocumentSynopsis() override;
    Okular::FontInfo::List fontsForPage(int page) override;
    const QList<Okular::EmbeddedFile *> *embeddedFiles() const override;
    PageSizeMetric pagesSizeMetric() const override
    {
        return Pixels;
    }
    QAbstractItemModel *layersModel() const override;
    void opaqueAction(const Okular::BackendOpaqueAction *action) override;
    void freeOpaqueActionContents(const Okular::BackendOpaqueAction &action) override;

    // [INHERITED] document information
    bool isAllowed(Okular::Permission permission) const override;

    // [INHERITED] perform actions on document / pages
    QImage image(Okular::PixmapRequest *request) override;

    // [INHERITED] print page using an already configured kprinter
    Okular::Document::PrintError print(QPrinter &printer) override;

    // [INHERITED] reply to some metadata requests
    QVariant metaData(const QString &key, const QVariant &option) const override;

    // [INHERITED] reparse configuration
    bool reparseConfig() override;
    void addPages(KConfigDialog *) override;

    // [INHERITED] text exporting
    Okular::ExportFormat::List exportFormats() const override;
    bool exportTo(const QString &fileName, const Okular::ExportFormat &format) override;

    // [INHERITED] print interface
    Okular::PrintOptionsWidget *printConfigurationWidget() const override;

    // [INHERITED] save interface
    bool supportsOption(SaveOption) const override;
    bool save(const QString &fileName, SaveOptions options, QString *errorText) override;
    Okular::AnnotationProxy *annotationProxy() const override;

    bool canSign() const override;
    bool sign(const Okular::NewSignatureData &oData, const QString &rFilename) override;

    Okular::CertificateStore *certificateStore() const override;

    QByteArray requestFontData(const Okular::FontInfo &font) override;

    static void okularToPoppler(const Okular::NewSignatureData &oData, Poppler::PDFConverter::NewSignatureData *pData);

protected:
    SwapBackingFileResult swapBackingFile(QString const &newFileName, QVector<Okular::Page *> &newPagesVector) override;
    bool doCloseDocument() override;
    Okular::TextPage *textPage(Okular::TextRequest *request) override;

private:
    Okular::Document::OpenResult init(QVector<Okular::Page *> &pagesVector, const QString &password);

    // create the document synopsis hierarchy
    void addSynopsisChildren(const QVector<Poppler::OutlineItem> &outlineItems, QDomNode *parentDestination);
    // fetch annotations from the pdf file and add they to the page
    void addAnnotations(Poppler::Page *popplerPage, Okular::Page *page);
    // fetch the transition information and add it to the page
    void addTransition(Poppler::Page *pdfPage, Okular::Page *page);
    // fetch the poppler page form fields
    QList<Okular::FormField *> getFormFields(Poppler::Page *popplerPage);

    Okular::TextPage *abstractTextPage(const QList<Poppler::TextBox *> &text, double height, double width, int rot);

    void resolveMediaLinkReferences(Okular::Page *page);
    void resolveMediaLinkReference(Okular::Action *action);

    bool setDocumentRenderHints();

    // poppler dependent stuff
    Poppler::Document *pdfdoc;

    void xrefReconstructionHandler();

    // misc variables for document info and synopsis caching
    bool docSynopsisDirty;
    bool xrefReconstructed;
    Okular::DocumentSynopsis docSyn;
    mutable bool docEmbeddedFilesDirty;
    mutable QList<Okular::EmbeddedFile *> docEmbeddedFiles;
    int nextFontPage;
    PopplerAnnotationProxy *annotProxy;
    mutable Okular::CertificateStore *certStore;
    // the hash below only contains annotations that were present on the file at open time
    // this is enough for what we use it for
    QHash<Okular::Annotation *, Poppler::Annotation *> annotationsOnOpenHash;

    QBitArray rectsGenerated;

    QPointer<PDFOptionsPage> pdfOptionsPage;
};

#endif

/* kate: replace-tabs on; indent-width 4; */
