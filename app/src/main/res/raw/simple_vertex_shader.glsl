attribute vec4 a_Position;  
uniform   mat4 u_Matrix;

attribute vec2 a_TextureCoordinates;
varying   vec2 v_TextureCoordinates;

void main()                    
{                            
    gl_Position = u_Matrix * a_Position;    
    gl_PointSize = 10.0;   
    
    v_TextureCoordinates = a_TextureCoordinates;       
}     	

