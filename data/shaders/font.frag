#version 330

// uniform buffer inputs
uniform UBO
{
	uniform vec4 textColor;
} ubo;

uniform sampler2D glyph;

in vec2 pass_uv;

out vec4 out_Color;

void main() 
{
	float alpha = texture(glyph, pass_uv.xy).r;

	out_Color = vec4(ubo.textColor.rgb, ubo.textColor.a * alpha);
}
