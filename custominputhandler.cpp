#include "custominputhandler.h"

#include <QtDataVisualization/Q3DCamera>

CustomInputHandler::CustomInputHandler(QObject *parent) :
    QAbstract3DInputHandler(parent)
{
}

void CustomInputHandler::mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos)
{
    Q_UNUSED(event)
    setInputPosition(mousePos);
}

void CustomInputHandler::wheelEvent(QWheelEvent *event)
{
    int zoomLevel = scene()->activeCamera()->zoomLevel();
    if (zoomLevel > 100)
        zoomLevel += event->angleDelta().y() / 12;
    else if (zoomLevel > 50)
        zoomLevel += event->angleDelta().y() / 60;
    else
        zoomLevel += event->angleDelta().y() / 120;
    if (zoomLevel > 500)
        zoomLevel = 500;
    else if (zoomLevel < 10)
        zoomLevel = 10;

    scene()->activeCamera()->setZoomLevel(zoomLevel);
}
