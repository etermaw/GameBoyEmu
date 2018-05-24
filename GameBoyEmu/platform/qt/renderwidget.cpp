#include "renderwidget.h"

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent)
{
}

RenderWidget::~RenderWidget()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteTextures(2, textures);
}

void RenderWidget::update_frame(u16* new_frame)
{
    current_texture = (current_texture + 1) % 2;
    
    glBindTexture(GL_TEXTURE_2D, textures[current_texture]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, new_frame);
    glBindTexture(GL_TEXTURE_2D, 0);
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

    float vbo_data[] = {
         //vertex data
        -1, 1,  //top left
        1, 1,   //top right
        1, -1,  //bottom right
        -1, -1,  //bottom left

        //texture coords data
        0, 1,   //tl
        1, 1,   //tr
        1, 0,   //br
        0, 0    //bl
    };

    GLubyte indexes[] = {0,1,2, 0,2,3};

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //generate our "frame buffers"
    glGenTextures(2, textures);

    for (auto tex : textures)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    //set up everything and leave on forever (we only change textures, so why even bother with state changes?)
    shaders.bind();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //load vertices
    glEnableVertexAttribArray(vertex_location);
    glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //load texture coords
    glEnableVertexAttribArray(texc_location);
    glVertexAttribPointer(texc_location, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(8));

    //enable indexing
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void RenderWidget::resizeGL(int w, int h)
{

}

void RenderWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[current_texture]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[(current_texture + 1) % 2]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

    current_texture = (current_texture + 1) % 2;

    glBindTexture(GL_TEXTURE_2D, 0);
}
