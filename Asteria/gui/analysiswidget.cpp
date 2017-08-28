#include "gui/analysiswidget.h"
#include "gui/glmeteordrawer.h"
#include "infra/analysisinventory.h"
#include "infra/asteriastate.h"
#include "gui/videodirectorymodel.h"
#include "util/timeutil.h"

#ifdef REANALYSE
    #include "infra/analysisworker.h"
#endif

#include <memory>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QTreeView>

AnalysisWidget::AnalysisWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state), inv(0), display(0) {

    display = new GLMeteorDrawer(this, this->state);

    tree = new QTreeView(this);
    model = new VideoDirectoryModel(state->videoDirPath, tree);
    tree->setModel(model);
    tree->resizeColumnToContents(0);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Display the usual symbols for each button
    QIcon playIcon(":/images/play.png");
    QIcon pauseIcon(":/images/pause.png");
    QIcon stopIcon(":/images/stop.png");
    QIcon stepfIcon(":/images/stepf.png");
    QIcon stepbIcon(":/images/stepb.png");

    play_button = new QPushButton(this);
    play_button->setIcon(playIcon);
    pause_button = new QPushButton(this);
    pause_button->setIcon(pauseIcon);
    stop_button = new QPushButton(this);
    stop_button->setIcon(stopIcon);
    stepb_button = new QPushButton(this);
    stepb_button->setIcon(stepbIcon);
    stepf_button = new QPushButton(this);
    stepf_button->setIcon(stepfIcon);
    slider = new QSlider(Qt::Horizontal, this);
    dicheckbox = new QCheckBox("&De-interlaced stepping", this);
    overlaycheckbox = new QCheckBox("&Show overlay image", this);

#ifdef REANALYSE
    reanalyse_button = new QPushButton("Reanalyse", this);
#endif

    replayThread = new ReplayVideoThread(this->state->nominalFramePeriodUs);

    // A widget to contain the control buttons
    QWidget * controls = new QWidget(this);
    QHBoxLayout * controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(play_button);
    controlsLayout->addWidget(pause_button);
    controlsLayout->addWidget(stop_button);
    controlsLayout->addWidget(stepb_button);
    controlsLayout->addWidget(stepf_button);

    QWidget * boxes = new QWidget(this);
    QVBoxLayout * boxesLayout = new QVBoxLayout;
    boxesLayout->addWidget(dicheckbox);
    boxesLayout->addWidget(overlaycheckbox);

#ifdef REANALYSE
    boxesLayout->addWidget(reanalyse_button);
#endif

    boxes->setLayout(boxesLayout);
    controlsLayout->addWidget(boxes);

    controls->setLayout(controlsLayout);

    // A widget to present the stats of the video clip
    clipLengthSecsField = new QLabel("");
    clipLengthFramesField = new QLabel("");
    utcField = new QLabel("");

    QWidget * statsDisplay = new QWidget(this);
    QHBoxLayout *statsLayout = new QHBoxLayout;
    statsLayout->addWidget(clipLengthSecsField);
    statsLayout->addWidget(clipLengthFramesField);
    statsDisplay->setLayout(statsLayout);

    QWidget * utcDisplay = new QWidget(this);
    QHBoxLayout *utcLayout = new QHBoxLayout;
    utcLayout->addWidget(utcField);
    utcDisplay->setLayout(utcLayout);

    connect(play_button, SIGNAL(pressed()), replayThread, SLOT(play()));
    connect(pause_button, SIGNAL(pressed()), replayThread, SLOT(pause()));
    connect(stop_button, SIGNAL(pressed()), replayThread, SLOT(stop()));
    connect(stepb_button, SIGNAL(pressed()), replayThread, SLOT(stepb()));
    connect(stepf_button, SIGNAL(pressed()), replayThread, SLOT(stepf()));
    connect(dicheckbox, SIGNAL(stateChanged(int)), replayThread, SLOT(toggleDiStepping(int)));
    connect(overlaycheckbox, SIGNAL(stateChanged(int)), replayThread, SLOT(toggleOverlay(int)));

#ifdef REANALYSE
    connect(reanalyse_button, SIGNAL(pressed()), this, SLOT(reanalyse()));
#endif

    // Slider response to user actions in the player
    connect(replayThread, SIGNAL(queuedFrameIndex(int)), slider, SLOT(setValue(int)));

    // Player response to user moving the slider
    connect(slider, SIGNAL(sliderMoved(int)), replayThread, SLOT(queueFrameIndex(int)));

    // Display image when one is queued
    connect(replayThread, SIGNAL(queueNewFrame(std::shared_ptr<Image>, bool, bool, bool)), display, SLOT(newFrame(std::shared_ptr<Image>, bool, bool, bool)));
    connect(replayThread, SIGNAL (videoStats(const AnalysisVideoStats &)), this, SLOT (updateVideoStats(const AnalysisVideoStats &)));

    // Capture right-clicks in the tree view for displaying context menu
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    // Capture double-clicks in the tree view for replaying videos
    connect(tree, SIGNAL (doubleClicked(const QModelIndex)), this, SLOT(replayVideo(const QModelIndex)));

    // Arrange layout
    QVBoxLayout *rightPanelLayout = new QVBoxLayout;
    rightPanelLayout->addWidget(display);
    rightPanelLayout->addWidget(statsDisplay);
    rightPanelLayout->addWidget(utcDisplay);
    rightPanelLayout->addWidget(slider);
    rightPanelLayout->addWidget(controls);
    QWidget * rightPanel = new QWidget(this);
    rightPanel->setLayout(rightPanelLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tree);
    mainLayout->addWidget(rightPanel);

    this->setLayout(mainLayout);
}

