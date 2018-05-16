attribute vec2 position;
attribute vec3 color;

varying vec3 col;

void main()
{
   col = color;
   gl_Position = vec4(position, 0, 1);
}
