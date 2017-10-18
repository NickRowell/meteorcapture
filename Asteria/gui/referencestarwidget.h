#ifndef REFERENCESTARWIDGET_H
#define REFERENCESTARWIDGET_H

#include "infra/image.h"

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>

class AsteriaState;
class GLMeteorDrawer;
class QGroupBox;

/**
 * @brief Provides a QWidget used to display the median image overlaid with the current positions of
 * the reference stars and with buttons to enable interactive updating of the camera pointing etc.
 */
class ReferenceStarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReferenceStarWidget(QWidget *parent = 0, AsteriaState * state = 0);

    /**
     * @brief Handle to the object storing all state information.
     */
    AsteriaState * state;

    /**
     * @brief The image currently being displayed.
     */
    std::shared_ptr<Image> image;

    /**
     * @brief Image viewer for the median image.
     */
    GLMeteorDrawer * medianImageViewer;

    /**
     * @brief QGroupBox to contain the reference stars faint magnitude limit slider.
     */
    QGroupBox *refStarMagSliderGroupBox;

    /**
     * @brief Previous i coordinate of the mouse pointer, for handling drag events.
     */
    int mousePrevI;

    /**
     * @brief Previous j coordinate of the mouse pointer, for handling drag events.
     */
    int mousePrevJ;

    /**
     * @brief Recrods current pressed/unpressed state of the left mouse button
     */
    bool leftButtonIsPressed;

    /**
     * @brief Recrods current pressed/unpressed state of the middle mouse button
     */
    bool middleButtonIsPressed;

    /**
     * @brief Recrods current pressed/unpressed state of the right mouse button
     */
    bool rightButtonIsPressed;


signals:

public slots:

    void up();
    void down();
    void left();
    void right();
    void clockwise();
    void anticlockwise();
    void zoomin();
    void zoomout();
    void slide(double position);

    void loadImage(std::shared_ptr<Image> &newImage);

    void update();

protected:

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent * event) override;

};

#endif // REFERENCESTARWIDGET_H
