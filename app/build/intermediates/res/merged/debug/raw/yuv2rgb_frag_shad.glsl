precision mediump float;
varying vec2 texCoordVarying;			
				
uniform sampler2D texture0;
uniform sampler2D texture1;
void main()
{
	vec3 colourYCrCb;
	mat3 transformYCrCbITURec601FullRangeToRGB = mat3(1.0,    1.0,   1.0,
	1.402, -0.714, 0.0,
	0.0,   -0.344, 1.772);
	colourYCrCb.x  = texture2D(texture0, texCoordVarying).r;
	colourYCrCb.yz = texture2D(texture1, texCoordVarying).ra - 0.5;
	gl_FragColor = vec4(transformYCrCbITURec601FullRangeToRGB * colourYCrCb, 1.0);
//	gl_FragColor = vec4(0.5,0.5,0, 1.0);
}
				
