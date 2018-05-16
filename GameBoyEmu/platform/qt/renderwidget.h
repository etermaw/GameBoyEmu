#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    public:
        explicit RenderWidget(QWidget* parent = nullptr);
        ~RenderWidget();

    protected:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

    private:
        QOpenGLShaderProgram shaders;
        int vertex_location;
        int color_location;
        GLuint vert_vbo, color_vbo;
};

#endif // RENDERWIDGET_H
