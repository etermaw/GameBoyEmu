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

    vertex_location = shaders.attributeLocation("position");
    texc_location = shaders.attributeLocation("tex_coord");
    int sampler_location1 = shaders.uniformLocation("samp");
    int sampler_location2 = shaders.uniformLocation("samp2");

    shaders.setUniformValue(sampler_location1, 0);
    shaders.setUniformValue(sampler_location2, 1);

    shaders.release();

    float vertices[] = {
        -1, 1,  //top left
        1, 1,   //top right
        1, -1,  //bottom right
        -1, -1  //bottom left
    };

    float tex_coords[] = {
        0, 1,   //tl
        1, 1,   //tr
        1, 0,   //br
        0, 0    //bl
    };

    glGenBuffers(1, &vert_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vert_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &texc_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, texc_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTextures(2, textures);

    for (auto tex : textures)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderWidget::resizeGL(int w, int h)
{

}

void RenderWidget::paintGL()
{
    static const GLbyte indexes[] = {0,1,2, 0,2,3};

    glClear(GL_COLOR_BUFFER_BIT);

    shaders.bind();

    glBindBuffer(GL_ARRAY_BUFFER, vert_vbo);
    glEnableVertexAttribArray(vertex_location);
    glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, texc_vbo);
    glEnableVertexAttribArray(texc_location);
    glVertexAttribPointer(texc_location, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[current_texture]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[(current_texture + 1) % 2]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indexes);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(texc_location);
    glDisableVertexAttribArray(vertex_location);

    shaders.release();

    current_texture = (current_texture + 1) % 2;
}
