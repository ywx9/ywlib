#include "ywlib"
using namespace yw;

int main() {
  try {
    /// ランダムな２０個の頂点の座標を生成
    array<vector, 20> points1;
    for (int i = 0; i < 20; i++) {
      points1[i] = vector(rand() % 100, rand() % 100);
    }
    /// ランダムな100個の頂点の座標を再生成
    array<vector, 100> points2;
    for (int i = 0; i < points2.count; i++) {
      points2[i] = vector(rand() % 100, rand() % 100);
    }
    /// create structured_buffers
    structured_buffer<vector> points(points1);
    structured_buffer<vector> point_cloud(points2);
    /// create unordered_buffers
    unordered_buffer<fat4> distances(points.count);
    unordered_buffer<nat4> nearests(points.count);
    /// create constant_buffers
    constant_buffer<xmatrix> world(xv_identity);

    calc_distance_point_to_point(distances, nearests, points, point_cloud, world);
    auto result = distances.to_cpu();
    auto indices = nearests.to_cpu();

    for (nat i = 0; i < points.count; i++) {
      std::cout << "Point " << i << " is nearest to Point " << indices[i] << " with distance " << result[i] << std::endl;
    }

  } catch (const std::exception& E) {
    std::cout << E.what();
    return -1;
  }
  return 0;
}
