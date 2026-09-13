#include "Controller.h"

Controller::Controller(QObject* parent) : QObject(parent) {}

void Controller::setClearColor(const QColor& color) {
    if (clearColor_ == color) {
        return;
    }
    clearColor_ = color;
    emit clearColorChanged();
}
