#ifdef FRAGMENT
layout(input_attachment_index=0,binding=G2,set=w3) uniform lowp subpassInputMS k9;layout(location=0) out i Ab;void main(){Ab=(subpassLoad(k9,0)+subpassLoad(k9,1)+subpassLoad(k9,2)+subpassLoad(k9,3))*.25;}
#endif
