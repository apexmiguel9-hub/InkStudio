#pragma once

#include "draw_input_attachment.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_input_attachment_frag[] = R"===(#ifdef EB
layout(input_attachment_index=0,
#ifdef GF
binding=GF,
#else
binding=0,
#endif
set=w3) uniform lowp subpassInput Ai;layout(location=0) out i Ab;void main(){Ab=subpassLoad(Ai);}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive