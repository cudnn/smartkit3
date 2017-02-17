attribute vec4 position;
attribute vec2 inputTextureCoordinate;
uniform   mat4 u_Matrix;

varying vec2 vTexCoord;
void main(){
	gl_Position = u_Matrix * position;
	vTexCoord = inputTextureCoordinate;
}