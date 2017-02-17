attribute vec4 vPosition;
attribute vec2 a_texCoord;
varying vec2 texCoordVarying;

void main(){

	gl_Position = vPosition;
	texCoordVarying = a_texCoord;
}		