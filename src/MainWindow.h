#pragma once

#include <QMainWindow>

class Controller;
class VulkanWindow;

// Widgets shell hosting two children side by side:
//
//   [ Vulkan view (native window container) | QML controls (QQuickWidget) ]
//
// A Widgets shell is required because QVulkanWindow is a QWindow, and Qt Quick
// has no way to embed an arbitrary QWindow as a QML item -- createWindowContainer
// is Widgets-only, and QQuickRhiItem needs Qt 6.7 (this is 6.4).
//
// Note: the container is a *native* child window, and native windows always
// stack above sibling widgets. QML can sit beside or dock around the Vulkan
// view, but cannot be overlaid on top of it.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(VulkanWindow* vulkanWindow, Controller* controller,
               QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    VulkanWindow* vulkanWindow_ = nullptr;
    QWidget* vulkanContainer_ = nullptr;
};
