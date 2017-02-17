attribute vec4 a_Position;  
uniform   mat4 u_Matrix;

attribute vec2 a_TextureCoordinates;
varying   vec2 v_TextureCoordinates;

void main()                    
{                            
    gl_Position = a_Position; 
    v_TextureCoordinates = a_TextureCoordinates;       
}     	

