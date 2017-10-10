#include "referencestarwidget.h"
#include "infra/asteriastate.h"
#include "gui/glmeteordrawer.h"
#include "util/coordinateutil.h"
#include "util/mathutil.h"
#include "util/timeutil.h"
#include "util/renderutil.h"
#include "gui/doubleslider.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>

ReferenceStarWidget::ReferenceStarWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state) {

    medianImageViewer = new GLMeteorDrawer(this, this->state->width, this->state->height);

    // Capture mouse clicks etc in the GLMeteorDrawer
    medianImageViewer->installEventFilter(this);

    QPushButton * up_button = new QPushButton(this);
    up_button->setIcon(QIcon(":/images/up.png"));
    QPushButton * down_button = new QPushButton(this);
    down_button->setIcon(QIcon(":/images/down.png"));
    QPushButton * left_button = new QPushButton(this);
    left_button->setIcon(QIcon(":/images/left.png"));
    QPushButton * right_button = new QPushButton(this);
    right_button->setIcon(QIcon(":/images/right.png"));
    QPushButton * clockwise_button = new QPushButton(this);
    clockwise_button->setIcon(QIcon(":/images/clockwise.png"));
    QPushButton * anticlockwise_button = new QPushButton(this);
    anticlockwise_button->setIcon(QIcon(":/images/anticlockwise.png"));
    QPushButton * zoomin_button = new QPushButton(this);
    zoomin_button->setIcon(QIcon(":/images/plus.png"));
    QPushButton * zoomout_button = new QPushButton(this);
    zoomout_button->setIcon(QIcon(":/images/minus.png"));
    // TODO: reference star catalogue should know the magnitude range of it's contents
    DoubleSlider * slider = new DoubleSlider(this, -1.0, 6.0, state->ref_star_faint_mag_limit, 100);

    connect(up_button, SIGNAL(pressed()), this, SLOT(up()));
    connect(down_button, SIGNAL(pressed()), this, SLOT(down()));
    connect(left_button, SIGNAL(pressed()), this, SLOT(left()));
    connect(right_button, SIGNAL(pressed()), this, SLOT(right()));
    connect(clockwise_button, SIGNAL(pressed()), this, SLOT(clockwise()));
    connect(anticlockwise_button, SIGNAL(pressed()), this, SLOT(anticlockwise()));
    connect(zoomin_button, SIGNAL(pressed()), this, SLOT(zoomin()));
    connect(zoomout_button, SIGNAL(pressed()), this, SLOT(zoomout()));
    // Player response to user moving the slider
    connect(slider, SIGNAL(doubleSliderMoved(double)), this, SLOT(slide(double)));

    refStarMagSliderGroupBox = new QGroupBox(QString("Reference stars faint magnitude limit [%1]").arg(state->ref_star_faint_mag_limit, 0, 'f', 2));
    QVBoxLayout *sliderVbox = new QVBoxLayout;
    sliderVbox->addWidget(slider);
    sliderVbox->addStretch(1);
    refStarMagSliderGroupBox->setLayout(sliderVbox);

    QHBoxLayout * controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(up_button);
    controlsLayout->addWidget(down_button);
    controlsLayout->addWidget(left_button);
    controlsLayout->addWidget(right_button);
    controlsLayout->addWidget(clockwise_button);
    controlsLayout->addWidget(anticlockwise_button);
    controlsLayout->addWidget(zoomin_button);
    controlsLayout->addWidget(zoomout_button);
    QWidget * controls = new QWidget(this);
    controls->setLayout(controlsLayout);

    QVBoxLayout *medianImageLayout = new QVBoxLayout;
    medianImageLayout->addWidget(medianImageViewer);
    medianImageLayout->addWidget(controls);
    medianImageLayout->addWidget(refStarMagSliderGroupBox);
    medianImageLayout->addStretch();
    this->setLayout(medianImageLayout);
}

void ReferenceStarWidget::loadImage(std::shared_ptr<Image> &newImage) {
    image = newImage;
    update();
}

void ReferenceStarWidget::up() {

    // Compute change in az/el required to move stars 'up' in image.
    state->elevation += 1.0 * std::cos(MathUtil::toRadians(state->roll));
    state->azimuth += 1.0 * std::sin(MathUtil::toRadians(state->roll));
    fprintf(stderr, "Performing action: up; elevation = %f\n", state->elevation);
    update();
}

void ReferenceStarWidget::down() {

    // Compute change in az/el required to move stars 'down' in image.
    state->elevation -= 1.0 * std::cos(MathUtil::toRadians(state->roll));
    state->azimuth -= 1.0 * std::sin(MathUtil::toRadians(state->roll));

    fprintf(stderr, "Performing action: down; elevation = %f\n", state->elevation);
    update();
}

void ReferenceStarWidget::left() {

    // Compute change in az/el required to move stars 'left' in image.
    state->elevation -= 1.0 * std::sin(MathUtil::toRadians(state->roll));
    state->azimuth += 1.0 * std::cos(MathUtil::toRadians(state->roll));

    fprintf(stderr, "Performing action: left; azimuth = %f\n", state->azimuth);
    update();
}

