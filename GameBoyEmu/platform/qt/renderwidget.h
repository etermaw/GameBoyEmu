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
        int vertex_location = 0;
        int texc_location = 0;
        unsigned current_texture = 0;
        GLuint vert_vbo, texc_vbo;
        GLuint textures[2];
};

#endif // RENDERWIDGET_H
