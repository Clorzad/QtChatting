#include "FramelessWindow.h"
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QSpacerItem>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QVBoxLayout>
#include <qbitmap.h>
#include <qboxlayout.h>
#include <qevent.h>
#include <qgridlayout.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qtoolbutton.h>
#include <qwidget.h>


FramelessWindow::FramelessWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    mainWindow = new FramelessWidget;
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(mainWindow);
    setMinimumSize(mainWindow->size().width() + 4, mainWindow->size().height() + 4);
    connect(mainWindow, &FramelessWidget::closeRequested, this, &QWidget::close);
}

FramelessWidget::FramelessWidget(QWidget* parent)
    : QWidget(parent)
{
    initial();
    add_shadow();
}
FramelessWidget::~FramelessWidget()
{
}

void FramelessWidget::initial()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignCenter);
    QHBoxLayout* titleLayout = new QHBoxLayout;
    mainLayout->addLayout(titleLayout);
    titleLayout->setContentsMargins(5, 5, 5, 5);
    titleLayout->setSpacing(0);
    contentWidget = new QWidget;
    mainLayout->addWidget(contentWidget);

    titleLabel = new QLabel;
    titleLabel->setFixedHeight(20);
    titleLabel->setAlignment(Qt::AlignLeft);
    titleLabel->setMinimumWidth(0);
    titleLayout->addWidget(titleLabel);
    titleLayout->addSpacerItem(new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));
    QToolButton* closeButton = new QToolButton;
    closeButton->setFixedSize(20, 20);
    closeButton->setStyleSheet("QToolButton{ border: none; image: url(:/image/assets/close_button.png);}QToolButton:hover { background-color: rgb(235,57,57); border-radius: 6px; }");
    titleLayout->addWidget(closeButton);
    connect(closeButton, &QToolButton::clicked, this, &FramelessWidget::closeButtonClicked);
}

void FramelessWidget::set_titleLabel(const QPixmap& pixmap)
{
    titleLabel->setPixmap(pixmap);
}

void FramelessWidget::set_titleLabel(const QPixmap& pixmap, const QString title)
{
    titleLabel->setPixmap(pixmap);
    titleLabel->setText(title);
}

void FramelessWidget::set_titleLabel(const QString title)
{
    titleLabel->setText(title);
}

void FramelessWidget::paintEvent(QPaintEvent* ev)
{

    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 6, 6);
    painter.setClipPath(path);
    if (background.isNull())
        painter.fillPath(path, QColor(255, 255, 255, 100));
    else
        painter.drawPixmap(rect(), background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    QWidget::paintEvent(ev);
}

void FramelessWidget::add_shadow()
{
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(10);
    effect->setOffset(0, 0);
    effect->setColor(QColor(0, 0, 0, 150));
    this->setGraphicsEffect(effect);
}

void FramelessWidget::closeButtonClicked()
{
    emit closeRequested();
    this->close();
}

void FramelessWindow::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        m_bDragging = true;
        m_dragStartPos = ev->globalPosition().toPoint();
        m_windowStarPos = this->pos();
    }
    QWidget::mousePressEvent(ev);
}

void FramelessWindow::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_bDragging == true) {
        QPoint delta = ev->globalPosition().toPoint() - m_dragStartPos;
        QPoint newPos = m_windowStarPos + delta;
        this->move(newPos);
    }
    QWidget::mouseMoveEvent(ev);
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
        m_bDragging = false;
    QWidget::mouseReleaseEvent(ev);
}

QWidget* FramelessWidget::get_contentWidget() const
{
    return this->contentWidget;
}

void FramelessWidget::set_background(const QString& imagePath)
{
    background.load(imagePath);
    update();
}