#version 330 core

// Positions are pixel offsets from the center of the framebuffer.
layout (location = 0) in vec2 aPixelOffset;

uniform vec2 uFramebufferSize;

void main()
{
    // Convert pixel offsets into OpenGL's -1 to +1 screen space.
    vec2 screenOffset = (aPixelOffset * 2.0) / uFramebufferSize;
    gl_Position = vec4(screenOffset, 0.0, 1.0);
}
