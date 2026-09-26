#ifdef FRAGMENT
layout(input_attachment_index=0,
#ifdef INPUT_ATTACHMENT_BINDING
binding=INPUT_ATTACHMENT_BINDING,
#else
binding=0,
#endif
set=w3) uniform lowp subpassInput Ai;layout(location=0) out i Ab;void main(){Ab=subpassLoad(Ai);}
#endif
