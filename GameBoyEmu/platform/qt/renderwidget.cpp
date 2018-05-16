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

    float vertices[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    float colors[] = {
            1.0, 0, 0,
            0, 1.0, 0,
            0, 0, 1.0
        };

    glGenBuffers(1, &vert_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vert_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
}

void RenderWidget::resizeGL(int w, int h)
{

}

void RenderWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    shaders.bind();

    glBindBuffer(GL_ARRAY_BUFFER, vert_vbo);
    glEnableVertexAttribArray(vertex_location);
    glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glEnableVertexAttribArray(color_location);
    glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(color_location);
    glDisableVertexAttribArray(vertex_location);

    shaders.release();
}
