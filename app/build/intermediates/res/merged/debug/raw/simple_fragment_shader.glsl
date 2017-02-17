//precision mediump float; 
//      	 								
//uniform sampler2D u_TextureUnit;
//uniform sampler2D u_TextureUnit1;
//varying vec2      v_TextureCoordinates;
//
//void main()                    		
//{                              	
//    vec4 baseColor;
//    vec4 lightColor;
//
//    baseColor = texture2D( u_TextureUnit ,  v_TextureCoordinates);                                  		
//    lightColor = texture2D( u_TextureUnit1 ,  v_TextureCoordinates);                                  		
//    gl_FragColor = baseColor * ( lightColor + 0.25 );                                 		
//}

precision mediump float; 
      	 								
uniform sampler2D u_TextureUnit;
uniform float     Flag;
varying vec2      v_TextureCoordinates;

void main()                    		
{   
	vec4 alpha = vec4(0, 0, 0, 0.5);
	vec4 colour = vec4(1, 1, 1, 1);
	if(Flag == 0.0){		
		gl_FragColor = texture2D( u_TextureUnit ,  v_TextureCoordinates) - alpha;                                  		
	}else{
		gl_FragColor = colour;
	}
}

