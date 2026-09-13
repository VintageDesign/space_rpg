#pragma once

#include <QColor>
#include <QObject>

// Bridge between the QML UI and the renderer.
//
// QVulkanWindow renders on the main thread, so TriangleRenderer may read this
// directly in startNextFrame() with no locking.
//
// This is the seam for UI work: add a Q_PROPERTY here, bind it in Controls.qml,
// read it in the renderer.
class Controller : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY
                   clearColorChanged)

public:
    explicit Controller(QObject* parent = nullptr);

    QColor clearColor() const { return clearColor_; }
    void setClearColor(const QColor& color);

signals:
    void clearColorChanged();

private:
    QColor clearColor_{0, 0, 0};
};
