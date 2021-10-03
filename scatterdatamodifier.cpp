#include "scatterdatamodifier.h"
#include <QtDataVisualization/QScatterDataProxy>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DScene>
#include <QtDataVisualization/Q3DCamera>
#include <QtDataVisualization/QScatter3DSeries>
#include <QtDataVisualization/Q3DTheme>
#include <QtCore/qmath.h>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

using namespace QtDataVisualization;

ScatterDataModifier::ScatterDataModifier(Q3DScatter *scatter)
    : m_graph(scatter),
      m_inputHandler(new CustomInputHandler())
{
    m_graph->activeTheme()->setType(Q3DTheme::ThemeDigia);
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityMedium);
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);

    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    m_graph->axisX()->setRange(-10.0f, 10.0f);
    m_graph->axisY()->setRange(-5.0f, 5.0f);
    m_graph->axisZ()->setRange(-5.0f, 5.0f);

    QScatter3DSeries *series = new QScatter3DSeries;
    series->setItemLabelFormat(QStringLiteral("@xLabel, @yLabel, @zLabel"));
    series->setMesh(QAbstract3DSeries::MeshCube);
    series->setItemSize(0.15f);
    m_graph->addSeries(series);

    m_animationCameraX = new QPropertyAnimation(m_graph->scene()->activeCamera(), "xRotation");
    m_animationCameraX->setDuration(20000);
    m_animationCameraX->setStartValue(QVariant::fromValue(0.0f));
    m_animationCameraX->setEndValue(QVariant::fromValue(360.0f));
    m_animationCameraX->setLoopCount(-1);

    QPropertyAnimation *upAnimation = new QPropertyAnimation(m_graph->scene()->activeCamera(), "yRotation");
    upAnimation->setDuration(9000);
    upAnimation->setStartValue(QVariant::fromValue(5.0f));
    upAnimation->setEndValue(QVariant::fromValue(45.0f));

    QPropertyAnimation *downAnimation = new QPropertyAnimation(m_graph->scene()->activeCamera(), "yRotation");
    downAnimation->setDuration(9000);
    downAnimation->setStartValue(QVariant::fromValue(45.0f));
    downAnimation->setEndValue(QVariant::fromValue(5.0f));

    m_animationCameraY = new QSequentialAnimationGroup();
    m_animationCameraY->setLoopCount(-1);
    m_animationCameraY->addAnimation(upAnimation);
    m_animationCameraY->addAnimation(downAnimation);

    m_animationCameraX->start();
    m_animationCameraY->start();

    m_graph->setActiveInputHandler(m_inputHandler);

    m_selectionTimer = new QTimer(this);
    m_selectionTimer->setInterval(10);
    m_selectionTimer->setSingleShot(false);
    QObject::connect(m_selectionTimer, &QTimer::timeout, this,
                     &ScatterDataModifier::triggerSelection);
    m_selectionTimer->start();
}

ScatterDataModifier::~ScatterDataModifier()
{
    delete m_graph;
}

void ScatterDataModifier::start()
{
    addData();
}

void ScatterDataModifier::addData()
{
    QVector<QVector3D> itemList;

    QTextStream stream;
    QFile dataFile(":/data/data.txt");
    if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        stream.setDevice(&dataFile);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("#"))
                continue;
            QStringList strList = line.split(",", QString::SkipEmptyParts);

            if (strList.size() < 3) {
                qWarning() << "Satir degeri okunamadi:" << line;
                continue;
            }
            itemList.append(QVector3D(
                                 strList.at(0).trimmed().toFloat(),
                                 strList.at(1).trimmed().toFloat(),
                                 strList.at(2).trimmed().toFloat()));
        }
    } else {
        qWarning() << "Dosya acilamadi:" << dataFile.fileName();
    }

    QScatterDataArray *dataArray = new QScatterDataArray;
    dataArray->resize(itemList.count());
    QScatterDataItem *ptrToDataArray = &dataArray->first();
    for (int i = 0; i < itemList.count(); i++) {
        ptrToDataArray->setPosition(itemList.at(i));
        ptrToDataArray++;
    }

    m_graph->seriesList().at(0)->dataProxy()->resetArray(dataArray);
}

void ScatterDataModifier::toggleCameraAnimation()
{
    if (m_animationCameraX->state() != QAbstractAnimation::Paused) {
        m_animationCameraX->pause();
        m_animationCameraY->pause();
    } else {
        m_animationCameraX->resume();
        m_animationCameraY->resume();
    }
}

void ScatterDataModifier::triggerSelection()
{
    m_graph->scene()->setSelectionQueryPosition(m_inputHandler->inputPosition());
}

void ScatterDataModifier::shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality);
}

void ScatterDataModifier::changeShadowQuality(int quality)
{
    QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}
