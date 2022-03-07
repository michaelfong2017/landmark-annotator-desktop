#include "viewtab.h"

ViewTab::ViewTab(DesktopApp* parent)
{
    this->parent = parent;

    this->timer = new QTimer;
    QObject::connect(timer, &QTimer::timeout, [this]() {
        if (this->parent->deviceCount > 0) {
            switch (k4a_device_get_capture(this->parent->device, &this->parent->capture, K4A_WAIT_INFINITE)) {
            case K4A_WAIT_RESULT_SUCCEEDED:
                break;
            }

            if (this->parent->capture) {
                k4a_image_t k4aColorImage = k4a_capture_get_color_image(this->parent->capture);

                if (k4aColorImage != NULL) {
                    this->parent->colorImageQueue.push(k4aColorImage);

                    int width = this->parent->ui.graphicsViewVideo->width(), height = this->parent->ui.graphicsViewVideo->height();
                    QImage qColorImage = (this->parent->getQColorImage()).scaled(width, height, Qt::KeepAspectRatio);

                    // Deallocate heap memory used by previous GGraphicsScene object
                    if (this->parent->ui.graphicsViewVideo->scene()) {
                        delete this->parent->ui.graphicsViewVideo->scene();
                    }
                    
                    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qColorImage));
                    QGraphicsScene* scene = new QGraphicsScene;
                    scene->addItem(item);

                    this->parent->ui.graphicsViewVideo->setScene(scene);
                    this->parent->ui.graphicsViewVideo->show();

                    while (this->parent->colorImageQueue.size() > MAX_IMAGE_QUEUE_SIZE) {
                        k4a_image_release(this->parent->colorImageQueue.front());
                        this->parent->colorImageQueue.pop();
                    }
                }

                k4a_image_t k4aDepthImage = k4a_capture_get_depth_image(this->parent->capture);

                if (k4aDepthImage != NULL) {
                    this->parent->depthImageQueue.push(k4aDepthImage);

                    int width = this->parent->ui.graphicsViewVideo2->width(), height = this->parent->ui.graphicsViewVideo2->height();
                    QImage qDepthImage = (this->parent->getQDepthImage()).scaled(width, height, Qt::KeepAspectRatio);

                    // Deallocate heap memory used by previous GGraphicsScene object
                    if (this->parent->ui.graphicsViewVideo->scene()) {
                        delete this->parent->ui.graphicsViewVideo2->scene();
                    }

                    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qDepthImage));
                    QGraphicsScene* scene = new QGraphicsScene;
                    scene->addItem(item);

                    this->parent->ui.graphicsViewVideo2->setScene(scene);
                    this->parent->ui.graphicsViewVideo2->show();

                    while (this->parent->depthImageQueue.size() > MAX_IMAGE_QUEUE_SIZE) {
                        k4a_image_release(this->parent->depthImageQueue.front());
                        this->parent->depthImageQueue.pop();
                    }
                }

                k4a_image_t k4aIRImage = k4a_capture_get_ir_image(this->parent->capture);

                if (k4aIRImage != NULL) {
                    this->parent->irImageQueue.push(k4aIRImage);

                    int width = this->parent->ui.graphicsViewVideo3->width(), height = this->parent->ui.graphicsViewVideo3->height();
                    QImage qIRImage = (this->parent->getQIRImage()).scaled(width, height, Qt::KeepAspectRatio);

                    // Deallocate heap memory used by previous GGraphicsScene object
                    if (this->parent->ui.graphicsViewVideo->scene()) {
                        delete this->parent->ui.graphicsViewVideo3->scene();
                    }

                    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qIRImage));
                    QGraphicsScene* scene = new QGraphicsScene;
                    scene->addItem(item);

                    this->parent->ui.graphicsViewVideo3->setScene(scene);
                    this->parent->ui.graphicsViewVideo3->show();

                    while (this->parent->irImageQueue.size() > MAX_IMAGE_QUEUE_SIZE) {
                        k4a_image_release(this->parent->irImageQueue.front());
                        this->parent->irImageQueue.pop();
                    }
                }

                k4a_capture_release(this->parent->capture);
            }
            else {
                qDebug() << "No capture found\n";
            }
        }
    });
}

