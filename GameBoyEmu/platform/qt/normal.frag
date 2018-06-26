uniform sampler2D samp;
uniform sampler2D samp2;

varying vec2 texc;

void main()
{
    vec3 current_color = texture2D(samp,texc).bgr;
    vec3 prev_color = texture2D(samp2,texc).bgr;

    gl_FragColor = vec4(mix(current_color, prev_color, 0.5), 1);
}