void ReferenceStarWidget::right() {

    // Compute change in az/el required to move stars 'right' in image.
    state->elevation += 1.0 * std::sin(MathUtil::toRadians(state->roll));
    state->azimuth -= 1.0 * std::cos(MathUtil::toRadians(state->roll));

    fprintf(stderr, "Performing action: right; azimuth = %f\n", state->azimuth);
    update();
}

void ReferenceStarWidget::clockwise() {
    state->roll += 1.0;
    fprintf(stderr, "Performing action: clockwise; roll = %f\n", state->roll);
    update();
}

void ReferenceStarWidget::anticlockwise() {
    state->roll -= 1.0;
    fprintf(stderr, "Performing action: anticlockwise; roll = %f\n", state->roll);
    update();
}

void ReferenceStarWidget::zoomin() {
    state->focal_length += 0.1;
    fprintf(stderr, "Performing action: zoomin; f = %f\n", state->focal_length);
    update();
}

void ReferenceStarWidget::zoomout() {
    if(state->focal_length > 0.1) {
        state->focal_length -= 0.1;
    }
    fprintf(stderr, "Performing action: zoomout; f = %f\n", state->focal_length);
    update();
}

void ReferenceStarWidget::slide(double position) {
    state->ref_star_faint_mag_limit = position;
    refStarMagSliderGroupBox->setTitle(QString("Reference stars faint magnitude limit [%1]").arg(state->ref_star_faint_mag_limit, 0, 'f', 2));
    update();
}

void ReferenceStarWidget::update() {

    if(!image) {
        // No image loaded yet
        return;
    }

    // Clear the annotated image ready to be filled in with reference stars
    image->annotatedImage.clear();
    image->annotatedImage.reserve(image->width * image->height);

    // Initialise to full transparency
    for(unsigned int p = 0; p < image->width * image->height; p++) {
        image->annotatedImage.push_back(0x00000000);
    }

    // Project the reference stars into the image
    double gmst = TimeUtil::epochToGmst(image->epochTimeUs);

    double lon = MathUtil::toRadians(state->longitude);
    double lat = MathUtil::toRadians(state->latitude);
    double az = MathUtil::toRadians(state->azimuth);
    double el = MathUtil::toRadians(state->elevation);
    double roll = MathUtil::toRadians(state->roll);

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = CoordinateUtil::getSezToCamRot(az, el, roll);
    Matrix3d r_cam_im = CoordinateUtil::getCamIntrinsicMatrix(state->focal_length, state->pixel_width, state->pixel_height, state->width, state->height);

    // Full transformation
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    // Transformation BCRF->SEZ
    Matrix3d r_bcrf_sez = r_ecef_sez * r_bcrf_ecef;

    for(ReferenceStar &star : state->refStarCatalogue) {

        // Reject stars fainter than faint mag limit
        if(star.mag > state->ref_star_faint_mag_limit) {
            continue;
        }

        // Unit vector towards star in original frame:
        Vector3d r_bcrf;
        CoordinateUtil::sphericalToCartesian(r_bcrf, 1.0, star.ra, star.dec);
        // Transform to CAM frame:
        Vector3d r_cam = r_bcrf_cam * r_bcrf;

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        // TEMP
        //
        // Transform to SEZ frame:
        Vector3d r_sez = r_bcrf_sez * r_bcrf;
        // Get position in spherical coordinates
        double r, theta, phi;
        CoordinateUtil::cartesianToSpherical(r_sez, r, theta, phi);
        // Transform east-of-south angle to east-of-north for conventional azimuth
        CoordinateUtil::eastOfSouthToEastOfNorth(theta);
        //
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        if(r_cam[2] < 0) {
            // Star is behind the camera
            continue;
        }

        // Project into image coordinates
        Vector3d r_im = r_cam_im * r_cam;

        star.i = r_im[0] / r_im[2];
        star.j = r_im[1] / r_im[2];

        if(star.i>0 && star.i<state->width && star.j>0 && star.j<state->height) {
            // Star is visible in image!
            int ii = (int)std::round(star.i);
            int jj = (int)std::round(star.j);

            // Translate magnitude of star to gap size in cross hair
            double m0 = -1.0; // Bright magnitude limit
            double g0 = 6.0; // Maximum gap; for stars at the bright magnitude limit
            double m1 = state->ref_star_faint_mag_limit; // Faint magnitude limit
            double g1 = 2.0; // Minimum gap; for stars at the faint magnitude limit

            double gap;
            if(star.mag > m1) {
                gap = g1;
            }
            else if(star.mag < m0) {
                gap = g0;
            }
            else {
                gap = g0 + (star.mag - m0) * ((g1 - g0)/(m1 - m0));
            }

            unsigned int gap_int = (unsigned int)std::round(gap);

            RenderUtil::drawCrossHair(image->annotatedImage, image->width, image->height, ii, jj, gap_int, 0xFFFF00FF);

        }

    }

    medianImageViewer->newFrame(image, true, true, true);
}

bool ReferenceStarWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QPoint mouse = medianImageViewer->mapFromGlobal(QCursor::pos());
        fprintf(stderr, "Mouse clicked at %d, %d\n", mouse.x(), mouse.y());
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}


