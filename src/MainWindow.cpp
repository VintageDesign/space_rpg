#include "MainWindow.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QQmlContext>
#include <QQuickWidget>
#include <QSplitter>
#include <QUrl>
#include <QWidget>

#include "Controller.h"
#include "VulkanWindow.h"

MainWindow::MainWindow(VulkanWindow* vulkanWindow, Controller* controller,
                       QWidget* parent)
    : QMainWindow(parent) {

    this->installEventFilter(this);

    // Wraps the QWindow in a widget so it can live in a layout.
    QWidget* vulkanContainer =
        QWidget::createWindowContainer(vulkanWindow, this);
    vulkanContainer->setMinimumWidth(320);
    vulkanContainer->setFocusPolicy(Qt::StrongFocus);

    // On Wayland the embedded QWindow never receives keyboard focus itself:
    // keys go to the focused widget. So the container takes focus when the
    // game view is clicked, and forwards its key events to the Vulkan window.
    vulkanWindow_ = vulkanWindow;
    vulkanContainer_ = vulkanContainer;
    vulkanWindow->installEventFilter(this);
    vulkanContainer->installEventFilter(this);
    vulkanContainer->setFocus();

    QQuickWidget* quickWidget = new QQuickWidget(this);
    // Expose the bridge before loading, so bindings resolve on first
    // evaluation rather than erroring and re-resolving later.
    quickWidget->rootContext()->setContextProperty("controller", controller);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl(QStringLiteral("qrc:/qml/Controls.qml")));
    quickWidget->setMinimumWidth(240);

    setCentralWidget(vulkanContainer);
    setWindowTitle(QStringLiteral("Vulkan Triangle"));
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
  bool retval =false;
  if (object == vulkanWindow_ && event->type() == QEvent::MouseButtonPress) {
      vulkanContainer_->setFocus(Qt::MouseFocusReason);
  }
  if (object == vulkanContainer_ && (event->type() == QEvent::KeyPress ||
                                     event->type() == QEvent::KeyRelease)) {
      QCoreApplication::sendEvent(vulkanWindow_, event);
      retval = true;
  }

  return retval;
}
