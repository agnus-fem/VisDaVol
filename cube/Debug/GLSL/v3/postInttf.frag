// Transfer Functions -  4.3 Implementation of Post-Classification -  pg. 87
// Listing 4.3. Fragment programs for post-classifications via dependent texture
// look-up in Cg. The upper listing uses 3D textures.

uniform sampler3D volume_texture;
uniform sampler1D transfer_function;

void main(void){

      float index;
      //vec4 result;

      index = 180;//texture3D(volume_texture, gl_TexCoord[0].xyz).r*255;
      gl_FragColor  = texture1D(transfer_function, index);
      //gl_FragColor  = vec4(index,index,index,index);

}
