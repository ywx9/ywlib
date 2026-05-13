#pragma once

#define yw_hlsl_declare_row_major "\n#pragma pack_matrix(row)\n"

#define yw_hlsl_define_linear_to_srgb \
  "\nfloat4 linear_to_srgb(float4 Linear) {"\
  "\n  float3 srgb = pow(Linear.rgb, 1.0 / 2.2);"\
  "\n  return float4(srgb, Linear.a);"\
  "\n}\n"

#define yw_hlsl_define_srgb_to_linear \
  "\nfloat4 srgb_to_linear(float4 SRGB) {"\
  "\n  float3 l = pow(SRGB.rgb, 2.2);"\
  "\n  return float4(l, SRGB.a);"\
  "\n}\n"
