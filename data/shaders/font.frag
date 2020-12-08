#version 330

uniform vec4 textColor;
uniform sampler2D glyph;

in vec2 pass_uv;

out vec4 out_Color;

void main() 
{
	float alpha = texture(glyph, pass_uv.xy).r;

	out_Color = vec4(textColor.rgb, textColor.a * alpha);
}
