#include "ywlib"
using namespace yw;


int main() {
  main::resize(800, 600);
  auto label = yw::label({0, 0, 100, 20}, L"Hello, World!");
  label.show();
  key::escape.down = [] { main::terminate(); };

  D3D11_FEATURE_DATA_DOUBLES featureData;
  HRESULT hr = main::d3d_device->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &featureData, sizeof(featureData));

  if (SUCCEEDED(hr)) {
    if (featureData.DoublePrecisionFloatShaderOps) {
      std::cout << "OK" << std::endl;
    } else {
      std::cout << "NO" << std::endl;
    }
  }
}
