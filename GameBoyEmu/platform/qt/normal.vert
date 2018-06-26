attribute vec2 position;
attribute vec2 tex_coord;

varying vec2 texc;

void main()
{
    texc = tex_coord;
    gl_Position = vec4(position, 0, 1);
}
