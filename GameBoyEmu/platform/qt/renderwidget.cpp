#include "renderwidget.h"

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent)
{
}

RenderWidget::~RenderWidget()
{
}

void RenderWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    shaders.addShaderFromSourceFile(QOpenGLShader::Vertex, "normal.vert");
    shaders.addShaderFromSourceFile(QOpenGLShader::Fragment, "normal.frag");

    shaders.link();
    shaders.bind();

    color_location = shaders.attributeLocation("color");
    vertex_location = shaders.attributeLocation("position");

    shaders.release();
}

void RenderWidget::resizeGL(int w, int h)
{

}

void RenderWidget::paintGL()
{
    static const float vert[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    static const float color[] = {
        1.0, 0, 0,
        0, 1.0, 0,
        0, 0, 1.0
    };

    glClear(GL_COLOR_BUFFER_BIT);

    shaders.bind();
    glEnableVertexAttribArray(vertex_location);
    glEnableVertexAttribArray(color_location);

    glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, vert);
    glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, 0, color);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(color_location);
    glDisableVertexAttribArray(vertex_location);
    shaders.release();
}
