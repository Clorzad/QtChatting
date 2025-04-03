#pragma once
#include <QLabel>
#include <QWidget>
#include <qboxlayout.h>
#include <qevent.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class FramelessWidget;
class FramelessWindow : public QWidget {
    Q_OBJECT
public:
    FramelessWindow(QWidget* parent = nullptr);

    void setMainStyleSheet(QString styleSheet);

    void setSubStyleSheet(QString styleSheet);


    FramelessWidget* mainWindow;

private:
    // 拖动状态标志
    bool m_bDragging = false;
    // 鼠标按下时的全局位置
    QPoint m_dragStartPos;
    // 窗口的初始位置
    QPoint m_windowStarPos;

    void mousePressEvent(QMouseEvent* ev) override;

    void mouseMoveEvent(QMouseEvent* ev) override;

    void mouseReleaseEvent(QMouseEvent* ev) override;
};

class FramelessWidget : public QWidget {
    Q_OBJECT
signals:
    void closeRequested();

public:
    FramelessWidget(QWidget* parent = nullptr);
    ~FramelessWidget();
    void set_titleLabel(const QPixmap& pixmap);

    void set_titleLabel(const QPixmap& pixmap, const QString title);

    void set_titleLabel(const QString title);

    /**
     * @brief 获取内容区域的QWidget
     * @param 参数名 参数说明
     * @return 指向内容区域的QWidget指针
     */
    
    QWidget* get_contentWidget() const;

    /**
     * @brief 设置窗口背景
     * @param imagePath 图片路径
     * @return 无返回值
     */
    
    void set_background(const QString& imagePath);
    

private:
    QLabel* titleLabel;

    QWidget* contentWidget;

    QPixmap background;

    void initial();

    void paintEvent(QPaintEvent* ev) override;

    void add_shadow();

    void closeButtonClicked();


};