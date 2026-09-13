#include "VulkanWindow.h"

#include <QKeyEvent>

#include "SceneRenderer.h"
#include "engine/SceneTree.h"

VulkanWindow::VulkanWindow(engine::SceneTree* tree, Controller* controller)
    : tree_(tree), controller_(controller) {}

QVulkanWindowRenderer* VulkanWindow::createRenderer() {
    // Qt takes ownership of the returned renderer.
    return new SceneRenderer(this, tree_, controller_);
}

void VulkanWindow::keyPressEvent(QKeyEvent* event) {
    if (!event->isAutoRepeat()) {
        tree_->input.keyEvent(event->key(), true);
    }
}

void VulkanWindow::keyReleaseEvent(QKeyEvent* event) {
    if (!event->isAutoRepeat()) {
        tree_->input.keyEvent(event->key(), false);
    }
}
