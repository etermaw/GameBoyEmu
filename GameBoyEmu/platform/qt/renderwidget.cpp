#include "renderwidget.h"

static const char vertex_shader[] = "       \
attribute vec2 position;                    \
attribute vec2 tex_coord;                   \
                                            \
varying vec2 texc;                          \
                                            \
void main()                                 \
{                                           \
   texc = tex_coord;                        \
   gl_Position = vec4(position, 0, 1);      \
}";

static const char fragment_shader[] = "                             \
uniform sampler2D samp;                                             \
uniform sampler2D samp2;                                            \
                                                                    \
varying vec2 texc;                                                  \
                                                                    \
void main()                                                         \
{                                                                   \
    vec3 current_color = texture2D(samp, texc).bgr;                 \
    vec3 prev_color = texture2D(samp2, texc).bgr;                   \
                                                                    \
    gl_FragColor = vec4(mix(current_color, prev_color, 0.5), 1);    \
}";

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000.0 / 60.0);
}

RenderWidget::~RenderWidget()
{
    makeCurrent();

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteTextures(2, textures);

    doneCurrent();
}

void RenderWidget::update_frame(u32* new_frame)
{
    makeCurrent();

    current_texture = (current_texture + 1) % 2;
    
    glBindTexture(GL_TEXTURE_2D, textures[current_texture]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGBA, /*GL_UNSIGNED_SHORT_1_5_5_5_REV*/ GL_UNSIGNED_INT_8_8_8_8_REV, new_frame);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[current_texture]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[(current_texture + 1) % 2]);

    doneCurrent();
}

void RenderWidget::initializeGL()
{
    initializeOpenGLFunctions();
    makeCurrent();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    shaders.addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader);
    shaders.addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader);

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
        0, 0,
        160.0/256.0, 0,
        160.0/256.0, 144.0/256.0,
        0, 144.0/256.0
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, /*GL_UNSIGNED_SHORT_1_5_5_5_REV*/ GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    //set up everything and leave on forever (we only change textures, so why even bother with state changes?)
    shaders.bind();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //load vertices
    glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertex_location);

    //load texture coords
    glVertexAttribPointer(texc_location, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof(float) * 8));
    glEnableVertexAttribArray(texc_location);

    //enable indexing
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    doneCurrent();
}

void RenderWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void RenderWidget::paintGL()
{
    makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

    doneCurrent();
}
