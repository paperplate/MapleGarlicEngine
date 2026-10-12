import helpers;

cbuffer Uniforms
{
   float2 uResolution;
   float uTime;
}

struct FragmentOutput
{
   float4 color : SV_Target;
};

[shader("fragment")]
FragmentOutput main(float4 glFragCoord: SV_Position)
{
   FragmentOutput output;

   float2 uv = (glFragCoord.xy - 0.5 * uResolution.xy) / uResolution;

   float4 bgSky = float4(0.53, 0.81, 0.92, 1.0);
   float4 pink = float4(0.98, 0.61, 0.76, 1.0);
   float outline = 0.015;

   float4 col = bgSky;

   float leftArmSwing = -0.5 + sin(uTime * 5.0) * 0.4;
   float rightArmSwing = 0.5 - sin(uTime * 5.0) * 0.4;

   col = drawShape(col, uv, float2(0.0, 0.0), float2(0.35, 0.35), 0.0, pink, outline, uResolution);
   col = drawShape(col, uv, float2(-0.35, -0.05), float2(0.12, 0.08), leftArmSwing, pink, outline,
                   uResolution);
   col = drawShape(col, uv, float2(0.35, -0.05), float2(0.12, 0.08), rightArmSwing, pink, outline,
                   uResolution);

   output.color = col;
   return output;
}