void AnalysisWidget::loadClip(QString path) {

    // If there's already an AnalysisInventory loaded then delete it
    if(inv) {
        delete inv;
    }

    inv = AnalysisInventory::loadFromDir(path.toStdString());

    if(!inv) {
        // Couldn't load from dir!
        fprintf(stderr, "Couldn't load analysis from %s\n", path.toStdString().c_str());
        return;
    }

    // Initialise the overlay image switch
    overlaycheckbox->setEnabled(true);
    overlaycheckbox->setChecked(true);

    // Set the range of the slider according to how many frames we have
    slider->setRange(0, inv->eventFrames.size()-1);
    slider->setValue(0);

    // Enable/disable the de-interlaced stepping checkbox depending on whether the clip consists of
    // interlaced-scan type images
    switch(inv->eventFrames[0]->field) {
    case V4L2_FIELD_NONE:
        // progressive format; not interlaced
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(false);
        break;
    case V4L2_FIELD_INTERLACED:
        // interleaved/interlaced format
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(true);
        break;
    case V4L2_FIELD_INTERLACED_TB:
        // interleaved/interlaced format; top field is transmitted first
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(true);
        break;
    case V4L2_FIELD_INTERLACED_BT:
        // interleaved/interlaced format; bottom field is transmitted first
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(true);
        break;
    }

    // Pass the clip to the player
    replayThread->loadClip(inv->eventFrames, inv->peakHold);

    // Initialise it with the peak hold image
    display->newFrame(inv->peakHold, true, true, true);
}

void AnalysisWidget::updateVideoStats(const AnalysisVideoStats &stats) {
    utcField->setText(QString::fromStdString(stats.utc));
    QString clipLengthSecsStr;
    clipLengthSecsStr.sprintf("%05.2f / %05.2f", stats.framePositionSecs, stats.clipLengthSecs);
    clipLengthSecsField->setText(clipLengthSecsStr);

    // Unicode symbols to use when displaying full frame, top field & bottom field
    QString both = QString::fromUtf8("\u25CF");
    QString top = QString::fromUtf8("\u25D3");
    QString bottom = QString::fromUtf8("\u25D2");

    QString symbol = (stats.isTopField && stats.isBottomField) ? both : (stats.isTopField ? top : bottom);
    clipLengthFramesField->setText(QString("%1 %2 / %3").arg(QString::number(stats.framePositionFrames), symbol, QString::number(stats.clipLengthFrames)));
}

#ifdef REANALYSE
    void AnalysisWidget::reanalyse() {
        fprintf(stderr, "Reanalysing...\n");
        // If there's no clip loaded, bail
        if(!inv) {
            fprintf(stderr, "No clip to analyse!\n");
            return;
        }
        QThread* thread = new QThread;
        AnalysisWorker* worker = new AnalysisWorker(NULL, this->state, inv->eventFrames);
        worker->moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(finished(std::string)), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished(std::string)), worker, SLOT(deleteLater()));
        connect(worker, SIGNAL(finished(std::string)), this, SLOT(reanalysisComplete(std::string)));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }

    void AnalysisWidget::reanalysisComplete(std::string utc) {
        fprintf(stderr, "Finished reanalysing %s\n", utc.c_str());
    }
#endif

void AnalysisWidget::replayVideo(const QModelIndex &index) {

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    QString title = item->data(0).toString();
    QString path = item->data(1).toString();

    // Detect if user has double clicked on a node that is not a video clip
    if(!std::regex_match (title.toStdString().c_str(), TimeUtil::getTimeRegex())) {
        // Not a clip (clips have titles like 01:34:56). Do nothing.
        return;
    }

    // Load the clip for display
    loadClip(path);
}

void AnalysisWidget::onCustomContextMenu(const QPoint &point) {
    QModelIndex index = tree->indexAt(point);
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        item->getContextMenu()->exec(tree->viewport()->mapToGlobal(point));
    }
}
