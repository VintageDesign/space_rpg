#include <QApplication>
#include <QVulkanInstance>

#include "Controller.h"
#include "MainWindow.h"
#include "VulkanWindow.h"
#include "backend/level.h"
#include "engine/SceneTree.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Declared before the window so it outlives it: Qt tears the window down
    // first, and the instance must still be alive at that point.
    QVulkanInstance inst;
    // Qt filters this against the available layers, so an absent validation
    // layer is silently dropped rather than failing instance creation.
    inst.setLayers({"VK_LAYER_KHRONOS_validation"});
    if (!inst.create()) {
        qFatal("failed to create Vulkan instance: %d", inst.errorCode());
    }

    Controller controller;

    // Must outlive the window, whose renderer ticks it.
    engine::SceneTree tree;
    tree.root().addChild<Level>();

    // Ownership passes to the window container inside MainWindow.
    VulkanWindow* vulkanWindow = new VulkanWindow(&tree, &controller);
    vulkanWindow->setVulkanInstance(&inst);

    MainWindow mainWindow(vulkanWindow, &controller);
    mainWindow.resize(1100, 700);
    mainWindow.show();

    return app.exec();
}
